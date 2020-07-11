#include <regex>
#include <fstream>
#include <iostream>

#include "read_font.hh"
#include "gunzip.hh"
#include "endian.hh"
#include "share.hh"

using namespace std::literals;

// Syntactic shorthand for creating regular expressions.
static std::regex operator ""_r(const char* pattern, std::size_t length)
{
    return std::regex(pattern,length);
}

std::string ReadGZ(std::string_view filename)
{
    std::string result;
    char buf[8192];
    std::size_t pos=0, cap=0;
    std::ifstream f{ std::string(filename) };
    Deflate([&]() // in char
            {
                if(pos >= cap)
                {
                    if(!f) return -1;
                    f.read(buf, sizeof(buf));
                    pos = 0;
                    cap = f.gcount();
                }
                return (int) (unsigned char) buf[pos++];
            },
            [&](unsigned char c) // out char
            {
                result += char(c);
            },
            [&](std::size_t length, std::size_t offset) // window
            {
                result.insert(result.end(),
                              result.end()-offset, result.end()-offset + length);
            });
    return result;
}

struct PSFheader
{
    unsigned fontlen;
    bool     hastable;
    unsigned offset;
    bool     utf8;
    unsigned charsize;
    unsigned width;
    unsigned height;
};
PSFheader ReadPSFheader(const char* data)
{
    unsigned magic1 = R16r(&data[0]);
    unsigned magic2 = R32r(&data[0]);
    if(magic1 == 0x3604)
    {
        unsigned char mode  = data[2];
        unsigned char csize = data[3];
        return { (mode&1) ? 512u : 256u, // fontlen
                 (mode&6) != 0,          // hastable
                 4,       // offset
                 false,   // utf8
                 csize,   // charsize
                 8,       // width
                 csize }; // height
    }
    else if(magic2 == 0x72B54A86)
    {
        //unsigned      vers  = R32(&data[4]);
        unsigned    hdrsize = R32(&data[8]);
        unsigned      flags = R32(&data[12]);
        unsigned     length = R32(&data[16]);
        unsigned   charsize = R32(&data[20]);
        unsigned     height = R32(&data[24]);
        unsigned      width = R32(&data[28]);
        return { length,          // fontlen
                 (flags&1) != 0,  // hastable
                 hdrsize,         // offset
                 true,            // utf8
                 charsize,        // charsize
                 width,           // width
                 height };        // height
    }
    return {};
}

std::multimap<std::size_t, char32_t>
    Read_PSFgzEncoding(std::string_view data, const PSFheader& h)
{
    static constexpr char32_t SS   = 0xFFEFFAEE; // magic
    static constexpr char32_t TERM = 0xFFEFFAEF; // magic

    std::multimap<std::size_t, char32_t> table;
    if(h.hastable)
    {
        std::size_t pos = h.offset + h.fontlen * h.charsize;
        auto ReadUC = [&h,&pos,&data]() -> char32_t
        {
            char32_t unichar = (unsigned char) data[pos++];
            if(h.utf8)
            {
                if(unichar == 0xFE) return SS;
                if(unichar == 0xFF) return TERM;
                if(unichar >= 0x80)
                {
                    /*
                     UTF-8 encodings:
                         7 bits:  0xxxxxxx                              (7)
                         11 bits: 110xxxxx 10xxxxxx                     (5 + 6)
                         16 bits: 1110xxxx 10xxxxxx 10xxxxxx            (4 + 6 + 6)
                         21 bits: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx   (3 + 6 + 6 + 6)
                    */
                    if((unichar & 0xF8) == 0xF0) unichar &= 0x07;      // 3 bits
                    else if((unichar & 0xF0) == 0xE0) unichar &= 0x0F; // 4 bits
                    else if((unichar & 0xE0) == 0xC0) unichar &= 0x1F; // 5 bits
                    else unichar &= 0x7F;                              // 7 bits
                    while(pos < data.size() && ((data[pos] & 0xC0) == 0x80))
                        unichar = (unichar << 6) | (data[pos++] & 0x3F);
                }
                return unichar;
            }
            else
            {
                unichar |= (unsigned char)data[pos++] << 8;
                switch(unichar)
                {
                    case 0xFFFE: return SS;
                    case 0xFFFF: return TERM;
                    default: return unichar;
                }
            }
        };
        for(unsigned n=0; n<h.fontlen; ++n)
        {
            // Grammar:
            //   UC* (SS UC+)* TERM
            std::vector<char32_t> list;
            char32_t code;
            // Single-codepoint alternatives for this glyph
            while(pos < data.size())
            {
                code = ReadUC();
                if(code == TERM || code == SS) break;
                list.push_back(code);
            }
            // Multi-codepoint alternatives for this glyph (for e.g. combining diacritics)
            while(pos < data.size() && code == SS)
            {
                std::vector<char32_t> seq;
                for(;;)
                {
                    code = ReadUC();
                    if(code == SS || code == TERM) break;
                    seq.push_back(code);
                }
                // We ignore all of them if they have more than 1 codepoint.
                if(seq.size() == 1) list.push_back(seq[0]); //else { print 'seq'; print_r($seq); }
            }
            for(auto u: list)
                table.emplace(n, u);
        }
    }
    return table;
}

