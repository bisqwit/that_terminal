#ifndef bqtCellHH
#define bqtCellHH
/** @file rendering/cell.hh
 * @brief Defines Cell, the structure that represents a character on screen.
 */
#include <cstdint>
#include <cstring>

/** Elements in a screen buffer are called cells.
 * Cell is a structure that can be memcpy'd verbatim.
 * Its properties can be assigned and read directly.
 *
 * It includes:
 *
 *    ch      The character code   (unicode character index)
 *    fgcolor The foreground color (RGB)
 *    bgcolor The background color (RGB)
 *
 *    Attributes, defined as in SGR of ANSI:
 *
 *       bold, dim, italic, underline, underline2,
 *       overstrike, inverse, framed, encircled,
 *       overlined, fraktur, conceal, proportional,
 *       ideo_underline, ideo_underline2, ideo_overline,
 *       ideo_stress
 *       render_size: 0=normal,1=doublewidth,2=doublewidth+topline,3=doublewidth+bottomline
 *       blink:       0=none, 1=blink, 2=faster blink, 3=undefined
 *       scriptsize:  0=normal, 1=superscript, 2=subscript
 *       protect (protected-flag as in ANSI)
 *
 * And a dirty-flag, used by screen renderer.
 *
 * Attributes and flags are defined as bit fields
 * with attention to alignment and total size of structure.
 *
 * Constructor and comparison operators are inlined for performance.
 */
struct Cell
{
    // 24-bit color foreground,background = 48 bits total
    // 32-bit character
    std::uint_least32_t fgcolor;            ///< foreground color
    std::uint_least32_t bgcolor;            ///< background color
    char32_t            ch;                 ///< the character symbol (unicode codepoint)
    bool                bold: 1;            ///< bit 0
    bool                dim: 1;             ///< bit 1
    bool                italic: 1;          ///< bit 2
    bool                underline: 1;       ///< bit 3
    bool                underline2: 1;      ///< bit 4
    bool                overstrike: 1;      ///< bit 5
    bool                inverse: 1;         ///< bit 6
    bool                framed: 1;          ///< bit 7
    bool                encircled: 1;       ///< bit 8
    bool                overlined: 1;       ///< bit 9
    bool                fraktur: 1;         ///< bit 10
    bool                conceal: 1;         ///< bit 11
    unsigned char       render_size: 2;     ///< bit 12-13 0=normal,1=doublewidth,2=doublewidth+topline,3=doublewidth+bottomline
    unsigned char       blink: 2;           ///< bit 14-15

    unsigned char       scriptsize: 2;      ///< bit 16-17: 0=normal,1=superscript,2=subscript
    bool                proportional: 1;    ///< bit 18
    bool                ideo_underline:  1; ///< bit 19
    bool                ideo_underline2: 1; ///< bit 20
    bool                ideo_overline:  1;  ///< bit 21
    bool                ideo_overline2: 1;  ///< bit 22
    bool                ideo_stress: 1;     ///< bit 23
    unsigned padding: 6;                    ///< bit 24,25,26, 27,28,29

    bool                protect: 1;         ///< bit 30
    bool                dirty: 1;           ///< bit 31 (placed last for efficient access)

    /* Total size: 4 * 32 bits = 128 bits (16 bytes) */

    /* Constructor: Initializes the object with default attributes.
     * Sets the character as a space, color as light-gray on black, and the dirty flag.
     */
    Cell()
    {
        std::memset(this, 0, sizeof(Cell));
        fgcolor = 0xCCCCCC;
        bgcolor = 0x000000;
        ch      = U' ';
        dirty   = true;
    }

    /** operator==: Equality comparison
     * Compares everything except dirty and protect.
     * @param b The other cell to compare against.
     * @returns True if the cells are equivalent.
     */
    bool operator== (const Cell& b) const
    {
        Cell tmp1 = *this;
        Cell tmp2 = b;
        tmp1.dirty = false; tmp1.protect = false;
        tmp2.dirty = false; tmp2.protect = false;
        return std::memcmp(&tmp1, &tmp2, sizeof(Cell)) == 0;
        /*
        return std::tuple(fgcolor,bgcolor,ch,bold,dim,italic,
                          underline,underline2,overstrike,inverse,blink,
                          framed,encircled,fraktur,conceal,overlined,
                          scriptsize,proportional,
                          ideo_underline,ideo_underline2,ideo_overline,ideo_overline2,ideo_stress,
                          render_size)
            == std::tuple(b.fgcolor,b.bgcolor,b.ch,b.bold,b.dim,b.italic,
                          b.underline,b.underline2,b.overstrike,b.inverse,b.blink,
                          b.framed,b.encircled,b.fraktur,b.conceal,b.overlined,
                          b.scriptsize,b.proportional,
                          b.ideo_underline,b.ideo_underline2,b.ideo_overline,b.ideo_overline2,b.ideo_stress,
                          b.render_size);
        */
    }

    /** operator!=. Simply inverses operator==.
     * @param b The other cell to compare against.
     * @returns True if the cells are not equivalent.
     */
    bool operator!= (const Cell& b) const
    {
        return !operator==(b);
    }
};

#endif
