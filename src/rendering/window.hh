#ifndef bqtRenderingScreenHH
#define bqtRenderingScreenHH
/** @file rendering/window.hh
 * @brief Defines Window, a renderer and manager of screen contents.
 */

#include <cstdint>
#include <vector>
#include <tuple>
#include <cstring>

#include "cset.hh"
#include "cell.hh"

/** Window: A two-dimensional storage for cells. */
struct Window
{
    std::vector<Cell> cells; ///< Storage for cells. Indexed row-first. Size: xsize*ysize */
    std::size_t       xsize; ///< Width of window in cells
    std::size_t       ysize; ///< Height of window in cells
    std::size_t       cursx=0; ///< Cursor location (horizontal)
    std::size_t       cursy=0; ///< Cursor location (vertical)
    bool              inverse   = false;            ///< Whether screen-wide inverse effect is in effect
    bool              cursorvis = true;             ///< Whether cursor is visible
    unsigned          cursorcolor = 0xFFFFFF;       ///< Color of cursor
    unsigned          mousecolor1 = 0xFFFFFF;       ///< Ignored
    unsigned          mousecolor2 = 0xFFFFFF;       ///< Ignored
    unsigned          mouseselectcolor = 0xFFFFFF;  ///< Ignored
    Cell blank {}; ///< The current "blank" cell. It is used as the model cell for inserting empty rows.
private:
    std::size_t lastcursx, lastcursy;
    unsigned    lasttimer=0;
public:
    /** Initializes the window to the given size. Each cell is default-initialized. */
    Window(std::size_t xs, std::size_t ys) : cells(xs*ys), xsize(xs), ysize(ys)
    {
        Dirtify();
    }

    /** Used in scrolling: Fills the given region with the blank character. */
    void FillBox(std::size_t x, std::size_t y, std::size_t width, std::size_t height)
    {
        for(std::size_t h=0; h<height; ++h)
            for(std::size_t w=0; w<width; ++w)
            {
                auto tx = x+w, ty = y+h;
                PutCh(tx, ty, blank);
            }
    }
    /** Used in erasing: Fills the given region with the given character,
     * but does not change protected cells.
     */
    void FillBox(std::size_t x, std::size_t y, std::size_t width, std::size_t height, Cell with)
    {
        for(std::size_t h=0; h<height; ++h)
            for(std::size_t w=0; w<width; ++w)
            {
                auto tx = x+w, ty = y+h;
                if(with.ch != blank.ch || !cells[ty*xsize+tx].protect)
                    PutCh(tx, ty, with);
            }
    }
    /** Copies a rectangle from given coordinates to the target coordinates.
     * @param tgtx Target X-coordinate
     * @param tgty Target Y-coordinate
     * @param srcx Source X-coordinate
     * @param srcy Source Y-coordinate
     * @param width Width of region to copy
     * @param height Height of region to copy
     */
    void CopyText(std::size_t tgtx,std::size_t tgty, std::size_t srcx,std::size_t srcy,
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

    /** Marks the given cell as dirty and places an invalid character there. */
    void Dirtify(std::size_t x, std::size_t y)
    {
        auto& tgt = cells[y*xsize+x];
        // Write an invalid character, to make sure it gets properly
        // cleared when a valid character gets written instead.
        // This glyph must _not_ register as doublewidth.
        tgt.ch    = 0xFFFE;
        tgt.dirty = true;
    }
    /** Marks entire screen as dirty (without changing ch)
     * and forgets the cursor's last known position.
      */
    void Dirtify();

    /** Places a cell at the given position on screen.
     * If the cell changed, it is marked dirty.
     */
    void PutCh(std::size_t x, std::size_t y, const Cell& c)
    {
        auto& tgt = cells[y*xsize+x];
        if(tgt != c)
        {
            tgt = c;
            tgt.dirty = true;
        }
    }

    /** Places a character at the given position on screen.
     * Attributes are copied from the blank cell,
     * except render_size which is kept unchanged.
     */
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
    /** Same as PutCh(x,y,c,cset), but instead of taking attributes from the blank
     * cell, preserves existing attributes on screen.
     */
    void PutCh_KeepAttr(std::size_t x, std::size_t y, char32_t c, int cset = 0)
    {
        auto& cell = cells[y*xsize+x];
        Cell temp = cell;
        if(cset)
            c = TranslateCSet(c, cset);
        if(temp != cell)
        {
            cell = temp;
            cell.dirty = true;
        }
    }
    /** Same as PutCh(x,y,c), but preserves existing character symbol on screen. */
    void PutCh_KeepChar(std::size_t x, std::size_t y, const Cell& c)
    {
        auto& cell = cells[y*xsize+x];
        Cell temp = c;
        temp.ch = cell.ch;
        if(temp != cell)
        {
            cell = temp;
            cell.dirty = true;
        }
    }

    /** Renders the screen into a pixel buffer using given font size.
     * Only changed regions are rendered. Variables that are used to optimize
     * the rendering (to save work) are the dirty flag in cells, lastcurs, lastcury and lasttimer.
     *
     * @param fx Font width in pixels.
     * @param fy Font height in pixels.
     * @param pixels Target buffer which must have room for at least fx*xsize*fy*ysize pixels.
     */
    void Render(std::size_t fx, std::size_t fy, std::uint32_t* pixels);

    /** Resizes the window to the new size, keeping existing contents.
     * The entire screen is marked dirty. If the cursor is outside
     * the new boundaries of the window, it is placed in the bottom row
     * or rightmost column where necessary to keep it inside the boundaries.
     */
    void Resize(std::size_t newsx, std::size_t newsy);

    /** Changes the render_size attribute on current cursor line (cursy)
     * to the specified value.
     * @param val Value to be copied to the render_size attribute on the cells of that row.
     *            0=normal, 1=doublewidth, 2=doublewidth+topline, 3=doublewidth+bottomline
     */
    void LineSetRenderSize(unsigned val);
};

#endif