std::multimap<std::size_t, char32_t>
    Read_PSFgzEncoding(std::string_view filename)
{
    std::string data = ReadGZ(filename);
    const PSFheader h = ReadPSFheader(data.data());
    return Read_PSFgzEncoding(data, h);
}

GlyphList Read_PSFgz(std::string_view filename, unsigned /*width*/, unsigned /*height*/)
{
    std::string data = ReadGZ(filename);
    const PSFheader h = ReadPSFheader(data.data());
    auto table = Read_PSFgzEncoding(data, h);
    //print_r($table);
    GlyphList result;
    result.height = h.height;
    result.widths.push_back(h.width);

    for(std::size_t n=0; n<h.fontlen; ++n)
    {
        // List of unicode characters fulfilled by this glyph
        std::vector<char32_t> index{ char32_t(n) };
        if(!table.empty())
        {
            auto [begin, end] = table.equal_range(n);
            for(index.clear(); begin != end; ++begin) index.push_back(begin->second);
        }
        std::size_t start_offset = result.bitmap.size();
        result.bitmap.reserve(start_offset + h.charsize);
        for(auto c: index) result.glyphs.emplace(c, start_offset);
        // Read the bitmap
        std::size_t pos          = h.offset + n * h.charsize;
        std::size_t bytesperchar = h.charsize / h.height;
        for(std::size_t m=0; m<h.charsize; m += bytesperchar)
        {
            std::uint_fast64_t w = 0;
            // Read big-endian
            for(std::size_t a=0; a<bytesperchar; ++a)
                w |= ((std::uint_fast64_t)(unsigned char)data[pos + m + bytesperchar - a - 1]) << (a*8);
            if(h.width % 8)
                w >>= (8 - h.width%8);
            // Write little-endian
            for(std::size_t a=0; a<bytesperchar; ++a)
                result.bitmap.push_back( w >> (a*8) );
        }
    }
    return result;
}

/* Read a file in share/encodings/ that specifies character mapping for this encoding */
static std::vector<std::pair<char32_t, char32_t>> ReadEncoding(std::string_view filename)
{
    std::vector<std::pair<char32_t, char32_t>> result;
    auto [fn, status] = FindShareFile(std::filesystem::path("encodings") / std::filesystem::path(filename));
    if(std::filesystem::exists(status))
    {
        std::ifstream f(fn);
        auto delim = " \t"sv, hex = "0123456789ABCDEFabcdef"sv;
        for(std::string line; std::getline(f,line), f; )
        {
            std::size_t space    = line.find_first_of(delim);
            if(space == line.npos) continue;
            std::size_t notspace = line.find_first_not_of(delim, space);
            if(notspace == line.npos) continue;
            std::size_t notdigit = line.find_first_not_of(hex, notspace);
            if(notdigit == line.npos) notdigit = line.size();
            result.emplace_back(std::stoi(std::string(line, space), nullptr, 16),
                                std::stoi(std::string(line, notspace, notdigit-notspace), nullptr, 16));
        }
        std::sort(result.begin(), result.end());
    }
    return result;
}

