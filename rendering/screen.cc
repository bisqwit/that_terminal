#include <unordered_map>

#include "screen.hh"

static const unsigned char p32font[32*256] = {
#include "8x32.inc"
};
static const unsigned char p19font[19*256] = {
#include "8x19.inc"
};
static const unsigned char p12font[12*256] = {
#include "8x12.inc"
};
static const unsigned char p10font[10*256] = {
#include "8x10.inc"
};
static const unsigned char p15font[15*256] = {
#include "8x15.inc"
};
static const unsigned char p32wfont[32*256] = {
#include "16x32.inc"
};
static const unsigned char dcpu16font[8*256] = {
#include "4x8.inc"
};
static const unsigned char p16font[16*256] = {
#include "8x16.inc"
};
static const unsigned char p14font[14*256] = {
#include "8x14.inc"
};
static const unsigned char p8font[8*256] = {
#include "8x8.inc"
};

static std::unordered_map<unsigned, const unsigned char*> fonts
{
    { 16*256 + 32, p32wfont },
    { 4*256 + 8, dcpu16font },
    { 8*256 + 8, p8font },
    { 8*256 + 10, p10font },
    { 8*256 + 12, p12font },
    { 8*256 + 14, p14font },
    { 8*256 + 15, p15font },
    { 8*256 + 16, p16font },
    { 8*256 + 19, p19font },
    { 8*256 + 32, p32font },
};

void Window::Render(std::size_t fx, std::size_t fy, std::uint32_t* pixels)
{
    auto i = fonts.find(fx*256+fy);
    if(i == fonts.end()) return; // TODO: Do something better when a font is not found
    const unsigned char* font = i->second;

    std::size_t character_size_in_bytes = (fx*fy+7)/8;
    std::size_t font_row_size_in_bytes = (fx+7)/8;

    std::size_t screen_width  = fx*xsize;
    //std::size_t screen_height = fy*ysize;
    for(std::size_t y=0; y<ysize; ++y) // cell-row
    {
        for(std::size_t fr=0; fr<fy; ++fr) // font-row
        {
            std::uint32_t* pix = pixels + (y*fy+fr)*screen_width;
            for(std::size_t x=0; x<xsize; ++x) // cell-column
            {
                const auto& cell = cells[y * xsize + x];
                unsigned translated_ch = cell.ch; // TODO: Character-set translation
                if(translated_ch >= 256) translated_ch = '?';

                const unsigned char* fontptr =
                    font + translated_ch * character_size_in_bytes
                         + fr * font_row_size_in_bytes;

                for(std::size_t fc=0; fc<fx; ++fc, ++pix)
                {
                    bool bit   = (fontptr[fc/8] >> (7-fc%8)) & 1;
                    auto fg    = cell.fgcolor;
                    auto bg    = cell.bgcolor;

                    //fg = 0xAAAAAA;
                    //bg = 0x000055;

                    if(!(bit ^ cell.reverse ^ (x == cursx && y == cursy)))
                    {
                        std::swap(fg, bg);
                    }
                    // TODO: deal with
                    //         - bold
                    //         - dim
                    //         - italic
                    //         - underline
                    //         - underline2
                    //         - overstrike
                    *pix = fg;
                }
            }
        }
    }
}
