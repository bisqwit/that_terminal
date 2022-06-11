#include <unordered_map>
#include <array>
#include <thread>

#include "window.hh"
#include "color.hh"
#include "person.hh"
#include "clock.hh"
#include "ctype.hh"
#include "font_planner.hh"

/** Generates an intensity table for the given combination of options:
 * @param dim    Dim symbols
 * @param bold   Bold symbols
 * @param italic A value in range 0-1 representing the slant of the pixel row for italic text
 */
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
        bool values[4] = { bool(value&8), bool(value&4), bool(value&2), bool(value&1) }; // before,current,after,next
        float thisresult = calc(values[0], values[1], values[2]);
        float nextresult = calc(values[1], values[2], values[3]);
        float factor = thisresult + (nextresult-thisresult)*italic;
        // possible values of factor: 0, 1, 1.5,  0.333, 0.5
        result[value] = int(factor*127 + 0.5f);
    }
    return result;
}

/** Intensity tables for different combinations of font rendering.
 *
 * For taketables[mode][mask],
 *   mode is 16 when cell is dim
 *       plus 8 when cell is bold
 *       plus 0 when cell is not italic, a range from 0-7 when cell is italic (top to bottom)
 *   mask is a bitmask of four consecutive bits from the font, horizontally:
 *       bit 0 is previous
 *       bit 1 is current
 *       bit 2 is next
 *       bit 3 is next after next
 *  Result is a value in range 0-128,
 *  where 0 means fully background color,
 *  128 means fully foreground color,
 *  and the rest of values are linearly interpolated between.
 */
static constexpr std::array<unsigned char,16> taketables[] =
{
    #define i(n,i) CalculateIntensityTable(n&2,n&1,i),
    #define j(n) i(n,0/8.f)i(n,1/8.f)i(n,2/8.f)i(n,3/8.f)i(n,4/8.f)i(n,5/8.f)i(n,6/8.f)i(n,7/8.f)
    j(0) j(1) j(2) j(3) j(4) j(5) j(6) j(7)
    #undef j
    #undef i
};