char32_t BDFtranslateToUnicode(int index, std::string_view reg, std::string_view enc)
{
    if(index == -1) return 65533; // notdef in ibmfonts

    std::string regenc(reg); regenc += '-'; regenc += enc;
    for(char& c: regenc) if(c >= 'A' && c <= 'Z') c += 32; // tolower

    if(regenc == "ISO10646-1"sv)
        return index;
    if(regenc == "jisx0201.1976-0"sv)
    {
        if(index == 0x5C) return 0xA5;
        if(index == 0x7E) return 0x203E;
        if(index >= 0xA1) return index - 0xA1 + 0xFF61;
        return index;
    }

    static std::map<std::string, std::vector<std::pair<char32_t,char32_t>>> data;
    auto i = data.lower_bound(regenc);
    if(i == data.end())
    {
        i = data.emplace_hint(i, regenc, ReadEncoding(regenc + ".txt"));
    }
    auto& translation = i->second;
    auto j = std::lower_bound(translation.begin(), translation.end(), index,
                              [](const auto& pair, char32_t value) { return pair.first < value; });
    if(j != translation.end() && j->first == (char32_t)index)
    {
        return j->second;
    }
    return index;
}

GlyphList Read_BDF(std::string_view filename, unsigned width, unsigned /*height*/)
{
    std::size_t chno = 0;
    std::vector<std::size_t> data;
    unsigned mode            = 0;
    unsigned matrix_row_size = (8 + 7) >> 3; // 1 byte
    int ascent = 0, descent = 0, shiftbits = 0, beforebox = 0, afterbox = 0;
    std::string registry, encoding;
    unsigned fontwidth = width, fontheight = 1;
    GlyphList result;

    // Ignore width for variable-width fonts
    bool ignore_width = std::regex_search(std::string(filename), "monak1[246]|mona6x12|mona7x14"_r);

    std::ifstream f{ std::string(filename) };
    auto space = " \t\r"sv;
    for(std::string line; std::getline(f,line), f; )
    {
        // Trim
        std::size_t begin = line.find_first_not_of(space);
        if(begin == line.npos) continue;
        std::size_t end   = line.find_last_not_of(space)+1;
        if(begin > 0 || end < line.size()) line = line.substr(begin, end-begin);
        // Identify
        std::smatch mat;
        /**/ if(std::regex_match(line, mat, "FONT_ASCENT ([0-9]+)"_r)) ascent = std::stoi(mat[1]);
        else if(std::regex_match(line, mat, "FONT_DESCENT ([0-9]+)"_r)) descent = std::stoi(mat[1]);
        else if(std::regex_match(line, mat, "FONT -.*-([^-]*)-([^-]*)"_r)) { registry = mat[1]; encoding = mat[2]; }
        else if(std::regex_match(line, mat, "ENCODING (.*)"_r)) chno = BDFtranslateToUnicode(std::stoi(mat[1]), registry, encoding);
        else if(std::regex_match(line, mat, "CHARSET_REGISTRY \"?([^\"]*)"_r)) registry = mat[1];
        else if(std::regex_match(line, mat, "CHARSET_ENCODING \"?([^\"]*)"_r)) encoding = mat[1];
        else if(std::regex_match(line, mat, "FONTBOUNDINGBOX ([0-9]+) ([0-9]+).*"_r))
        {
            //fontwidth = std::stoi(mat[1]);
            fontheight = std::stoi(mat[2]);
            //matrix_row_size = (fontwidth + 7) >> 3; // 1 byte
            result.height = fontheight;
        }
        else if(std::regex_match(line, mat, "DWIDTH ([0-9]+) ([0-9]+).*"_r))
        {
            fontwidth = std::stoi(mat[1]);
            matrix_row_size = (fontwidth + 7) >> 3; // 1 byte
            if(std::find(result.widths.begin(), result.widths.end(), fontwidth) == result.widths.end())
                result.widths.push_back(fontwidth);
        }
        else if(std::regex_match(line, mat, "BBX (-?[0-9]+) (-?[0-9]+) (-?[0-9]+) (-?[0-9]+).*"_r))
        {
            int x = std::stoi(mat[1]);
            int y = std::stoi(mat[2]);
            int xo = std::stoi(mat[3]);
            int yo = std::stoi(mat[4]);
            shiftbits = (matrix_row_size - ((x + 7) >> 3) ) * 8 - xo;
            beforebox = (ascent - yo - y) * matrix_row_size;
            afterbox  = (descent + yo) * matrix_row_size;
            if(width == 9 && filename.find("gb16st") != filename.npos)
            {
                // The autodetected value seems bad, manually override
                beforebox = 1;
                afterbox = 0;
            }
            if(width == 10 && filename.find("gb24st") != filename.npos)
            {
                // The autodetected value seems bad, manually override
                beforebox = 4;
                afterbox = 0;
            }
        }
        else if(line == "BITMAP")
            mode = 1;
        else if(line == "ENDCHAR")
        {
            mode = 0;
            if(fontwidth == width || ignore_width)
            {
                std::vector<std::uint_fast64_t> map;
                if(beforebox < 0)
                {
                    if(data.size() >= std::size_t(-beforebox))
                        data.erase(data.begin(), data.begin() + (-beforebox));
                    else
                        data.clear();
                    beforebox = 0;
                }
                if(afterbox < 0)
                {
                    if(data.size() >= std::size_t(-afterbox))
                        data.erase(data.end() - (-afterbox), data.end());
                    else
                        data.clear();
                    afterbox = 0;
                }
                if(beforebox > 0)
                    map.resize(map.size() + beforebox);
                map.insert(map.end(), data.begin(), data.end());
                if(afterbox > 0)
                    map.resize(map.size() + afterbox);
                if(map.size() < fontheight)
                    map.resize(fontheight);

                unsigned bytes  = (width + 7) >> 3;
                unsigned sbytes = (fontwidth + 7) >> 3;

                std::size_t start_offset = result.bitmap.size();
                result.glyphs.emplace(chno, start_offset);
                result.bitmap.reserve(start_offset + fontheight * bytes);

                for(unsigned y=0; y<fontheight; ++y)
                {
                    auto m = map[y];
                    for(unsigned b=0; b<bytes; ++b)
                        result.bitmap.push_back( ((m >> ((sbytes*8)-fontwidth)) >> (b*8)) & 0xFF );
                }
                ++chno;
            }
            data.clear();
        }
        else if(mode)
        {
            std::uint_fast64_t v = std::stoll(line, nullptr, 16);
            if(shiftbits > 0)
                v <<= shiftbits;
            else
                v >>= -shiftbits;
            data.push_back(v);
        }
    }
    std::sort(result.widths.begin(), result.widths.end());
    if(ignore_width)
    {
        result.widths.erase(result.widths.begin(), std::prev(result.widths.end()));
    }
    return result;
}

