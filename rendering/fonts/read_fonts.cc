#include <cstdio>

#include <future>
#include <charconv>
#include <fstream>
#include <sstream>
#include <regex>

#include "read_fonts.hh"
#include "read_font.hh"
#include "endian.hh"
#include "share.hh"

using namespace std::literals;

static constexpr auto Ropt = std::regex_constants::ECMAScript | std::regex_constants::optimize;

static std::string GuessEncoding(std::string_view name)
{
    static const unsigned char latin[] = {0, 1, 2, 3, 4, 9, 10, 13, 14, 15, 16 };
    unsigned iso = 0;
    if(name.size() >= 4 && name.compare(name.size()-4,4, ".inc"sv) == 0)
    {
        if(name == "8x12.inc"sv) return "ibm-cp850";
        return "ibm-cp437";
    }
    if(name == "4x5.bdf"sv)
        return "xorg-4x5";
    if(name.compare(0,3, "lat"sv) == 0)
    {
        unsigned value = 0;
        std::from_chars(name.data()+3, name.data()+name.size(), value);
        if(value > 0 && value < std::size(latin)) iso = latin[value];
    }
    else if(name.compare(0,3, "iso"sv) == 0)
    {
        std::from_chars(name.data()+3, name.data()+name.size(), iso);
    }
    if(iso)
        return "iso-8859-" + std::to_string(iso);
    return {};
}

static auto FontPath()
{
    return FindShareFile(std::filesystem::path("fonts") / std::filesystem::path("files"));
}

void ReadFonts(std::ostream& out)
{
    std::vector<std::future<std::string>> tasks;

    auto [fn, status] = FontPath();
    for(auto p: std::filesystem::directory_iterator(fn))
        if(!p.is_directory())
        {
            std::string guess_enc = GuessEncoding(std::string(p.path().filename()));
            std::fprintf(stderr, "Reading %s ...\n", p.path().c_str());

            tasks.emplace_back(std::async(std::launch::async, [=]
            {
                auto data = Read_Font(p.path(), 0,0, false, guess_enc);

                std::ostringstream temp;
                temp << "FONT "sv << p.path().filename() << '\n';
                for(auto w: std::vector<unsigned>(data.widths)) // make copy
                {
                    temp << "SIZE "sv << w << ' ' << data.height << '\n';
                    if(!data.unicode)
                    {
                        temp << "GUESSED_ENCODING \""sv << guess_enc << "\"\n"sv;
                    }

                    data = Read_Font(p.path(), w, data.height, false, guess_enc);

                    std::vector<bool> bitmap(65536);
                    for(auto [ch, ofs]: data.glyphs)
                    {
                        while(bitmap.size() <= ch) bitmap.resize(bitmap.size() + 65536);
                        bitmap[ch] = true;
                    }

                    auto PrintRange = [&](char32_t begin, char32_t length)
                    {
                        if(length == 1)
                        {
                            char buf[64] = "  CHAR ";
                            auto [p, erc]  = std::to_chars(buf+7, buf+sizeof(buf)-1, begin, 16);
                            *p++ = '\n';
                            temp << std::string_view(buf, p-buf);
                        }
                        else
                        {
                            char buf[80] = "  CHARS ";
                            auto [p, erc]   = std::to_chars(buf+8, buf+sizeof(buf)/2,  begin,          16);
                            p[0] = ' '; p[1] = '-'; p[2] = ' ';
                            auto [p2, erc2] = std::to_chars(p+3,   buf+sizeof(buf)-1,  begin+length-1, 16);
                            *p2++ = '\n';
                            temp << std::string_view(buf, p2-buf);
                        }
                    };

                    for(std::size_t begin=0; begin<bitmap.size(); ++begin)
                    {
                        if(!bitmap[begin]) continue;
                        std::size_t length = 1;
                        while(begin+length < bitmap.size() && bitmap[begin+length]) ++length;
                    #if 1
                        std::uint_fast64_t bits=0, run=1, popcount=0, toggles=0, wbits = 64;
                        for(std::size_t bit=0; bit<64; ++bit)
                            if(begin+bit < bitmap.size() && bitmap[begin+bit])
                                { if(!run) {++toggles;} bits |= 1ull << bit; ++run; ++popcount; }
                            else
                                { if(run)  {++toggles;} run = 0; }
                        if(run >= 8)
                        {
                            bits = (bits << run) >> run;
                            wbits -= run;
                        }

                        if(toggles >= 3 && !run)
                        {
                            char buf[128] = "  MAP ";
                            auto [p, erc]   = std::to_chars(buf+6, buf+sizeof(buf)/2, begin, 16);
                            p[0] = ' ';
                            auto [p2, erc2] = std::to_chars(p+1,   buf+sizeof(buf)-1, bits,   2);
                            *p2++ = '\n';
                            temp << std::string_view(buf, p2-buf);
                            begin += wbits-1;
                        }
                        else
                    #endif
                        {
                            PrintRange(begin, length);
                            begin += length-1;
                        }
                    }
                }
                temp << '\n';
                return temp.str();
            }));
        }
    for(auto& t: tasks) out << t.get();
}

