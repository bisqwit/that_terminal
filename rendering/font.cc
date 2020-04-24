#include <algorithm>
#include <unordered_map>
#include <tuple>
#include <cassert>

#include "font.hh"
#include "ctype.hh"

namespace realfonts
{
    #include "fonts-authentic.inc"
}
namespace fakefonts
{
    #include "fonts.inc"
}
#define NS1 fakefonts::
#define NS2 realfonts::
#include "fonts-list.inc"
#undef NS1
#undef NS2

extern unsigned long ScaleFont(unsigned long bitmap, unsigned oldwidth, unsigned newwidth)
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

auto FindFont(std::size_t fx, std::size_t fy)
{
    std::size_t actual_fx = fx, actual_fy = fy;
    bool        actual_bold = true;

    auto i = fonts.find(actual_fx*256 + actual_fy);
    if(i == fonts.end())
    {
        int worst = 0;
        for(auto j = fonts.begin(); j != fonts.end(); ++j)
        {
            std::size_t jx = j->first / 256, jy = j->first % 256, fb = std::get<3>(j->second);
            int wdiff = std::abs(int(jx - actual_fx))*2;
            int hdiff = std::abs(int(jy - actual_fy))*2;
            int bdiff = std::abs(int(fb - actual_bold));
            int diff = wdiff*wdiff + hdiff*hdiff + bdiff*bdiff;
            bool good = diff < worst;
            if(diff == worst)
            {
                if(jx <= actual_fx && jy <= actual_fy) good = true;
            }
            if(!worst || good)
            {
                i     = j;
                worst = diff;
            }
        }
        assert(i != fonts.end()); if(i == fonts.end()) i = fonts.begin();
        actual_fx   = i->first / 256;
        actual_fy   = i->first % 256;
    }

    auto [font,map,realmap,bold] = i->second;

    std::size_t font_row_size_in_bytes = (actual_fx+7)/8;
    std::size_t character_size_in_bytes = font_row_size_in_bytes*actual_fy;

    return std::tuple{actual_fx,actual_fy, font,map,realmap,
                      font_row_size_in_bytes,
                      character_size_in_bytes,
                      bold};
}

FontHandler LoadFont(std::size_t fx, std::size_t fy)
{
    FontHandler result;
    result.Load(fx,fy);
    return result;
}
void FontHandler::Load(std::size_t fx, std::size_t fy)
{
    auto& a = choices[0], &b = choices[1];
    std::tie(a.actual_fx,a.actual_fy,a.font,a.map,a.realmap,a.font_row_size_in_bytes,a.character_size_in_bytes,a.bold) = FindFont(fx,fy);

    // For the secondary map, choose one of these:
    //    8x8  (misaki, adds Japanese (JIS))      - perfect pair with 4x8
    //   12x12 (mona + f12, adds Japanese (JIS))  - perfect pair with 6x12
    //   12x13 (misc, adds Japanese)              - perfect pair with 6x13
    //   14x14 (mona + f14, adds Japanese (JIS))  - perfect pair with 7x14
    //   16x16 (unifont + song, adds everything)  - perfect pair with 9x16
    //   18x18 (misc, adds Japanese and Korean)   - perfect pair with 9x18
    //   24x24 (song, adds CJ)                    - perfect pair with 10x24 - does not include Korean though
    std::tie(b.actual_fx,b.actual_fy,b.font,b.map,b.realmap,b.font_row_size_in_bytes,b.character_size_in_bytes,b.bold) =
              std::apply(FindFont, (fy>=20)          ? std::tuple<int,int>{24,24}
                                 : (fy>=17)          ? std::tuple<int,int>{18,18}
                                 : (fx>=8 && fy>=15) ? std::tuple<int,int>{16,16}
                                 : (fx<6)            ? std::tuple<int,int>{8,8}
                                 : (fy>=14)          ? std::tuple<int,int>{14,14}
                                 : (fy<=12)          ? std::tuple<int,int>{12,12}
                                 :                     std::tuple<int,int>{12,13});

    this->fx = fx;
    this->fy = fy;
}

FontHandler::Glyph FontHandler::LoadGlyph(char32_t char_to_render, unsigned scanline, unsigned render_width)
{
    auto& a = choices[0], &b = choices[1];

    // Character-set translation
    auto use_fx = a.actual_fx;
    auto use_fy = a.actual_fy;
    auto use_charsize = a.character_size_in_bytes;
    auto use_fontsize = a.font_row_size_in_bytes;
    auto use_font = a.font;

    bool was_double = isdouble(char_to_render);
    bool is_bold    = a.bold;

    auto [translated_ch, avail] = a.map(char_to_render);

    // If there is no alt font, take everything from that font.
    // For double-width characters, try real characters only from first font
    //   If there is no real character in primary font, try approximation from second font
    //   If not available there either, use approximation from first font.
    // For single-width characters, try approximation from first font
    //   And if not available, try the wide font next.
    if(b.font != a.font
    && (was_double ? !a.realmap(char_to_render).second : !avail))
    {
        if(auto [t2, avail2] = b.map(char_to_render); avail2)
        {
            translated_ch = t2;
            avail = avail2;
            use_fx = b.actual_fx;
            use_fy = b.actual_fy;
            use_charsize = b.character_size_in_bytes;
            use_fontsize = b.font_row_size_in_bytes;
            use_font = b.font;
            is_bold = b.bold;
        }
    }

    unsigned long widefont = 0;

    unsigned fr_actual = (scanline  ) * use_fy / fy;
    unsigned fr_next   = (scanline+1) * use_fy / fy;
    do {
        const unsigned char* fontptr =
            use_font + translated_ch * use_charsize
                     + fr_actual * use_fontsize;

        widefont |= fontptr[0];
        if(use_fx > 8)  {widefont |= (fontptr[1] << 8);
        if(use_fx > 16) {widefont |= (fontptr[2] << 16);
        if(use_fx > 24) {widefont |= (fontptr[3] << 24);}}}
    } while(++fr_actual < fr_next);

    widefont = ScaleFont(widefont, use_fx, render_width);

    return {widefont,is_bold};
}