GlyphList Read_Inc(std::string_view filename, unsigned width, unsigned height)
{
    if(width != 8) return {};

    GlyphList result;
    result.height = height;
    result.widths.push_back(width);

    std::ifstream f{ std::string(filename) };
    for(std::string line; std::getline(f,line), f; )
    {
    /*
    preg_match_all('/0x[0-9A-F]+/i', $line, $mat);
    foreach($mat[0] as $hex)
    {
      $data[(int)($n/$height)][] = hexdec($hex);
      ++$n;
    }
    */
    }
    return result;
}

GlyphList Read_ASM(std::string_view filename, unsigned width, unsigned height)
{
    if(width != 8) return {};

    GlyphList result;
    result.height = height;
    result.widths.push_back(width);

    std::ifstream f{ std::string(filename) };
    for(std::string line; std::getline(f,line), f; )
    {
    /*
    if(preg_match('/db\s+([^;]+)/i', $line, $mat))
    {
      preg_match_all('/[0-9A-F]+/i', $mat[1], $mat);
      foreach($mat[0] as $hex)
      {
        $data[(int)($n/$height)][] = hexdec($hex);
        ++$n;
      }
    }
    */
    }
    return result;
}

#include <iostream>

GlyphList Read_Font(std::filesystem::path filename, unsigned width, unsigned height, bool find)
{
    if(find)
    {
        auto [location, status] = FindShareFile(std::filesystem::path("fonts") / filename);
        filename = location;
    }

    std::string ext = filename.extension();
    if(ext == ".inc") return Read_Inc(std::string(filename), width, height);
    if(ext == ".asm") return Read_ASM(std::string(filename), width, height);
    if(ext == ".bdf") return Read_BDF(std::string(filename), width, height);
    return Read_PSFgz(std::string(filename), width, height);
}
