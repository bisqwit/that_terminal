#ifdef RUN_TESTS
# include <gtest/gtest.h>
# include <algorithm>
# include <set>
#endif

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
    lasttimer = ~0u;
    for(auto& c: cells) c.dirty = true;
}

void Window::LineSetRenderSize(unsigned val)
{
    for(std::size_t x=0; x<xsize; ++x)
        cells[cursy*xsize+x].render_size = val;
}

#ifdef RUN_TESTS
TEST(window, window_dirty)
{
    std::cout << "This test will take some time (will load 8x8 fonts for rendering)\n";
    Window w(80, 25);
    // Initially dirty
    w.cells[0].ch = 'X';
    EXPECT_TRUE( std::all_of(w.cells.begin(), w.cells.end(), [](Cell c){return c.dirty;}) );
    // Clean after render
    std::vector<std::uint32_t> buffer(80*8 * 25*8);
    w.Render(8,8, &buffer[0]);
    EXPECT_TRUE( std::none_of(w.cells.begin(), w.cells.end(), [](Cell c){return c.dirty;}) );
    // PutCh dirtifies some but not all cells
    w.PutCh(0,0, U'A');
    EXPECT_FALSE( std::none_of(w.cells.begin(), w.cells.end(), [](Cell c){return c.dirty;}) );
    EXPECT_FALSE( std::all_of(w.cells.begin(), w.cells.end(), [](Cell c){return c.dirty;}) );
    // Dirty after resize
    w.Resize(40, 25);
    EXPECT_TRUE( std::all_of(w.cells.begin(), w.cells.end(), [](Cell c){return c.dirty;}) );
}
TEST(window, fillbox)
{
    Window w(80, 25);
    // Initially blank
    EXPECT_TRUE( std::all_of(w.cells.begin(), w.cells.end(), [](Cell c){return c.ch == U' ';}) );
    // Fillbox with 'A' fills with 'A'
    Cell a; a.ch = U'A';
    w.FillBox(20,20, 40,3, a);
    EXPECT_EQ( std::count_if(w.cells.begin(), w.cells.end(), [&](Cell c){return c == a;}), 40*3 );
    // Fillbox without character fills with space
    w.FillBox(30,21, 20,1);
    EXPECT_EQ( std::count_if(w.cells.begin(), w.cells.end(), [&](Cell c){return c == a;}),       40*3-20 );
    EXPECT_EQ( std::count_if(w.cells.begin(), w.cells.end(), [&](Cell c){return c == w.blank;}), 80*25-(40*3-20) );
}
TEST(window, copytext)
{
    Window w(80, 25);
    // Create 15x10 'A' box
    Cell a; a.ch = U'A';
    w.FillBox(20,10, 15,10, a);
    // Copy it
    w.CopyText(60,14, 20,10, 15,10);
    EXPECT_EQ( std::count_if(w.cells.begin(), w.cells.end(), [&](Cell c){return c == a;}), 2*15*10 );
    // Test overlap (left) -- should add 10
    w.CopyText(19,10, 20,10, 15,10);
    EXPECT_EQ( std::count_if(w.cells.begin(), w.cells.end(), [&](Cell c){return c == a;}), 2*15*10 + 10 );
    // Test overlap (right) -- should add 10
    w.CopyText(21,10, 20,10, 15,10);
    EXPECT_EQ( std::count_if(w.cells.begin(), w.cells.end(), [&](Cell c){return c == a;}), 2*15*10 + 10+10 );
    // Test overlap (above) -- should add 15
    w.CopyText(20,9, 20,10, 15,10);
    EXPECT_EQ( std::count_if(w.cells.begin(), w.cells.end(), [&](Cell c){return c == a;}), 2*15*10 + 10+10+15 );
    // Test overlap (below) -- should add 15
    w.CopyText(20,11, 20,10, 15,10);
    EXPECT_EQ( std::count_if(w.cells.begin(), w.cells.end(), [&](Cell c){return c == a;}), 2*15*10 + 10+10+15+15 );
}
TEST(window, font_styles_are_distinct)
{
    // Stop timer, so that we do not get blinking cursor
    SetTimeFactor(0.);
    Window w(10,10);
    const std::size_t fx=8, fy=8, npixels = w.xsize*fx * w.ysize*fy;
    // Place 'B' at (4,4)
    w.PutCh(4,4, U'B');
    unsigned position = 4 * w.xsize + 4;
    // Backup that cell
    Cell c = w.cells[position];
    // Create model rendering
    std::vector<std::uint32_t> model_rendering(npixels);
    w.Render(fx,fy, &model_rendering[0]);
    // Render in different styles
    auto makestyle = [&](auto&& mogrify)
    {
        std::vector<std::uint32_t> pix(npixels);
        w.Dirtify();
        w.cells[position]=c;
        mogrify(w.cells[position]);
        w.Render(fx,fy, &pix[0]);
        return pix;
    };
    auto bold       = makestyle([&](Cell&t) { t.bold=true; });
    auto dim        = makestyle([&](Cell&t) { t.dim=true; });
    auto underline  = makestyle([&](Cell&t) { t.underline=true; });
    auto underline2 = makestyle([&](Cell&t) { t.underline2=true; });
    auto italic     = makestyle([&](Cell&t) { t.italic=true; });
    auto inverse    = makestyle([&](Cell&t) { t.inverse=true; });
    auto overstrike = makestyle([&](Cell&t) { t.overstrike=true; });
    auto overlined   = makestyle([&](Cell&t) { t.overlined=true; });
    auto whitefg    = makestyle([&](Cell&t) { t.fgcolor=0xFFFFFF; });
    auto whitebg    = makestyle([&](Cell&t) { t.bgcolor=0xFFFFFF; });
    EXPECT_NE(model_rendering, bold);
    EXPECT_NE(model_rendering, dim);
    EXPECT_NE(model_rendering, underline);
    EXPECT_NE(model_rendering, underline2);
    EXPECT_NE(model_rendering, italic);
    EXPECT_NE(model_rendering, inverse);
    EXPECT_NE(model_rendering, overstrike);
    EXPECT_NE(model_rendering, overlined);
    EXPECT_NE(model_rendering, whitefg);
    EXPECT_NE(model_rendering, whitebg);
    EXPECT_NE(bold, dim); EXPECT_NE(bold, underline); EXPECT_NE(bold, italic);
    EXPECT_NE(bold, inverse); EXPECT_NE(bold, overstrike); EXPECT_NE(bold, overlined);
    EXPECT_NE(bold, whitefg); EXPECT_NE(bold, whitebg); EXPECT_NE(bold, underline2);
    EXPECT_NE(dim, underline); EXPECT_NE(dim, italic);
    EXPECT_NE(dim, inverse); EXPECT_NE(dim, overstrike); EXPECT_NE(dim, overlined);
    EXPECT_NE(dim, whitefg); EXPECT_NE(dim, whitebg); EXPECT_NE(dim, underline2);
    EXPECT_NE(underline, italic);  EXPECT_NE(underline, underline2);
    EXPECT_NE(underline, inverse); EXPECT_NE(underline, overstrike); EXPECT_NE(underline, overlined);
    EXPECT_NE(underline, whitefg); EXPECT_NE(underline, whitebg);
    EXPECT_NE(italic, inverse); EXPECT_NE(italic, overstrike); EXPECT_NE(italic, overlined);
    EXPECT_NE(italic, whitefg); EXPECT_NE(italic, whitebg); EXPECT_NE(italic, underline2);
    EXPECT_NE(inverse, overstrike); EXPECT_NE(inverse, overlined); EXPECT_NE(inverse, whitefg); EXPECT_NE(inverse, whitebg);
    EXPECT_NE(overstrike, overlined); EXPECT_NE(overstrike, whitefg); EXPECT_NE(overstrike, whitebg);
    EXPECT_NE(overlined, whitefg); EXPECT_NE(overlined, whitebg); EXPECT_NE(inverse, underline2);
    EXPECT_NE(whitefg, whitebg); EXPECT_NE(overstrike, underline2); EXPECT_NE(whitefg, underline2); EXPECT_NE(whitebg, underline2);
}
TEST(window, rendersize)
{
    SetTimeFactor(1.);
    Window w(10,4);
    w.cursorvis = false; // Make cursor invisible
    const std::size_t fx=8, fy=8, ncells=w.xsize*w.ysize, npixels = fx*fy*ncells;

    // Place 'X' at (4,2).
    // This symbol is chosen because its top half looks like V and bottom half like Λ.
    // This makes it easy to detect programmatically whether we have rendered it correctly.
    w.PutCh(4,2, U'X');

    // Create model rendering
    std::vector<std::uint32_t> model_rendering(npixels), pixels(npixels);
    w.Render(fx,fy, &model_rendering[0]);

    auto makestyle = [&](auto&& mogrify)
    {
        std::vector<std::uint32_t> pix(npixels);
        w.Dirtify();
        mogrify(w);
        w.Render(fx,fy, &pix[0]);
        /*unsigned xpix = fx*w.xsize, ypix=fy*w.ysize;
        for(unsigned p=0,y=0; y<ypix; ++y)
        {
            for(unsigned x=0; x<xpix; ++x,++p)
                std::cout << (pix[p] ? '#' : ' ');
            std::cout << '\n';
        }*/
        return pix;
    };
    // Since cursor is at (0,0), check that LineSetRenderSize does nothing
    EXPECT_EQ(w.cursx, 0u);
    EXPECT_EQ(w.cursy, 0u);
    EXPECT_EQ(model_rendering, makestyle([](auto&w){ w.LineSetRenderSize(0); }));
    EXPECT_EQ(model_rendering, makestyle([](auto&w){ w.LineSetRenderSize(1); }));
    EXPECT_EQ(model_rendering, makestyle([](auto&w){ w.LineSetRenderSize(2); }));
    EXPECT_EQ(model_rendering, makestyle([](auto&w){ w.LineSetRenderSize(3); }));

    auto analyze_shape = [&](auto&& pixels)
    {
        // Find the first and last rows where something is drawn
        // Find the first and last pixel on both rows
        unsigned first_row=~0u, first_ranges[2]{~0u,0};
        unsigned last_row=~0u,  last_ranges[2]{~0u,0};
        unsigned xpix = fx*w.xsize, ypix=fy*w.ysize;
        for(unsigned p=0,y=0; y<ypix; ++y)
        {
            bool first = false;
            for(unsigned x=0; x<xpix; ++x,++p)
                if(pixels[p])
                {
                    if(first_row == ~0u) first = true;
                    if(first)
                    {
                        if(first_ranges[0] == ~0u) { first_row = y; first_ranges[0] = x; }
                        first_ranges[1] = x;
                    }
                    if(last_row != y) { last_ranges[0] = x; last_row = y; }
                    last_ranges[1] = x;
                }
        }
        return std::array<std::size_t,6>{
            first_row, last_row,
            first_ranges[0], first_ranges[1],
            last_ranges[0], last_ranges[1]};
    };

    auto model_shape = analyze_shape(model_rendering);
    w.cursx = 4;
    w.cursy = 2;

    // First, make sure that in the model rendering,
    // the symbol is strictly within permitted bounds
    EXPECT_GE(model_shape[0], w.cursy*fy); EXPECT_LT(model_shape[1], (w.cursy+1)*fy);
    EXPECT_GE(model_shape[2], w.cursx*fx); EXPECT_LT(model_shape[3], (w.cursx+1)*fx);
    EXPECT_GE(model_shape[4], w.cursx*fx); EXPECT_LT(model_shape[5], (w.cursx+1)*fx);
    // Symbol 'X' height should be at least 50% of cell height
    EXPECT_GE(model_shape[1], model_shape[0] + fy*2/4);
    // Symbol 'X' width at top should be at least 75% of cell width
    EXPECT_GE(model_shape[3], model_shape[2] + fx*3/4);
    // Symbol 'X' width at bottom should be at least 75% of cell width
    EXPECT_GE(model_shape[5], model_shape[4] + fx*3/4);

    // Size 1: Double width, full symbol (X); height stays the same.
    auto shape1 = analyze_shape( makestyle([](auto&w){ w.LineSetRenderSize(1); }) );
    EXPECT_EQ(shape1[0], model_shape[0]); EXPECT_EQ(shape1[1], model_shape[1]); // Height should be identical
    EXPECT_EQ(shape1[2], model_shape[2]*2); EXPECT_EQ(shape1[3], model_shape[3]*2+1); // Width should be double
    EXPECT_EQ(shape1[4], model_shape[4]*2); EXPECT_EQ(shape1[5], model_shape[5]*2+1); // Width should be double

    // Size 2: Double width, only top half (V) is displayed at double size
    auto shape2 = analyze_shape( makestyle([](auto&w){ w.LineSetRenderSize(2); }) );
    EXPECT_GE(shape2[0], w.cursy*fy); EXPECT_LT(shape2[1], (w.cursy+1)*fy); // Y coordinates should be in right bounds
    EXPECT_EQ(shape2[2], shape1[2]); EXPECT_EQ(shape2[3], shape1[3]);       // Top line should be identical to shape1
    EXPECT_GT(shape2[4], shape1[4]); EXPECT_LT(shape2[5], shape1[5]);       // Bottom line should be narrower.

    // Size 2: Double width, only bottom half (Λ) is displayed at double size
    auto shape3 = analyze_shape( makestyle([](auto&w){ w.LineSetRenderSize(3); }) );
    EXPECT_GE(shape3[0], w.cursy*fy); EXPECT_LT(shape3[1], (w.cursy+1)*fy); // Y coordinates should be in right bounds
    EXPECT_GT(shape3[2], shape1[2]); EXPECT_LT(shape3[3], shape1[3]);       // Top line should be narrower
    EXPECT_EQ(shape3[4], shape1[4]); EXPECT_EQ(shape3[5], shape1[5]);       // Bottom line should be identical to shape1.
}
TEST(window, cursor_blinks)
{
    SetTimeFactor(0.);
    Window w(10,4);
    w.cursorvis = true; // Make cursor visible

    const std::size_t fx=8, fy=8, cell_pixels = fx*fy, npixels = cell_pixels * w.xsize*w.ysize;

    // Render 1 second at 64 fps, count frames where cursor is visible
    unsigned count_cursor_visible = 0, count_frames = 0;
    for(unsigned frame=0; frame<64; ++frame, ++count_frames)
    {
        AdvanceTime(1.0 / 64.0);
        std::vector<std::uint32_t> pix(npixels);
        w.Dirtify();
        w.Render(fx,fy, &pix[0]);
        unsigned visible_pixels = std::count_if(pix.begin(), pix.end(), [](auto c){return c; });
        // Cursor size should be less than 20% of cell size
        EXPECT_LT(visible_pixels, cell_pixels/5);
        if(visible_pixels) ++count_cursor_visible;
    }
    // Cursor should be visible at least 25% of frames
    EXPECT_GT(count_cursor_visible, count_frames*1/4);
    // Cursor should be invisible at least 25% of frames
    EXPECT_LT(count_cursor_visible, count_frames*3/4);
}
TEST(window, text_blinks)
{
    SetTimeFactor(0.);
    Window w(3,3);
    w.cursorvis = false; // Make cursor invisible
    w.blank.blink = 0; w.PutCh(0,0, U'A');
    w.blank.blink = 1; w.PutCh(1,0, U'B');
    w.blank.blink = 2; w.PutCh(2,0, U'C');
    const std::size_t fx=8, fy=8, cell_pixels = fx*fy, npixels = cell_pixels * w.xsize*w.ysize;
    // Render 1 second at 64 fps
    for(unsigned frame=0; frame<64; ++frame)
    {
        AdvanceTime(1.0 / 64.0);
        std::vector<std::uint32_t> pix(npixels);
        w.Dirtify();
        w.Render(fx,fy, &pix[0]);
    }
}
TEST(window, person_animation)
{
    // Let window width be 20 cells, font height 16 pixels.
    // Render some text in top row in inverse.
    static const char32_t text[] = U"This is sample text ";
    const unsigned width = sizeof(text)/sizeof(*text)-1, height = 2;
    const unsigned fx = 8, fy = 16, time = 30, fps = 4, npixels=width*height*fx*fy;
    SetTimeFactor(0.);      // Chose manual timer
    Window w(width, height);
    w.cursorvis = false;    // Make cursor invisible
    w.blank.inverse = true; // Choose inverse attribute
    for(auto c: text) if(c) { w.PutCh(w.cursx, height-1, c); w.PutCh(w.cursx++, w.cursy, c); }
    EXPECT_EQ(w.cursx, width);

    // Wait until Person's starting X coordinate is outside window boundaries
    while(PersonBaseX(width*fx) < int(width*fx))
        AdvanceTime(0.01);

    std::vector<std::uint32_t> model(npixels);
    w.Render(fx,fy, &model[0]);

    /*{unsigned xpix = fx*w.xsize, ypix=fy*w.ysize;
    for(unsigned p=0,y=0; y<fy; ++y)
    {
        for(unsigned x=0; x<xpix; ++x,++p)
        {
            auto q = Unpack(model[p]);
            char Buf[32]; std::sprintf(Buf, "%X%X%X", q[0]>>4,q[1]>>4,q[2]>>4);
            std::cout << Buf;
        }
        std::cout << '\n';
    }}*/

    // Re-render screen for 30 seconds at 4 fps.
    // Every frame, the following conditions must be true:
    //   - Most of the text is visible
    //   - At most 16x16 pixels may be changed at any given time
    //   - The list of first changed columns should not be constant
    std::set<unsigned> change_columns;
    for(unsigned frame=0; frame<time*fps; ++frame)
    {
        const unsigned allowed_diff_x = 16, allowed_diff_y = 16;

        AdvanceTime(1.0 / fps);
        std::vector<std::uint32_t> pix(npixels);
        w.Dirtify();
        w.Render(fx,fy, &pix[0]);
        unsigned differences = 0, diffy[2]={~0u,0u}, diffx[2]={~0u,0u}, xpix=width*fx;

        /*{unsigned xpix = fx*w.xsize, ypix=fy*w.ysize;
        for(unsigned p=0,y=0; y<fy; ++y)
        {
            for(unsigned x=0; x<xpix; ++x,++p)
            {
                auto q = Unpack(pix[p]);
                char Buf[32]; std::sprintf(Buf, "%X%X%X", q[0]>>4,q[1]>>4,q[2]>>4);
                std::cout << Buf;
            }
            std::cout << '\n';
        }}*/

        for(unsigned n=0; n<xpix*fy; ++n)
            if(pix[n] != model[n])
            {
                ++differences;
                diffy[0] = std::min(diffy[0], n / xpix);
                diffy[1] = std::max(diffy[1], n / xpix);
                diffx[0] = std::min(diffx[0], n % xpix);
                diffx[1] = std::max(diffx[1], n % xpix);
            }
        EXPECT_LT(differences, npixels - allowed_diff_x*allowed_diff_y);
        //printf("Diffs: y(%d..%d) x(%d..%d)\n", diffy[0],diffy[1], diffx[0],diffx[1]);
        if(differences)
        {
            EXPECT_LE(diffy[1]-diffy[0], allowed_diff_y);
            EXPECT_LE(diffx[1]-diffx[0], allowed_diff_x);
            change_columns.insert(diffx[0]);
        }
    }
    // Expect the person to appear in at least 10 different positions
    EXPECT_GT(change_columns.size(), std::size_t(10));
}
TEST(window, coverage)
{
    // For coverage, run Cell comparisons
    Window w(10, 10);
    Cell mod; mod.bold = true;
    EXPECT_TRUE(w.blank == w.blank);
    EXPECT_FALSE(w.blank != w.blank);
    EXPECT_FALSE(w.blank == mod);
    EXPECT_TRUE(w.blank != mod);
}
#endif