FontsInfo ParseFontsList(std::istream& f)
{
    FontsInfo result;

    #define r(str) [&]{ static std::regex r{str##s, Ropt}; return regex_match(line, mat, r); }()

    std::string filename;
    unsigned    x=0, y=0;
    std::smatch mat;

    std::vector<bool> bitmap;
    std::string guessed_encoding;
    auto Flush = [&]()
    {
        if(!bitmap.empty())
        {
            if(guessed_encoding.empty()) { guessed_encoding = GuessEncoding(filename); }
            result[ std::pair(filename, std::pair(x,y)) ] = std::pair(std::move(guessed_encoding), std::move(bitmap));
            bitmap.clear();
        }
    };
    auto Set = [&](std::size_t n)
    {
        while(bitmap.size() <= n) bitmap.resize(bitmap.size() + (n-bitmap.size() < 16384 ? 512 : 32768));
        bitmap[n] = true;
    };

    for(std::string line; std::getline(f,line), f; )
    {
        if(r("FONT[ \t]+\"([^\"]+)\""))
        {
            Flush();
            filename = mat[1];
        }
        else if(r("GUESSED_ENCODING[ \t]+\"([^\"]+)\""))
        {
            guessed_encoding = mat[1];
        }
        else if(r("[ \t]*SIZE[ \t]+([0-9]+)[ \t]+([0-9]+)"))
        {
            Flush();
            x = std::stoi(mat[1]);
            y = std::stoi(mat[2]);
        }
        else if(r("[ \t]*CHAR[ \t]+([0-9a-fA-F]+)"))
        {
            unsigned a = std::stoi(mat[1], nullptr, 16);
            Set(a);
        }
        else if(r("[ \t]*CHARS[ \t]+([0-9a-fA-F]+)[ \t]*-[ \t]*([0-9a-fA-F]+)"))
        {
            unsigned a = std::stoi(mat[1], nullptr, 16), b = std::stoi(mat[2], nullptr, 16);
            while(a <= b) Set(a++);
        }
        else if(r("[ \t]*MAP[ \t]+([0-9a-fA-F]+)[ \t]+([01]+)"))
        {
            unsigned a = std::stoi(mat[1], nullptr, 16);
            for(long n=0; n<mat[2].length(); ++n)
                if(mat[2].first[n] == '1')
                    Set(a + mat[2].length() - (n+1));
        }
    }

    #undef r
    Flush();
    return result;
}

FontsInfo ReadFontsList()
{
    auto [path, status] = FindCacheFile("fonts-list.dat", true);
    auto [fn, status2] = FontPath();
    if(!std::filesystem::exists(status)
    || (std::filesystem::exists(fn)
     && std::filesystem::last_write_time(fn) > std::filesystem::last_write_time(path)))
    {
        std::fprintf(stderr, "Building fonts cache...\n");
        std::ofstream file(path);
        ReadFonts(file);
    }
    std::ifstream f(path);
    return ParseFontsList(f);
}