void Window::Render(std::size_t fx, std::size_t fy, std::uint32_t* pixels)
{
    /* Blinking happens on timer. */
    unsigned timer = unsigned(GetTime() * 60.0);
    /* Blink settings:
     *     blink1 = Used for blinking speed 1 (normal):        1.5 Hz with 50% duty (60/40)
     *     blink2 = Used for blinking speed 2 (fast blinking): 5   Hz with 50% duty (60/12)
     *     blink3 = Used for cursor blinking:                  6   Hz with 50% duty (60/10)
     */
    bool old_blink1 = (lasttimer/20)&1, cur_blink1 = (timer/20)&1;
    bool old_blink2 = (lasttimer/ 6)&1, cur_blink2 = (timer/ 6)&1;
    bool old_blink3 = lasttimer%10<5,   cur_blink3 = timer%10<5;

    /* Fonts are cached for each 0x400 slots in Unicode */
    constexpr std::size_t font_granularity = 0x400;

    /* Cached fonts. Single-size and double-size. */
    static std::unordered_map<char32_t, FontPlan> fonts[2];
    for(const auto& cell: cells)
    {
        unsigned dbl = isdouble(cell.ch) ? 1 : 0;
        char32_t ch = cell.ch & ~(font_granularity-1);
        fonts[dbl][ch].Create(dbl ? fx*2 : fx, fy, ch, font_granularity);
    }

#ifdef _OPENMP
    std::size_t num_dirtycells = std::count_if(cells.begin(),cells.end(),
                                               [](const Cell& c) { return c.dirty; });
    double dirtiness = num_dirtycells / double(cells.size());
    // 0.1: 4.17s (nt=4)    5.1s (nt=2)  3.8s (nt=8) 3.58s (nt=16) 8.0s (nt=48)
    // 0.2: about 4s (nt=4)
    // 0.5: 4.3s (nt=4)
    // 1.0: 5.7s
    unsigned thread_count = std::clamp(std::min(16u, unsigned(dirtiness*16/0.1)),
                                       1u, std::thread::hardware_concurrency());
#endif

    std::size_t screen_width  = fx*xsize;
    #pragma omp parallel for schedule(static) collapse(2) num_threads(thread_count)
    for(std::size_t y=0; y<ysize; ++y) // cell-row
        for(std::size_t fr=0; fr<fy; ++fr) // font-row
        {
            bool is_person_row = cells[y*xsize+0].inverse && cells[y*xsize+xsize-1].inverse;

            std::uint32_t* pix = pixels + (y*fy+fr)*screen_width;
            std::size_t xroom = xsize;
            bool cursor_seen = !cursorvis;
            if(y != cursy && y != lastcursy) cursor_seen = true;
            for(std::size_t x=0; x<xroom; ++x) // cell-column
            {
                auto& cell = cells[y * xsize + x];

                unsigned xscale = 1;
                if(cell.render_size && (x+1) < xroom) { xscale = 2; --xroom; }
                unsigned width = fx * xscale;

                auto char_to_render = cell.ch;
                bool was_double     = isdouble(char_to_render);
                if(was_double) width *= 2;

                bool cursorloc      = y == cursy     && (x == cursx     || (was_double && cursx     == x+1));
                bool prev_cursorloc = y == lastcursy && (x == lastcursx || (was_double && lastcursx == x+1));

                if(!cell.dirty
                && !is_person_row
                && (!(cursorloc || prev_cursorloc)
                    || (cursorloc == prev_cursorloc && old_blink3 == cur_blink3)
                   )
                && (cell.blink!=1 || old_blink1 == cur_blink1)
                && (cell.blink!=2 || old_blink2 == cur_blink2)
                  )
                {
                    pix          += width;
                    x            += was_double;
                    continue;
                }

                unsigned fr_actual = fr;
                switch(cell.render_size)
                {
                    default: break;
                    case 2: fr_actual /= 2; break;
                    case 3: fr_actual /= 2; fr_actual += fy/2; break;
                }

                bool line = (cell.underline && (fr == (fy-1)))
                         || (cell.underline2 && (fr == (fy-1) || fr == (fy-3)))
                         || (cell.overstrike && (fr == (fy/2)))
                         || (cell.overlined && (fr == 0));

                bool do_cursor = !cursor_seen && cursorloc;
                if(do_cursor)
                {
                    unsigned lim = 7;
                    if(cells[4].ch == U'O') lim = 3;
                    if(fr >= fy*lim/8 && cur_blink3)
                        {}
                    else
                        do_cursor = false;
                    cursor_seen = true;
                }

                bool is_bold = true;
                unsigned long widefont = 0;
                /**/ if(cell.blink == 1 && !cur_blink1) {}
                else if(cell.blink == 2 && !cur_blink2) {}
                else { auto r = fonts[was_double ? 1 : 0].find(char_to_render &~ (font_granularity-1))->second
                                    .LoadGlyph(char_to_render % font_granularity, fr_actual, width);
                       widefont = r.bitmap;
                       is_bold  = r.bold; }

                unsigned shift = 2 + !cell.italic;
                bool dim = cell.dim && is_bold; // Disable dim on non-bold fonts, because it makes them look bad.

                const unsigned mode = cell.italic*(fr*8/fy)
                                    + 8*cell.bold
                                    + 16*dim;

                for(std::size_t fc=0; fc<width; ++fc)
                {
                    auto fg    = cell.fgcolor;
                    auto bg    = cell.bgcolor;

                    if(cell.inverse ^ inverse)
                    {
                        std::swap(fg, bg);
                    }

                    // taketables requires four consecutive bits from the font:
                    // previous, current, next and next2.
                    // <<2 is so that we can get previous pixel, and also not do -1 in width-1-fc

                    unsigned mask = ((widefont << shift) >> (width - fc)) & 0xF;
                    int take = taketables[mode][mask];
                    unsigned untake = std::max(0,128-take);
                    unsigned pre_bg = bg, pre_fg = fg;
                    if(cell.inverse)
                    {
                        PersonTransform(bg,fg, xsize*fx, x*fx+fc, y*fy+fr,
                                        (is_person_row && y==0) ? 1
                                      : y == (ysize-1) ? 2
                                      : 0);
                    }
                    if(do_cursor)
                    {
                        unsigned curs = cursorcolor;
                        if(curs == bg) curs = fg;
                        fg = bg;
                        bg = curs;
                    }
                    unsigned color  = Mix(bg,fg, untake, take, 128);

                    if(line && take == 0 && (!cell.inverse || color != 0x000000))
                    {
                        auto brightness = [](unsigned rgb)
                        {
                            auto p = Unpack(rgb);
                            return p[0]*299 + p[1]*587 + p[2]*114;
                        };
                        if(brightness(pre_fg) < brightness(pre_bg))
                            color = Mix(0x000000, color, 1,1,2);
                        else
                            color = Mix(0xFFFFFF, color, 1,1,2);
                    }

                    /*if(color && use_fx < fx)
                    {
                        // Make brighter to compensate for black 9th column
                        color = Mix(0xFFFFFF, color, fx-use_fx,use_fx, fx);
                    }*/

                    pix[fc] = color;
                }
                pix += width;
                x   += was_double;
            }
        }

    for(std::size_t y=0; y<ysize; ++y) // cell-row
        for(std::size_t x=0; x<xsize; ++x) // cell-column
            cells[y * xsize + x].dirty = false;

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

void Window::LineSetRenderSize(unsigned val)
{
    for(std::size_t x=0; x<xsize; ++x)
        cells[cursy*xsize+x].render_size = val;
}
