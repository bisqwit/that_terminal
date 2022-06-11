#include <map>
#include <regex>
#include <fstream>
#include <cassert>
#include <thread>
#include <chrono>
#include <unordered_map>
#include <memory>

#include "make_similarities.hh"
#include "font_planner.hh"
#include "read_fonts.hh"
#include "endian.hh"
#include "share.hh"

namespace
{
    static constexpr auto Ropt = std::regex_constants::ECMAScript | std::regex_constants::optimize;

    std::map<std::pair<std::string,std::pair<unsigned,unsigned>>,
             std::pair<std::chrono::time_point<std::chrono::system_clock>,
                       std::shared_ptr<GlyphList>>
            > loaded_fonts;
    std::mutex fonts_lock;

    const FontsInfo& GetFontsInfo()
    {
        static const FontsInfo data = ReadFontsList();
        return data;
    }

    std::shared_ptr<GlyphList> LoadFont(const std::string& name, unsigned width, unsigned height,
                                        std::string_view guessed_encoding)
    {
        std::pair key(name, std::pair(width,height));
        auto i = loaded_fonts.lower_bound(key);
        if(i == loaded_fonts.end() || i->first != key)
        {
            std::unique_lock<std::mutex> lk(fonts_lock);
            i = loaded_fonts.lower_bound(key);
            if(i == loaded_fonts.end() || i->first != key)
            {
                //lk.unlock();
                auto font = Read_Font(name, width, height, true, guessed_encoding);
                //lk.lock();
                i = loaded_fonts.emplace_hint(i, key, std::pair(std::chrono::system_clock::now(),
                                                                std::make_shared<GlyphList>( std::move(font) )));
            }
        }
        i->second.first = std::chrono::system_clock::now();
        return i->second.second;
    }

    std::vector<std::regex> LoadPreferences()
    {
        std::vector<std::regex> result;
        auto [location, status] = FindShareFile(std::filesystem::path("fonts") / "preferences.txt");
        if(std::filesystem::exists(status))
        {
            std::ifstream f{ location };
            for(std::string line; std::getline(f,line), f; )
                if(!line.empty() && line[0] != '#')
                    result.emplace_back(line, Ropt);
        }
        return result;
    }

    int FontNameScore(const std::string& name)
    {
        static const std::vector<std::regex> patterns = LoadPreferences();
        int score = static_cast<int>( patterns.size() );
        for(auto& p: patterns)
        {
            if(std::regex_match(name, p)) break;
            --score;
        }
        return score;
    }
    bool IsBoldFont(const std::string& name, unsigned width)
    {
        static std::regex negative("Terminus[0-9]|mona[67]x|10x20cn|gb16st|unifont|[456]x[0-9]+\\.bdf", Ropt);
        static std::regex affirmative("TerminusBold|VGA|lat[1-9]-[01]|iso[01][0-9]\\.f|[0-9]\\.(?:inc|asm)", Ropt);
        if(std::regex_search(name, negative)) return false;
        if(std::regex_search(name, affirmative)) return true;
        return width >= 7;
    }

    unsigned long ScaleFont(unsigned long bitmap, unsigned oldwidth, unsigned newwidth)
    {
        if(oldwidth == newwidth) return bitmap;
        if(newwidth == oldwidth*2)
        {
            // interleave bits in the word by itself
            bitmap = (bitmap | (bitmap << 16)) & ((~0ull)/65537); // 0000FFFF0000FFFF
            bitmap = (bitmap | (bitmap <<  8)) & ((~0ull)/257);   // 00FF00FF00FF00FF
            bitmap = (bitmap | (bitmap <<  4)) & ((~0ull)/17);    // 0F0F0F0F0F0F0F0F (00001111)
            bitmap = (bitmap | (bitmap <<  2)) & ((~0ull)/5);     // 3333333333333333 (00110011)
            bitmap = (bitmap | (bitmap <<  1)) & ((~0ull)/3);     // 5555555555555555 (01010101)
            bitmap = bitmap | (bitmap << 1);
            return bitmap;
        }
        unsigned long result = 0;
        unsigned carry = 0, index = 0, bit = 0;
        for(unsigned n=0; n<oldwidth; ++n)
        {
            bit |= (bitmap >> n)&1;
            carry += newwidth;
            if(carry >= oldwidth)
            {
                do {
                    result |= bit << index++;
                    carry -= oldwidth;
                } while(carry >= oldwidth);
                bit = 0;
            }
        }
        if(bit) result |= 1u << (newwidth-1);
        return result;
    }
} /* namespace */

