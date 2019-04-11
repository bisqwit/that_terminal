#include <unordered_map>
#include <array>

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

static std::array<unsigned,3> Unpack(unsigned rgb)
{
    return { rgb>>16, (rgb>>8)&0xFF, rgb&0xFF };
}
static unsigned Repack(const std::array<unsigned,3>& rgb)
{
    return (std::min(rgb[0],255u)<<16)
         + (std::min(rgb[1],255u)<<8)
         + (std::min(rgb[2],255u)<<0);
}

static unsigned MakeDim(unsigned rgb)
{
    auto a = Unpack(rgb);
    for(auto& e: a) e = e*2/3u;
    return Repack(a);
}
static unsigned MakeIntense(unsigned rgb)
{
    auto a = Unpack(rgb);
    for(auto& e: a) e = e*3/2u;
    return Repack(a);
}
static unsigned Mix13(unsigned color1,unsigned color2)
{
    auto a = Unpack(color1), b = Unpack(color2);
    for(unsigned n=0; n<3; ++n) a[n] = (b[n]*1 + a[n]*2)/3u;
    return Repack(a);
}
static unsigned Mix23(unsigned color1,unsigned color2)
{
    return Mix13(color2,color1);
}

void Window::Render(std::size_t fx, std::size_t fy, std::uint32_t* pixels)
{
    auto i = fonts.find(fx*256+fy);
    if(i == fonts.end()) return; // TODO: Do something better when a font is not found
    const unsigned char* font = i->second;

    std::size_t character_size_in_bytes = (fx*fy+7)/8;
    std::size_t font_row_size_in_bytes = (fx+7)/8;

    std::size_t row_for_underline1 = fy-1;
    std::size_t row_for_underline2a = fy-3;
    std::size_t row_for_underline2b = fy-1;

    static const unsigned char taketables[12][16] =
    {
        /*mode 0*/{0,0,0,0,3,3,3,3,0,0,0,0,3,3,3,3,},
        /*mode 1*/{0,0,0,0,1,1,3,3,0,0,0,0,1,1,3,3,},
        /*mode 2*/{0,0,0,0,3,3,3,3,1,1,1,1,3,3,3,3,},
        /*mode 3*/{0,0,0,0,1,1,3,3,1,1,1,1,1,1,3,3,},
        /*mode 4*/{0,0,1,1,2,2,3,3,0,0,1,1,2,2,3,3,},
        /*mode 5*/{0,0,0,1,1,1,2,3,0,0,0,1,1,1,2,3,},
        /*mode 6*/{0,0,1,1,2,2,3,3,1,1,2,2,2,2,3,3,},
        /*mode 7*/{0,0,0,1,1,1,2,3,1,1,1,2,1,1,2,3,},
        /*mode 8*/{0,0,2,2,1,1,3,3,0,0,2,2,1,1,3,3,},
        /*mode 9*/{0,0,1,2,0,0,2,3,0,0,1,2,0,0,2,3,},
        /*mode 10*/{0,0,2,2,2,2,3,3,0,0,2,2,2,2,3,3,},
        /*mode 11*/{0,0,1,2,1,1,2,3,0,0,1,2,1,1,2,3,},
    };

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

                const unsigned mode = cell.dim + cell.bold*2 + cell.italic*4*((fr*4/fy)%3);

                unsigned widefont = fontptr[0];
                widefont <<= 1;
                if(cell.italic && fr < fy*3/4) widefont >>= 1;

                for(std::size_t fc=0; fc<fx; ++fc, ++pix)
                {
                    auto fg    = cell.fgcolor;
                    auto bg    = cell.bgcolor;

                    //fg = 0xAAAAAA;
                    //bg = 0x000055;

                    if(cell.reverse ^ (x == cursx && y == cursy))
                    {
                        std::swap(fg, bg);
                    }

                    // TODO: deal with
                    //         - bold
                    //         - dim
                    //         - intense
                    //         - italic
                    //         - underline
                    //         - underline2
                    //         - overstrike
		    if(cell.intense)
                        fg = MakeIntense(fg);

                    if(cell.underline/* && !bit*/)
                    {
                        if(fr == row_for_underline1)
                            bg = 0x606060;
                    }
                    else if(cell.underline2/* && !bit*/)
                    {
                        if(fr == row_for_underline2a || fr == row_for_underline2b)
                            bg = 0x606060;
                    }
                    else if(cell.bold)
                    {
                    }

                    //bool bit   = (fontptr[fc/8] >> (7-fc%8)) & 1;

                    unsigned colors[4]  = { bg, Mix13(bg,fg), Mix23(bg,fg), fg };
                    unsigned mask = ((widefont << 2) >> (8-fc)) & 0xF;
                    *pix = colors[taketables[mode][mask]];
                }
            }
        }
    }
}

void Window::Resize(std::size_t newsx, std::size_t newsy)
{
    std::vector<Cell> newcells(newsx*newsy, blank);
    for(std::size_t my=std::min(ysize, newsy), y=0; y<my; ++y)
        for(std::size_t mx=std::min(xsize, newsx), x=0; x<mx; ++x)
            newcells[x + y*newsx] = cells[x + y*xsize];

    cells = std::move(newcells);
    xsize = newsx;
    ysize = newsy;
}
