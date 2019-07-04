#ifndef bqtRenderingScreenHH
#define bqtRenderingScreenHH

#include <cstdint>
#include <vector>
#include <tuple>
#include <cstring>

#include "cset.hh"

struct Cell
{
    // 24-bit color foreground,background = 48 bits total
    // 32-bit character
    // attributes:
    //    italic
    //    bold
    //    underline
    //    double underline
    //    dim
    //    overstrike
    //    inverse
    std::uint_least32_t fgcolor;
    std::uint_least32_t bgcolor;
    char32_t            ch;
    bool                bold: 1;
    bool                dim: 1;
    bool                italic: 1;
    bool                underline: 1;
    bool                underline2: 1;
    bool                overstrike: 1;
    bool                inverse: 1;
    bool                framed: 1;
    bool                encircled: 1;
    bool                overlined: 1;
    bool                fraktur: 1;
    bool                conceal: 1;
    unsigned char       render_size: 2; // 0=normal,1=doublewidth,2=doublewidth+topline,3=doublewidth+bottomline
    unsigned char       blink: 2;

    bool                dirty: 1;
    bool                protect: 1;

    Cell()
    {
        std::memset(this, 0, sizeof(Cell));
        fgcolor = 0xCCCCCC;
        bgcolor = 0x000000;
        ch      = U' ';
        dirty   = true;
    }

    // operator== compares everything except dirty and protect.
    bool operator== (const Cell& b) const
    {
        return std::tuple(fgcolor,bgcolor,ch,bold,dim,italic,
                          underline,underline2,overstrike,inverse,blink,
                          framed,encircled,fraktur,conceal,overlined,
                          render_size)
            == std::tuple(b.fgcolor,b.bgcolor,b.ch,b.bold,b.dim,b.italic,
                          b.underline,b.underline2,b.overstrike,b.inverse,b.blink,
                          b.framed,b.encircled,b.fraktur,b.conceal,b.overlined,
                          b.render_size);
    }
    bool operator!= (const Cell& b) const { return !operator==(b); }
};

struct Window
{
    std::vector<Cell> cells;
    std::size_t       xsize, ysize;
    std::size_t       cursx=0, cursy=0;
    bool              inverse   = false;
    bool              cursorvis = true;
    unsigned          cursorcolor = 0xFFFFFF;
    unsigned          mousecolor1 = 0xFFFFFF, mousecolor2 = 0xFFFFFF, mouseselectcolor = 0xFFFFFF;
    Cell blank {};
private:
    std::size_t lastcursx, lastcursy;
    unsigned    lasttimer=0;
public:
    Window(std::size_t xs, std::size_t ys) : cells(xs*ys), xsize(xs), ysize(ys)
    {
        Dirtify();
    }

    // fillbox used in scrolling:
    void fillbox(std::size_t x, std::size_t y, std::size_t width, std::size_t height)
    {
        for(std::size_t h=0; h<height; ++h)
            for(std::size_t w=0; w<width; ++w)
            {
                auto tx = x+w, ty = y+h;
                PutCh(tx, ty, blank);
            }
    }
    // fillbox used in erasing:
    void fillbox(std::size_t x, std::size_t y, std::size_t width, std::size_t height, Cell with)
    {
        for(std::size_t h=0; h<height; ++h)
            for(std::size_t w=0; w<width; ++w)
            {
                auto tx = x+w, ty = y+h;
                if(with.ch != blank.ch || !cells[ty*xsize+tx].protect)
                    PutCh(tx, ty, with);
            }
    }
    void copytext(std::size_t tgtx,std::size_t tgty, std::size_t srcx,std::size_t srcy,
                                                     std::size_t width,std::size_t height)
    {
        auto hcopy_oneline = [&](std::size_t ty, std::size_t sy)
        {
            if(tgtx < srcx)
                for(std::size_t w=0; w<width; ++w)
                    PutCh(tgtx+w, ty, cells[sy*xsize+(srcx+w)]);
            else
                for(std::size_t w=width; w-- > 0; )
                    PutCh(tgtx+w, ty, cells[sy*xsize+(srcx+w)]);
        };
        if(tgty < srcy)
            for(std::size_t h=0; h<height; ++h)
                hcopy_oneline(tgty+h, srcy+h);
        else
            for(std::size_t h=height; h-- > 0; )
                hcopy_oneline(tgty+h, srcy+h);
    }
    void PutCh(std::size_t x, std::size_t y, const Cell& c)
    {
        auto& tgt = cells[y*xsize+x];
        if(tgt != c)
        {
            tgt = c;
            tgt.dirty = true;
        }
    }
    void PutCh(std::size_t x, std::size_t y, char32_t c, int cset = 0)
    {
        /*
        cset options:
        0 = ascii
        1 = dec graphics
        */
        Cell ch = blank;
        if(cset)
        {
            c = TranslateCSet(c, cset);
        }
        ch.ch = c;
        ch.render_size = cells[y*xsize+x].render_size;
        /*if(c != U' ')
        {
            fprintf(stderr, "Ch at (%zu,%zu): <%c>\n", x,y, int(c));
        }*/
        PutCh(x, y, ch);
    }
    void Render(std::size_t fx, std::size_t fy, std::uint32_t* pixels, unsigned timer);
    void Resize(std::size_t newsx, std::size_t newsy);
    void Dirtify();

    void LineSetRenderSize(unsigned);
};

#endif