static auto BuildSieves(char32_t granularity)
{
    static const auto similarities = ParseSimilarities();

    using keytype = std::remove_const_t<FontsInfo::key_type>;
    std::unordered_map<char32_t, std::vector<std::shared_ptr<keytype>>> options;
    auto& info = GetFontsInfo();
    for(const auto& [key,data]: info)
    {
        auto k = std::make_shared<keytype>(key);
        for(char32_t base=0; base<data.second.size(); base+=granularity)
        {
            bool ok = false;
            for(char32_t ch=0; ch<granularity; ++ch)
            {
                if(data.second.size() > base+ch && data.second[base+ch])
                    { ok = true; break; }

                // Find viable approximations for this symbol
                auto choices = std::equal_range(similarities.begin(), similarities.end(), std::pair(base+ch,0),
                    [](const auto& pair1, const auto& pair2)
                    {
                        return pair1.first < pair2.first;
                    });
                // Check if the font implements one of those
                for(auto i = choices.first; i != choices.second; ++i)
                    if(data.second.size() > i->second && data.second[i->second])
                        { ok = true; break; }
            }
            if(ok)
                options[base].push_back(k);
        }
    }
    return options;
}
static const auto& GetSieves(char32_t granularity)
{
    static const auto sieves = BuildSieves(granularity);
    return sieves;
}

static const auto& GetSimilarities()
{
    static const auto similarities = ParseSimilarities();
    return similarities;
}

