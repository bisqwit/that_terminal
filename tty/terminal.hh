#ifndef bqtTTYterminalHH
#define bqtTTYterminalHH
/** @file tty/terminal.hh
 * @brief Defines TerminalWindow, the bulk of the terminal emulator. Its main purpose is to render anything that is printed by the subprocess.
 */

#include <deque>
#include <string>
#include <vector>
#include <array>

#include "window.hh"

/** A terminal emulator. It is always attached to some instance of Window. */
class TerminalWindow
{
private:
    Window& wnd;                     ///< The reference to Window instance.

public:
    std::deque<char32_t> OutBuffer;  ///< Outgoing symbols (some ANSI codes cause input to be generated)

private:
    std::size_t top;                 ///< Current top row
    std::size_t bottom;              ///< Current bottom row

    struct backup                    ///< The backup handled by SaveCur() and RestoreCur()
    {
        std::size_t cx; ///< Cursor X position
        std::size_t cy; ///< Cursor Y position
        Cell attr;      ///< Attributes
    } backup;

    std::array<unsigned char,4> gset; ///< Graphics set attributes: Changed by SCS, i.e. "esc(" and "esc)"
    unsigned char activeset;          ///< Identifies current index into gset to use for character set translation
    unsigned char utfmode;            ///< UTF-8 mode flag (set by "esc%@" and "esc%8", does not do anything for now)
    unsigned char scs;                ///< Index into gset to be changed in SCS commands.

    char32_t lastch = U' ';           ///< Last printed character. Required by CSI b.

    std::vector<unsigned> p;          ///< Parameters from the current escape mode

    unsigned       state=0;           ///< State index used in ANSI code parsing
    bool           edgeflag=false;    ///< Flag that is set when something is printed at rightmost column
    std::u32string string;            ///< String that is collected in DCS, OSC, PM, APC and SOS commands

private:
    void ResetFG();               ///< Resets the Window's fgcolor into whatever is default fgcolor in Cell
    void ResetBG();               ///< Resets the Window's bgcolor into whatever is default bgcolor in Cell
    void ResetAttr();             ///< Resets the Window's attributes into defaults, but keeps protect flag.
    /** Resets all attributes and states. If @param full is true, also clears screen and repositions cursor. */
    void Reset(bool full = true);

    /** Performs down-scrolling within a section of screen.
     * @param y1 First line (inclusive)
     * @param y2 Last line (inclusive)
     * @param amount Number of lines to scroll. Non-positive values are ignored.
     * The top of the window is filled with @amount lines of blank using Window.FillBox.
     */
    void YScrollDown(unsigned y1, unsigned y2, int amount) const;

    /** Performs top-scrolling within a section of screen.
     * @param y1 First line (inclusive)
     * @param y2 Last line (inclusive)
     * @param amount Number of lines to scroll. Non-positive values are ignored.
     * The bottom of the window is filled with @amount lines of blank using Window.FillBox.
     */
    void YScrollUp(unsigned y1, unsigned y2, int amount) const;

    void SaveCur();                   ///< Implements the DECSC/ANSI_SC command
    void RestoreCur();                ///< Implements the DECRC/ANSI_RC command
    void EchoBack(std::u32string_view buffer); ///< Appends content into OutBuffer
public:
    /** Processes output (input from the terminal's perspective) from the subprocess.
     * Interprets ANSI codes.
     * @param s String to parse. Internal state allows ANSI codes to be split between
     * successive calls, i.e.
     * Write("abc") works identically to Write("a");Write("b");Write("c").
     */
    void Write(std::u32string_view s);

    /** Initializes the terminal window with reference to a @param w Window. */
    TerminalWindow(Window& w): wnd(w)
    {
        ResetAttr();
        Reset();
        SaveCur();
    }

    /** Resizes the terminal window to a different size. */
    void Resize(std::size_t newsx, std::size_t newsy);
};

#endif
