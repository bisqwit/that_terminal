#include <unordered_map>
#include <array>

#include "screen.hh"
#include "color.hh"
#include "person.hh"

#include "fonts.inc"

static std::unordered_map<unsigned, std::pair<const unsigned char*, unsigned(*)(char32_t)>> fonts
{
    { 6*256 + 9,  {ns_f6x9::bitmap, ns_f6x9::unicode_to_bitmap_index} },
    { 8*256 + 8,  {ns_f8x8::bitmap, ns_f8x8::unicode_to_bitmap_index} },
    { 8*256 + 10, {ns_f8x10::bitmap, ns_f8x10::unicode_to_bitmap_index} },
    { 8*256 + 12, {ns_f8x12::bitmap, ns_f8x12::unicode_to_bitmap_index} },
    { 8*256 + 14, {ns_f8x14::bitmap, ns_f8x14::unicode_to_bitmap_index} },
    { 8*256 + 15, {ns_f8x15::bitmap, ns_f8x15::unicode_to_bitmap_index} },
    { 8*256 + 16, {ns_f8x16::bitmap, ns_f8x16::unicode_to_bitmap_index} },
    { 8*256 + 19, {ns_f8x19::bitmap, ns_f8x19::unicode_to_bitmap_index} },
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

void Window::Render(std::size_t fx, std::size_t fy, std::uint32_t* pixels, unsigned timer)
{
    std::size_t actual_fx = fx, actual_fy = fy;

    auto i = fonts.find(actual_fx*256 + actual_fy);
    if(i == fonts.end())
    {
        int worst = 0;
        for(auto j = fonts.begin(); j != fonts.end(); ++j)
        {
            std::size_t jx = j->first / 256, jy = j->first % 256;
            int wdiff = std::abs(int(jx - fx));
            int hdiff = std::abs(int(jy - fy));
            int diff = wdiff*wdiff + hdiff*hdiff;
            bool good = diff < worst;
            if(diff == worst)
            {
                if(jx <= fx && jy <= fy) good = true;
            }
            if(!worst || good)
            {
                i     = j;
                worst = diff;
            }
        }
        actual_fx = i->first / 256;
        actual_fy = i->first % 256;
    }

    const unsigned char* font = i->second.first;
    const auto            map = i->second.second;

    std::size_t font_row_size_in_bytes = (actual_fx+7)/8;
    std::size_t character_size_in_bytes = font_row_size_in_bytes*actual_fy;

    bool old_blink1 = (lasttimer/20)&1, cur_blink1 = (timer/20)&1;
    bool old_blink2 = (lasttimer/ 6)&1, cur_blink2 = (timer/ 6)&1;
    bool old_blink3 = lasttimer%10<5, cur_blink3 = timer%10<5;

    std::size_t bar_column = ~std::size_t();
    std::pair<std::size_t, std::size_t> bar_ranges[3]={ {0,0}, {0,0}, {0,0} };
    auto FindScrollBarInfo = [&]()
    {
        std::size_t cur_row=0, num_rows=0, cur_top=0;
        // If on row 0 there is a "<digits>" followed by dim "/<digits>",
        // parse these two numbers (current row, number of rows).
        // If on row 1, column 0 there is a dim "<digits>", parse that
        // as the current scrolling position, and save the next column
        // as the scrollbar position.
        // Note: This is designed for use with Joe.
        // It only works if your .joerc contains this text:
        //    "Row %r\d/%l\d"
        // on the line with -rmsg. And you have linenumbers enabled (^Tn)

        if(ysize < 2) return;
        // Parse the status line at top of screen
        for(std::size_t x=0; x<xsize; ++x)
            if(cells[x].ch == U'/' && cells[x].dim && cells[x].inverse)
            {
                // Parse the number of rows in the file (follows that '/')
                for(std::size_t n=1; x+n<xsize && cells[x+n].ch >= U'0' && cells[x+n].ch <= U'9'; ++n)
                    num_rows = num_rows*10 + cells[x+n].ch-U'0';
                // Parse the current row in file (right before the '/')
                for(std::size_t mul=1, n=x; n-- > 0 && cells[n].ch >= U'0' && cells[n].ch <= U'9'; mul*=10)
                    cur_row += (cells[n].ch-U'0')*mul;
                break;
            }
        if(!(num_rows && cur_row)) return;
        // Identify the line number of file that is at the top of the window currently
        for(std::size_t x=0; x<xsize; ++x)
        {
            if(cells[1*xsize + x].ch == U' ') { if(cur_top) { bar_column=x; break; } else continue; }
            if(cells[1*xsize + x].ch < U'0' || cells[1*xsize + x].ch > U'9') break;
            cur_top = cur_top*10 + cells[1*xsize + x].ch-U'0';
        }
        if(!(cur_top && bar_column)) { bar_column = ~std::size_t(); return; }
        // Find how many rows of room there is for scrollbar
        std::size_t firstrow = 1*fy, height = 0;
        for(std::size_t y=1; y<ysize; ++y, ++height)
            if(!(cells[y*xsize + bar_column].dim && cells[y*xsize + bar_column].ch == U' '))
                { break; }
        std::size_t heightp = height * fy; // scrollbar height in pixels
        // cursor position
        bar_ranges[0]        = std::pair(firstrow, firstrow+std::max<std::size_t>(1,heightp));
        // visible region
        bar_ranges[1].first  = firstrow + (cur_top-1)        * heightp / num_rows;
        bar_ranges[1].second = firstrow + (cur_top+height-1) * heightp / num_rows;
        // entire bar
        bar_ranges[2].first  = firstrow + (cur_row-1)        * heightp / num_rows;
        bar_ranges[2].second = firstrow + (cur_row+1-1)      * heightp / num_rows;
    };
    FindScrollBarInfo();

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
                if(cell.render_size && (x+1) < xroom) { xscale = 2; --xroom; }
                unsigned width = fx * xscale;

                if(!cell.dirty
                && y > 0 /* always render line 0 because of person */
                && (x != cursx || y != cursy)
                && (x != lastcursx || y != lastcursy)
                && (cell.blink!=1 || old_blink1 == cur_blink1)
                && (cell.blink!=2 || old_blink2 == cur_blink2)
                && x != bar_column
                  )
                {
                    pix += width;
                    continue;
                }
                unsigned translated_ch = map(cell.ch); // Character-set translation

                unsigned fr_actual = fr * actual_fy / fy;
                switch(cell.render_size)
                {
                    default: break;
                    case 2: fr_actual /= 2; break;
                    case 3: fr_actual /= 2; fr_actual += fy/2; break;
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
                         || (cell.overstrike && (fr == (fy/2)))
                         || (cell.overlined && (fr == 0));

                std::size_t sb_begin=0, sb_end=0;
                unsigned sb_xormask = 0;
                if(x == bar_column) // scrollbar displayed in this column?
                {
                    sb_begin = width/8;
                    sb_end   = width - sb_begin;
                    std::size_t row = y*fy+fr;
                    if(row >= bar_ranges[2].first && row < bar_ranges[2].second)
                    {
                        // cursor
                        sb_xormask = 0xFFFFBF;
                    }
                    else if(row >= bar_ranges[1].first && row < bar_ranges[1].second)
                    {
                        // window
                        sb_xormask = 0x7F7FAF;
                    }
                    else if(row >= bar_ranges[0].first && row < bar_ranges[0].second)
                    {
                        // bar
                        sb_xormask = 0x3F3F6F;
                    }
                }

                bool do_cursor = x == cursx && y == cursy && cursorvis;
                if(do_cursor)
                {
                    if(fr >= fy*7/8 && cur_blink3)
                        {}
                    else
                        do_cursor = false;
                }

                for(std::size_t fc=0; fc<width; ++fc, ++pix)
                {
                    auto fg    = cell.fgcolor;
                    auto bg    = cell.bgcolor;

                    //fg = 0xAAAAAA;
                    //bg = 0x000055;

                    if(cell.inverse ^ inverse)
                    {
                        std::swap(fg, bg);
                    }

                    unsigned mask = ((widefont << 2) >> (actual_fx-fc/xscale)) & 0xF;
                    int take = taketables[mode][mask];
                    unsigned untake = std::max(0,128-take);
                    unsigned pre_bg = bg, pre_fg = fg;
                    if(cell.inverse)
                    {
                        PersonTransform(bg,fg, xsize*fx, x*fx+fc,y*fy+fr,
                                        y == 0 ? 1
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

                    if(color && actual_fx < fx)
                    {
                        // Make brighter to compensate for black 9th column
                        color = Mix(0xFFFFFF, color, fx-actual_fx,actual_fx, fx);
                    }

                    if(fc >= sb_begin && fc < sb_end && (((x*width+fc)^(y*fy+fr))&1))
                        color ^= sb_xormask;

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

void Window::LineSetRenderSize(unsigned val)
{
    for(std::size_t x=0; x<xsize; ++x)
        cells[cursy*xsize+x].render_size = val;
}
