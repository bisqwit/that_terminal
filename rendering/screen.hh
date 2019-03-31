#ifndef bqtRenderingScreenHH
#define bqtRenderingScreenHH

#include <cstdint>
#include <vector>

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
    bool                dirty=true;
};

struct Window
{
    std::vector<Cell> cells;
    std::size_t       xsize, ysize;
    std::size_t       cursx=0, cursy=0;
    Cell blank {};
public:
    Window(std::size_t xs, std::size_t ys) : cells(xs*ys), xsize(xs), ysize(ys)
    {
    }

    void fillbox(std::size_t x, std::size_t y, std::size_t width, std::size_t height)
    {
        fillbox(x,y,width,height, blank);
    }
    void fillbox(std::size_t x, std::size_t y, std::size_t width, std::size_t height,
                 Cell with)
    {
        /*TODO*/
    }
    void copytext(std::size_t tgtx,std::size_t tgty, std::size_t srcx,std::size_t srcy,
                                                     std::size_t width,std::size_t height)
    {
        /*TODO*/
    }
    void PutCh(std::size_t x, std::size_t y, char32_t c, int cset = 0)
    {
        /*TODO*/
    }
};

#endif