void FontPlan::Create(unsigned width, unsigned height, char32_t firstch, char32_t numch)
{
    std::tuple cur(width,height,firstch,numch);
    if(cur == prev) return;

    ready = false;
    prev = cur;

    const auto& similarities = GetSimilarities();
    const auto& info = GetFontsInfo();
    const auto& sieves = GetSieves(numch);

    std::thread updater([=,this,&sieves,&info,&similarities]
    {
        std::unique_lock<std::mutex> lk(working);

        static constexpr char32_t ilseq = 0xFFFD;

        auto old_loaded_fonts   = std::move(loaded_fonts);   loaded_fonts.clear();
        auto old_font_filenames = std::move(font_filenames); font_filenames.clear();

        resized_bitmaps.clear();
        resized_bitmaps.reserve(numch * height * ((width+7)/8));
        bitmap_pointers.resize(numch);
        bold_list.resize(numch);

        std::vector<std::pair<int, std::remove_const_t<FontsInfo::key_type>>> fonts;
        if(auto i = sieves.find(firstch); i != sieves.end())
            for(const auto& keyptr: i->second)
            {
                const auto& key = *keyptr;
                int name_score = FontNameScore(key.first);
                int wdiff = std::abs(int(width  - key.second.first))*2;
                int hdiff = std::abs(int(height - key.second.second))*2;
                if(width == key.second.first*2 || width*2 == key.second.first) wdiff /= 2;
                if(height == key.second.second*2 || height*2 == key.second.second) hdiff /= 2;
                int bdiff = IsBoldFont(key.first, key.second.first) ? 0 : 1;
                int diff = wdiff*wdiff + hdiff*hdiff + bdiff*bdiff;
                int score = diff*64 - name_score;
                fonts.emplace_back(score, key);
            }
        // If there were no candidates, check for something that implements ilseq
        int ilseq_penalty = 18000000;
        if(fonts.empty())
        {
            ilseq_penalty = 0;
            for(const auto& [key,data]: info)
            {
                int name_score = FontNameScore(key.first);
                int wdiff = std::abs(int(width  - key.second.first))*2;
                int hdiff = std::abs(int(height - key.second.second))*2;
                if(width == key.second.first*2 || width*2 == key.second.first) wdiff /= 2;
                if(height == key.second.second*2 || height*2 == key.second.second) hdiff /= 2;
                int bdiff = IsBoldFont(key.first, key.second.first) ? 0 : 1;
                int diff = wdiff*wdiff + hdiff*hdiff + bdiff*bdiff;
                int score = diff*64 - name_score;
                fonts.emplace_back(score, key);
            }
        }
        std::sort(fonts.begin(), fonts.end());
        if(false)
        {
            for(const auto& [basescore,key]: fonts)
                std::fprintf(stderr, "For target %ux%u, source %ux%u %s has score %d\n",
                    width,height,  key.second.first,key.second.second, key.first.c_str(), -basescore);
        }

        std::vector<std::tuple<char32_t, std::size_t, unsigned,unsigned>> resize_pointers;
        for(char32_t ch = firstch; ch < firstch + numch; ++ch)
        {
            // List all possible approximations for this symbol */
            auto choices = std::equal_range(similarities.begin(), similarities.end(), std::pair(ch,0),
                [](const auto& pair1, const auto& pair2)
                {
                    return pair1.first < pair2.first;
                });

            /* List all proper-size fonts that _have_ this symbol          */
            struct candidate
            {
                std::string filename;
                unsigned    width;
                unsigned    height;
                char32_t    ch;
                long        score;
                std::string_view encoding;
            } candidate = {};

            for(const auto& [basescore,key]: fonts)
            {
                long score = 0x7FFFFFFFl - basescore;
                if(score <= candidate.score) break;

                const auto& data = info.find(key)->second;

                //bool correct_size       = (key.second == std::pair(width,height));
                bool has_correct_symbol = data.second.size() > ch && data.second[ch];
                if(has_correct_symbol)
                {
                    if(candidate.filename.empty() || score > candidate.score)
                    {
                        //fprintf(stderr, "U+%04X: Chose exact U+%04X from %ux%u %s (score %ld)\n",
                        //    ch, ch, key.second.first,key.second.second,key.first.c_str(),score);
                        candidate = { key.first, key.second.first, key.second.second, ch, score, data.first };
                    }
                }
                else
                {
                    constexpr int approx_penalty_once = 1000000;
                    constexpr int approx_penalty_per  = 10;

                    long saved_score = score;
                    score -= approx_penalty_once;
                    if(candidate.score < score)
                        for(auto i = choices.first; i != choices.second; ++i)
                        {
                            if(data.second.size() > i->second && data.second[i->second])
                            {
                                if(candidate.filename.empty() || score > candidate.score)
                                {
                                    //fprintf(stderr, "U+%04X: Chose approx U+%04X from %ux%u %s (score %ld)\n",
                                    //    ch, i->second, key.second.first,key.second.second,key.first.c_str(),score);
                                    candidate = { key.first, key.second.first, key.second.second, i->second, score, data.first };
                                }
                            }
                            score -= approx_penalty_per;
                        }
                    score = saved_score - ilseq_penalty;
                    if(data.second.size() > ilseq && data.second[ilseq] // illseq
                    && (candidate.filename.empty() || score > candidate.score))
                    {
                        //fprintf(stderr, "U+%04X: Chose fail U+%04X from %ux%u %s (score %ld)\n",
                        //    ch, ilseq, key.second.first,key.second.second,key.first.c_str(),score);
                        candidate = { key.first, key.second.first, key.second.second, ilseq, score, data.first };
                    }
                }
            }

            // Append bitmap
            // This causes a font file load, but it is done only after we have already decided on a font file.
            std::shared_ptr<GlyphList> font = LoadFont(candidate.filename, candidate.width, candidate.height, candidate.encoding);
            assert(&*font != nullptr);
            auto k = font->glyphs.find(candidate.ch);
            if(k == font->glyphs.end())
            {
                std::fprintf(stderr, "Font %s (%ux%u enc %s) doesn't contain 0x%X\n",
                    candidate.filename.c_str(),
                    candidate.width, candidate.height,
                    candidate.encoding.data(),
                    candidate.ch);
                std::fprintf(stderr, "  It contains ");
                for(auto& c: font->glyphs) std::fprintf(stderr, " %X", c.first);
                std::fprintf(stderr, "\n");
            }
            assert(k != font->glyphs.end());
            const unsigned char* bitmap_pointer = &font->bitmap[ k->second ];
            assert(bitmap_pointer != nullptr);

            bold_list[ch-firstch] = IsBoldFont(candidate.filename, candidate.width);
            bitmap_pointers[ch-firstch] = bitmap_pointer;

            if(candidate.width == width && candidate.height == height)
            {
                // Make sure this font doesn't get unloaded while it's in use
                if(font_filenames.find(candidate.filename) == font_filenames.end())
                {
                    font_filenames.insert(candidate.filename);
                    loaded_fonts.emplace_back(std::move(font));
                }
            }
            else
            {
                // Create resized bitmap
                resize_pointers.emplace_back(ch, resized_bitmaps.size(), candidate.width,candidate.height);
                resized_bitmaps.resize(resized_bitmaps.size() + height * ((width+7)/8));

                // Make sure this font doesn't get unloaded while it's in use
                if(font_filenames.find(candidate.filename) == font_filenames.end())
                {
                    font_filenames.insert(candidate.filename);
                    loaded_fonts.emplace_back(std::move(font));
                }
            }
        }

        //#pragma omp parallel for
        for(std::size_t n=0; n<resize_pointers.size(); ++n)
        {
            auto [ch, offs, swidth, sheight] = resize_pointers[n];
            const unsigned char* bitmap_pointer = bitmap_pointers[ch-firstch];

            unsigned bytes_per_fontrow = (swidth + 7) / 8;
            for(unsigned y=0; y<height; ++y)
            {
                unsigned fr_actual = (y  ) * sheight / height;
                unsigned fr_next   = (y+1) * sheight / height;
                unsigned long widefont = 0;
                do {
                    const unsigned char* source = bitmap_pointer + fr_actual * bytes_per_fontrow;
                    widefont |= Rn(source, bytes_per_fontrow);
                } while(++fr_actual < fr_next);

                widefont = ScaleFont(widefont, swidth, width);
                for(unsigned x=0; x<width; x+=8)
                    resized_bitmaps[offs + y*((width+7)/8) + x/8] = widefont >> x;
            }
            bitmap_pointers[ch-firstch] = &resized_bitmaps[offs];
        }

        /* After the list is updated, unload the old resources; free those which are no longer used. */
        old_loaded_fonts.clear();
        old_font_filenames.clear();

        {std::lock_guard<std::mutex> temp(fonts_lock); // for printing
        std::fprintf(stderr, "For U+%04X..U+%04X Chose ", firstch,firstch+numch-1);
        for(auto& f: font_filenames) fprintf(stderr, " %s", f.c_str());
        std::fprintf(stderr, "\n");}

        ready = true;
        lk.unlock();
        ready_notification.notify_all();
    });
    updater.detach();
}

