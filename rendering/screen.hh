#ifndef bqtRenderingScreenHH
#define bqtRenderingScreenHH

#include <cstdint>
#include <vector>
#include <tuple>

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
    //    reverse
    std::uint_least32_t fgcolor=0xAAAAAA;
    std::uint_least32_t bgcolor=0x000000;
    char32_t            ch = U' ';
    bool                bold = false;
    bool                dim = false;
    bool                italic = false;
    bool                underline = false;
    bool                underline2 = false;
    bool                overstrike = false;
    bool                reverse = false;
    bool                blink = false;
    bool                framed = false;
    bool                encircled = false;
    bool                overlined = false;
    bool                fraktur = false;
    bool                conceal = false;
    bool                dirty = true;

    bool operator== (const Cell& b) const
    {
        return std::tuple(fgcolor,bgcolor,ch,bold,dim,italic,
                          underline,underline2,overstrike,reverse,blink,
                          framed,encircled,fraktur,conceal,overlined)
            == std::tuple(b.fgcolor,b.bgcolor,b.ch,b.bold,b.dim,b.italic,
                          b.underline,b.underline2,b.overstrike,b.reverse,b.blink,
                          b.framed,b.encircled,b.fraktur,b.conceal,b.overlined);
    }
    bool operator!= (const Cell& b) const { return !operator==(b); }
};

struct Window
{
    std::vector<Cell> cells;
    std::size_t       xsize, ysize;
    std::size_t       cursx=0, cursy=0;
    bool              reverse   = false;
    bool              cursorvis = true;
    Cell blank {};
private:
    std::size_t lastcursx, lastcursy;
public:
    Window(std::size_t xs, std::size_t ys) : cells(xs*ys), xsize(xs), ysize(ys)
    {
        Dirtify();
    }

    void fillbox(std::size_t x, std::size_t y, std::size_t width, std::size_t height)
    {
        fillbox(x,y,width,height, blank);
    }
    void fillbox(std::size_t x, std::size_t y, std::size_t width, std::size_t height,
                 Cell with)
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
        Cell ch = blank;
        // TODO: Deal with cset
        ch.ch = c;
        /*if(c != U' ')
        {
            fprintf(stderr, "Ch at (%zu,%zu): <%c>\n", x,y, int(c));
        }*/
        PutCh(x, y, ch);
    }
    void Render(std::size_t fx, std::size_t fy, std::uint32_t* pixels);
    void Resize(std::size_t newsx, std::size_t newsy);
    void Dirtify();
};

#endif
