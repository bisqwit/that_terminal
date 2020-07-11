#include <unordered_map>
#include <array>
#include <thread>

#include "screen.hh"
#include "color.hh"
#include "person.hh"
#include "clock.hh"
#include "settings.hh"
#include "ctype.hh"
#include "font.hh"

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
    FontHandler fonthandler = LoadFont(fx,fy);

    unsigned timer = unsigned(GetTime() * 60.0);
    bool old_blink1 = (lasttimer/20)&1, cur_blink1 = (timer/20)&1;
    bool old_blink2 = (lasttimer/ 6)&1, cur_blink2 = (timer/ 6)&1;
    bool old_blink3 = lasttimer%10<5, cur_blink3 = timer%10<5;

    std::vector<std::pair<std::size_t, std::array<std::pair<std::size_t, std::size_t>, 3>>> bar_ranges;
    /* Bar_ranges: [2] = cursor position
                   [1] = visible region
                   [0] = entire bar
                   (first,second) = first row (incl) and last row (excl)
     */
    auto FindScrollBarInfo = [&]()
    {
        // If on row 0 there is a "<digits>" followed by dim "/<digits>",
        // parse these two numbers (current row, number of rows).
        // If on row 1, column 0 there is a dim "<digits>", parse that
        // as the current scrolling position, and save the next column
        // as the scrollbar position.
        // Note: This is designed for use with Joe.
        // It only works if your .joerc contains this text:
        //    "Row %r\d/%l\d"
        // on the line with -rmsg. And you have linenumbers enabled (^Tn)
        for(std::size_t row=0; row+1<ysize; ++row)
        {
            std::size_t cur_row=0, num_rows=0, cur_top=0, bar_column=0;
            // Parse the status line at top of screen
            for(std::size_t x=0; x<xsize; ++x)
                if(cells[row*xsize + x].ch == U'/' && cells[row*xsize + x].dim && cells[row*xsize + x].inverse)
                {
                    // Parse the number of rows in the file (follows that '/')
                    for(std::size_t n=1; x+n<xsize && cells[row*xsize + x+n].ch >= U'0' && cells[row*xsize + x+n].ch <= U'9'; ++n)
                        num_rows = num_rows*10 + cells[row*xsize + x+n].ch-U'0';
                    // Parse the current row in file (right before the '/')
                    for(std::size_t mul=1, n=x; n-- > 0 && cells[row*xsize + n].ch >= U'0' && cells[row*xsize + n].ch <= U'9'; mul*=10)
                        cur_row += (cells[row*xsize + n].ch-U'0')*mul;
                    break;
                }
            if(!(num_rows && cur_row)) continue;
            // Identify the line number of file that is at the top of the window currently
            for(std::size_t x=0; x<xsize; ++x)
            {
                if(cells[row*xsize + 1*xsize + x].ch == U' ') { if(cur_top) { bar_column=x; break; } else continue; }
                if(cells[row*xsize + 1*xsize + x].ch < U'0' || cells[row*xsize + 1*xsize + x].ch > U'9') break;
                cur_top = cur_top*10 + cells[row*xsize + 1*xsize + x].ch-U'0';
            }
            if(!(cur_top && bar_column)) continue;

            // Find how many rows of room there is for scrollbar
            std::size_t firstrow = row + 1, height = 0;
            for(std::size_t y=firstrow; y<ysize; ++y, ++height)
            {
                if(cells[y*xsize + bar_column].inverse) break;
                if(cells[y*xsize + bar_column].ch != U' ') break;
                if(y != firstrow && !cells[y*xsize + bar_column].dim) break;
                /*
                if(cells[y*xsize + bar_column - 1].ch < U'0'
                || cells[y*xsize + bar_column - 1].ch > U'9') break;
                */
            }
            if(!height) continue;

            std::array<std::pair<std::size_t, std::size_t>, 3> bar =
            {
                // entire bar
                std::pair{ firstrow*fy,
                           firstrow*fy + height*fy },
                // visible region
                std::pair{ firstrow*fy + height*fy * (cur_top-1)        / num_rows,
                           firstrow*fy + height*fy * std::min(num_rows, cur_top+height-1) / num_rows },
                // cursor position
                std::pair{ firstrow*fy + height*fy * (cur_row-1)        / num_rows,
                           firstrow*fy + height*fy * (cur_row+1-1)      / num_rows }
            };
            bar_ranges.emplace_back(bar_column, bar);
        }
    };
    FindScrollBarInfo();

    auto substitutions = [&]
    {
        // Replace $H:$M:$S with time, $TEMP with temperature if found on inverse background
        std::unordered_map<std::size_t/*position*/, char32_t> substitutions;
        auto compare = [&](std::size_t x, std::size_t y, std::u32string_view str)
        {
            for(auto c: str)
            {
                if(!cells[y*xsize + x].inverse) return false;
                if(cells[y*xsize + x].ch != c) return false;
                ++x;
            }
            return true;
        };

        if(EnableTimeTemp)
        {
            char time[16], temp[16], temp1[]="+26.7"; // °C, celsius, degC

#ifdef CLOCK_BACKWARDS
            unsigned t = 19*3600 - GetTime();
#else
            unsigned t = GetTime() + 15*3600;
#endif
            std::sprintf(time, "%02d:%02d:%02d", t/3600, (t/60)%60, t%60);
            std::sprintf(temp, "%5s", temp1);
            for(std::size_t y=0; y<ysize; ++y) // cell-row
                for(std::size_t x=0; x<xsize; ++x) // cell-column
                {
                    if(x+8 < xsize && compare(x,y,U"$H:$M:$S"))
                        for(std::size_t p=0; time[p]; ++p)
                            substitutions.emplace(y*xsize+x+p, time[p]);
                    if(x+5 < xsize && compare(x,y,U"$TEMP"))
                        for(std::size_t p=0; temp[p]; ++p)
                            substitutions.emplace(y*xsize+x+p, temp[p]);
                    // 🌤 cloudy symbol
                }
        }
        return substitutions;
    }();

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
    //std::size_t screen_height = fy*ysize;
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
                if(auto i = substitutions.find(y * xsize + x); i != substitutions.end())
                    char_to_render = i->second;
                bool was_double = isdouble(char_to_render);
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
                    bool has_bar = false;
                    for(auto& bar: bar_ranges)
                        if(x == bar.first)
                            { has_bar = true; break; }
                    if(!has_bar)
                    {
                        pix          += width;
                        x            += was_double;
                        continue;
                    }
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

                std::size_t sb_begin=0, sb_end=0;
                unsigned sb_xormask = 0;
                for(const auto& [bar_column,bar]: bar_ranges)
                    if(x == bar_column)
                    {
                        std::size_t row = y*fy+fr;
                        if(row >= bar[2].first && row < bar[2].second)
                        {
                            // cursor
                            sb_xormask = 0xFFFFBF;
                        }
                        else if(row >= bar[1].first && row < bar[1].second)
                        {
                            // window
                            sb_xormask = 0x7F7FAF;
                        }
                        else if(row >= bar[0].first && row < bar[0].second)
                        {
                            // bar
                            sb_xormask = 0x3F3F6F;
                        }
                        else
                            continue;
                        sb_begin = width/8;
                        sb_end   = width - sb_begin;
                        break;
                    }

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
                else { auto r = fonthandler.LoadGlyph(char_to_render, fr_actual, width);
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

                    if(fc >= sb_begin && fc < sb_end && (((x*width+fc)^(y*fy+fr))&1))
                        color ^= sb_xormask;

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
