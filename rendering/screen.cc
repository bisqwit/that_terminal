#include <unordered_map>
#include <array>

#include "screen.hh"
#include "color.hh"
#include "person.hh"

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

#include "data/cp850.cc"
extern unsigned uni_to_cp850(unsigned character);

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


static constexpr std::array<unsigned char,16> CalculateIntensityTable(bool dim,bool bold,float italic)
{
    std::array<unsigned char,16> result={};
    auto calc = [=](bool prev,bool cur,bool next) constexpr
    {
        float result = cur;
        if(dim)
        {
            if(cur && !next)
            {
                if(prev) result *= float(1.f/3.f); // diminish rightmost pixel
                else     result *= float(2.f/3.f); // diminish all pixels
            }
        }
        if(bold)
        {
            if(cur || prev) result += float(1.f/4.f); // add dim extra pixel, slightly brighten existing pixels
        }
        return result;
    };
    for(unsigned value=0; value<16; ++value)
    {
        bool values[4] = { value&8, value&4, value&2, value&1 }; // before,current,after,next
        float thisresult = calc(values[0], values[1], values[2]);
        float nextresult = calc(values[1], values[2], values[3]);
        float factor = thisresult + (nextresult-thisresult)*italic;
        // possible values of factor: 0, 1, 1.5,  0.333, 0.5
        result[value] = int(factor*127 + 0.5f);
    }
    return result;
}

static constexpr std::array<unsigned char,16> taketables[] =
{
    #define i(n,i) CalculateIntensityTable(n&2,n&1,i),
    #define j(n) i(n,0/8.f)i(n,1/8.f)i(n,2/8.f)i(n,3/8.f)i(n,4/8.f)i(n,5/8.f)i(n,6/8.f)i(n,7/8.f)
    j(0) j(1) j(2) j(3) j(4) j(5) j(6) j(7)
    #undef j
    #undef i
};

static unsigned CursorColorTransform(unsigned color, unsigned cursorcolor)
{
    auto orig = Unpack(color), curs = Unpack(cursorcolor);
    std::array<unsigned,3> diff{ std::abs(int(orig[0]-curs[0])),
                                 std::abs(int(orig[1]-curs[1])),
                                 std::abs(int(orig[2]-curs[2])) };
    if(diff[0] <= 128
    || diff[1] <= 128
    || diff[2] <= 128)
    {
        // Choose black or white, whichever is farther from orig
        unsigned sum = orig[0]+orig[1]+orig[2];
        if(sum > 255*3-sum)
            return 0x000000;
        else
            return 0xFFFFFF;
    }

    return cursorcolor;
}

void Window::Render(std::size_t fx, std::size_t fy, std::uint32_t* pixels, unsigned timer)
{
    auto i = fonts.find(fx*256+fy);
    if(i == fonts.end()) return; // TODO: Do something better when a font is not found
    const unsigned char* font = i->second;

    std::size_t character_size_in_bytes = (fx*fy+7)/8;
    std::size_t font_row_size_in_bytes = (fx+7)/8;

    bool old_blink1 = (lasttimer/10)&1, cur_blink1 = (timer/10)&1;
    bool old_blink2 = (lasttimer/ 3)&1, cur_blink2 = (timer/ 3)&1;

    std::size_t screen_width  = fx*xsize;
    //std::size_t screen_height = fy*ysize;
    for(std::size_t y=0; y<ysize; ++y) // cell-row
    {
        for(std::size_t fr=0; fr<fy; ++fr) // font-row
        {
            std::uint32_t* pix = pixels + (y*fy+fr)*screen_width;
            std::size_t xroom = xsize;
            for(std::size_t x=0; x<xroom; ++x) // cell-column
            {
                auto& cell = cells[y * xsize + x];

                unsigned xscale = 1;
                if(cell.double_width && (x+1) < xroom) { xscale = 2; --xroom; }
                unsigned width = fx * xscale;

                if(!cell.dirty
                && y > 0 /* always render line 0 because of person */
                && (x != cursx || y != cursy)
                && (x != lastcursx || y != lastcursy)
                && (cell.blink!=1 || old_blink1 == cur_blink1)
                && (cell.blink!=2 || old_blink2 == cur_blink2)
                  )
                {
                    pix += width;
                    continue;
                }
                unsigned translated_ch = uni_to_cp850(cell.ch); // Character-set translation

                unsigned fr_actual = fr;
                switch(cell.double_height)
                {
                    default: break;
                    case 1: fr_actual /= 2; break;
                    case 2: fr_actual /= 2; fr_actual += fy/2; break;
                }

                const unsigned char* fontptr =
                    font + translated_ch * character_size_in_bytes
                         + fr_actual * font_row_size_in_bytes;

                const unsigned mode = cell.italic*(fr*8/fy)
                                    + 8*cell.bold
                                    + 16*cell.dim;

                unsigned widefont = fontptr[0];

                // TODO: 16-pix wide font support
                if(!cell.italic) widefont <<= 1;

                if(cell.blink == 1 && !cur_blink1) widefont = 0;
                if(cell.blink == 2 && !cur_blink2) widefont = 0;

                bool line = (cell.underline && (fr == (fy-1)))
                         || (cell.underline2 && (fr == (fy-1) || fr == (fy-3)))
                         || (cell.overstrike && (fr == (fy/2)));

                for(std::size_t fc=0; fc<width; ++fc, ++pix)
                {
                    auto fg    = cell.fgcolor;
                    auto bg    = cell.bgcolor;

                    //fg = 0xAAAAAA;
                    //bg = 0x000055;

                    if(cell.reverse ^ reverse)
                    {
                        std::swap(fg, bg);
                    }

                    unsigned mask = ((widefont << 2) >> (fx-fc/xscale)) & 0xF;
                    int take = taketables[mode][mask];
                    unsigned untake = std::max(0,128-take);
                    if(cell.reverse)
                    {
                        PersonTransform(bg,fg, xsize*fx, x*fx+fc,y*fy+fr,
                                        y == 0 ? 1
                                      : y == (ysize-1) ? 2
                                      : 0);
                    }
                    if(x == cursx && y == cursy && cursorvis)
                    {
                        unsigned curs = cursorcolor;
                        if(curs == bg) curs = fg;
                        fg = bg;
                        bg = curs;
                    }
                    unsigned color  = Mix(bg,fg, untake, take, 128);

                    if(line && take == 0 && (!cell.reverse || color != 0x000000))
                    {
                        auto brightness = [](unsigned rgb)
                        {
                            auto p = Unpack(rgb);
                            return p[0]*299 + p[1]*587 + p[2]*114;
                        };
                        if(brightness(fg) > brightness(bg))
                            color = Mix(0x000000, color, 1,1,2);
                        else
                            color = Mix(0xFFFFFF, color, 1,1,2);
                    }
                    *pix = color;
                }
                if(fr == (fy-1))
                {
                    cell.dirty = false;
                }
            }
        }
    }
    lastcursx = cursx;
    lastcursy = cursy;
    lasttimer = timer;
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
    Dirtify();
    if(cursy >= ysize) cursy = ysize-1;
    if(cursx >= xsize) cursx = xsize-1;
}

void Window::Dirtify()
{
    lastcursx = lastcursy = ~std::size_t();
    for(auto& c: cells) c.dirty = true;
}

void Window::LineSetHeightAttr(unsigned val)
{
    for(std::size_t x=0; x<xsize; ++x)
        cells[cursy*xsize+x].double_height = val;
}

void Window::LineSetWidthAttr(bool val)
{
    for(std::size_t x=0; x<xsize; ++x)
        cells[cursy*xsize+x].double_width = val;
}