FontPlan::Glyph FontPlan::LoadGlyph(char32_t ch, unsigned scanline, unsigned render_width) const
{
    if(!ready)
    {
        std::unique_lock<std::mutex> lk(working);
        if(!ready)
            ready_notification.wait(lk, [&]{ return ready == true; });
    }
    unsigned bytes_per_fontrow = (std::get<0>(prev) + 7) / 8;
    const unsigned char* fontptr = bitmap_pointers[ch] + scanline * bytes_per_fontrow;

    unsigned long widefont = Rn(fontptr, bytes_per_fontrow);

    if(std::get<0>(prev) != render_width)
    {
        if(render_width != std::get<0>(prev)*2)
            fprintf(stderr, "font is %u, render %u\n",
                std::get<0>(prev), render_width);
    }
    widefont = ScaleFont(widefont, std::get<0>(prev), render_width);

    return {widefont, bold_list[ch]};
}

void FontPlannerTick()
{
    static auto prev = std::chrono::system_clock::now();
    auto now = std::chrono::system_clock::now();
    // Every 10 seconds, free fonts that have not been used for 60 seconds
    if(std::chrono::duration<double>(now - prev).count() >= 10.0)
    {
        std::lock_guard<std::mutex> lk(fonts_lock);
        for(auto i = loaded_fonts.begin(); i != loaded_fonts.end(); )
        {
            if(i->second.second.use_count() == 1
            && std::chrono::duration<double>(now - i->second.first).count() >= 60.0)
            {
                std::fprintf(stderr, "Unloading %ux%u font %s\n",
                    i->first.second.first, i->first.second.second,
                    i->first.first.c_str());
                i = loaded_fonts.erase(i);
            }
            else
                ++i;
        }
        prev = std::move(now);
    }
}
