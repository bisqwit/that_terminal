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
    bool                dirty: 1;
    bool                double_width: 1;
    unsigned char       double_height:2;
    unsigned char       blink: 2;

    Cell()
    {
        std::memset(this, 0, sizeof(Cell));
        fgcolor = 0xAAAAAA;
        bgcolor = 0x000000;
        ch      = U' ';
        dirty   = true;
    }

    // operator== compares everything except the dirty-flag.
    bool operator== (const Cell& b) const
    {
        return std::tuple(fgcolor,bgcolor,ch,bold,dim,italic,
                          underline,underline2,overstrike,inverse,blink,
                          framed,encircled,fraktur,conceal,overlined,
                          double_width,double_height)
            == std::tuple(b.fgcolor,b.bgcolor,b.ch,b.bold,b.dim,b.italic,
                          b.underline,b.underline2,b.overstrike,b.inverse,b.blink,
                          b.framed,b.encircled,b.fraktur,b.conceal,b.overlined,
                          b.double_width,b.double_height);
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

    void fillbox(std::size_t x, std::size_t y, std::size_t width, std::size_t height)
    {
        fillbox(x,y,width,height, blank.ch);
    }
    template<typename T>
    void fillbox(std::size_t x, std::size_t y, std::size_t width, std::size_t height,
                 T with)
    {
        for(std::size_t h=0; h<height; ++h)
            for(std::size_t w=0; w<width; ++w)
                PutCh(x+w, y+h, with);
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
        ch.double_width = cells[y*xsize+x].double_width;
        ch.double_height = cells[y*xsize+x].double_height;
        /*if(c != U' ')
        {
            fprintf(stderr, "Ch at (%zu,%zu): <%c>\n", x,y, int(c));
        }*/
        PutCh(x, y, ch);
    }
    void Render(std::size_t fx, std::size_t fy, std::uint32_t* pixels, unsigned timer);
    void Resize(std::size_t newsx, std::size_t newsy);
    void Dirtify();

    void LineSetHeightAttr(unsigned);
    void LineSetWidthAttr(bool);
};

#endif
