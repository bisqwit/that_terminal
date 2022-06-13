#ifdef RUN_TESTS
# include <gtest/gtest.h>
#endif
/** @file tty/terminal.cc
 * @brief Defines TerminalWindow, the bulk of the terminal emulator. Its main purpose is to render anything that is printed by the subprocess.
 */

#include <array>
#include <cstdio> // sprintf

#include <SDL.h> // for window title

#include "terminal.hh"
#include "ctype.hh"
#include "256color.hh"
#include "color.hh"
#include "ui.hh"

void TerminalWindow::ResetAttr()
{
    bool prot = wnd.blank.protect;
    wnd.blank = Cell{};
    wnd.blank.protect = prot;
}
void TerminalWindow::Reset(bool full)
{
    top    = 0;
    bottom = wnd.ysize-1;

    gset = {0,0,0,0}; activeset = 0; scs = 0;
    utfmode = 0;
    lastch = U' ';

    state = 0;
    p.clear();

    wnd.inverse   = false;
    wnd.cursorvis = true;
    if(full)
    {
        edgeflag = false;
        wnd.cursx = 0;
        wnd.cursy = 0;
        wnd.FillBox(0,0, wnd.xsize,wnd.ysize, wnd.blank); // Clear screen
    }
}

void TerminalWindow::YScrollDown(unsigned y1, unsigned y2, int amount) const
{
    if(amount <= 0) return;
    unsigned hei = y2-y1+1;
    if(unsigned(amount) > hei) amount = hei;
    //fprintf(stderr, "Height=%d, amount=%d, scrolling DOWN by %d lines\n", hei,amount, hei-amount);
    wnd.CopyText(0,y1+amount, 0,y1, wnd.xsize,hei-amount);
    wnd.FillBox(0,y1, wnd.xsize,amount);
}

void TerminalWindow::YScrollUp(unsigned y1, unsigned y2, int amount) const
{
    if(amount <= 0) return;
    unsigned hei = y2-y1+1;
    if(unsigned(amount) > hei) amount = hei;
    //fprintf(stderr, "Height=%d, amount=%d, scrolling UP by %d lines\n", hei,amount, hei-amount);
    wnd.CopyText(0,y1, 0,y1+amount, wnd.xsize,hei-amount);
    wnd.FillBox(0,y2-amount+1, wnd.xsize,amount);
}

void TerminalWindow::Write(std::u32string_view s)
{
    unsigned color = 0;
    /** Repositions cursor horizontally and ensures the new location is within allowed range. */
    auto ClampedMoveX = [&](int tgtx)
    {
        wnd.cursx = std::min(std::size_t(std::max(0,tgtx)), wnd.xsize-1);
        edgeflag = false;
    };
    /** Repositions cursor vertically and ensures the new location is within allowed range.
     * param tgty = Target y coordinate
     * param strict = If set, only permits moving inside current window; otherwise permits moving anywhere on screen.
     */
    auto ClampedMoveY = [&](int tgty, bool strict = true)
    {
        if(wnd.cursy >= top && wnd.cursy <= bottom && strict)
        {
            // Only permit moving inside window
            wnd.cursy = std::min(std::size_t(std::max(int(top), tgty)), bottom);
        }
        else
        {
            // Permit moving anywhere
            wnd.cursy = std::min(std::size_t(std::max(0,tgty)), wnd.ysize-1);
        }
    };
    /** Combination of ClampedMoveX and ClampedMoveY. */
    auto ClampedMove = [&](int tgtx, int tgty, bool strict = true)
    {
        ClampedMoveX(tgtx);
        ClampedMoveY(tgty, strict);
    };
    /** Performs line feed. */
    auto Lf = [&]
    {
        if(wnd.cursy == bottom)
            YScrollUp(top, bottom, 1);
        else
            ClampedMoveY(wnd.cursy+1);
    };
    /** Performs typewriter write for one character. */
    auto PutC = [&](char32_t c, bool doublewidth)
    {
        if(edgeflag)
        {
            if(wnd.cursx == wnd.xsize-1)
            {
                Lf();
                wnd.cursx = 0;
            }
            edgeflag = false;
        }
        wnd.PutCh(wnd.cursx,wnd.cursy, c, gset[activeset]);
        if(wnd.cursx == wnd.xsize-1) edgeflag = true;
        else                         ++wnd.cursx;

        if(doublewidth)
        {
            // If this character was double-width,
            // skip the next column but mark it also dirty.
            wnd.Dirtify(wnd.cursx,wnd.cursy);
            if(wnd.cursx == wnd.xsize-1) edgeflag = true;
            else                         ++wnd.cursx;
        }
    };

    enum : unsigned {
        MODE38_2 = 70,  MODE38_2_x1, MODE38_2_x2, MODE_38_x = 108,
        MODE38_3 = 80,  MODE38_3_x1, MODE38_3_x2, MODE38_5 = 56,
        MODE48_2 = 87,  MODE48_2_x1, MODE48_2_x2, MODE_48_x = 109,
        MODE48_3 = 111, MODE48_3_x1, MODE48_3_x2, MODE48_5 = 57,
        MODE58_2 = 114, MODE58_2_x1, MODE58_2_x2, MODE_58_x = 110,
        MODE58_3 = 117, MODE58_3_x1, MODE58_3_x2, MODE58_5 = 99,
        MODE38_4 = 66,  MODE38_4_x1, MODE38_4_x2, MODE38_4_x3,
        MODE48_4 = 76,  MODE48_4_x1, MODE48_4_x2, MODE48_4_x3,
        MODE58_4 = 83,  MODE58_4_x1, MODE58_4_x2, MODE58_4_x3,
    };
    static_assert(MODE38_4_x1 == MODE38_4+1);    static_assert(MODE38_3_x1 == MODE38_3+1);    static_assert(MODE38_2_x1 == MODE38_2+1);
    static_assert(MODE48_4_x1 == MODE48_4+1);    static_assert(MODE48_3_x1 == MODE48_3+1);    static_assert(MODE48_2_x1 == MODE48_2+1);
    static_assert(MODE58_4_x1 == MODE58_4+1);    static_assert(MODE58_3_x1 == MODE58_3+1);    static_assert(MODE58_2_x1 == MODE58_2+1);
    static_assert(MODE38_4_x2 == MODE38_4_x1+1); static_assert(MODE38_3_x2 == MODE38_3_x1+1); static_assert(MODE38_2_x2 == MODE38_2_x1+1);
    static_assert(MODE48_4_x2 == MODE48_4_x1+1); static_assert(MODE48_3_x2 == MODE48_3_x1+1); static_assert(MODE48_2_x2 == MODE48_2_x1+1);
    static_assert(MODE58_4_x2 == MODE58_4_x1+1); static_assert(MODE58_3_x2 == MODE58_3_x1+1); static_assert(MODE58_2_x2 == MODE58_2_x1+1);
    static_assert(MODE38_4_x3 == MODE38_4_x2+1);
    static_assert(MODE48_4_x3 == MODE48_4_x2+1);
    static_assert(MODE58_4_x3 == MODE58_4_x2+1);
    static_assert(MODE_48_x == MODE_38_x+1); static_assert(MODE_58_x == MODE_48_x+1);
    /* Translation maps for ProcessSGR for a smaller jump table */
    auto M = [](unsigned n) constexpr -> unsigned char
    {
        // 16? 36, 37?
        unsigned char result = 0;
        // SKIP the following, as they are not implemented:
        //   8 (conceal)
        //  20 (fraktur)
        //  26 (proportional)
        //  28 (clear conceal)
        //  50 (clear proportional)
        //  51-52 (framed & encircled)
        //  54 (clear framed & encircled)
        //  59 (default underline color)
        //  60-65 (ideogram settings)
        //  73-75 (superscript & subscript settings)
        // This makes the code smaller. They can be enabled when implemented.
        //
        // Put most common things first
        if(/*n >= 0 &&*/ n <= 1) return result+n-0; else result += 2;
        //
        if(n >= 30 && n <= 37) return result; else result+=1; // 2 -- handled by a single case
        if(n >= 90 && n <= 97) return result; else result+=1; // 3 -- handled by a single case
        if(n == MODE38_5)      return result; else result+=1; // 4 -- fallback from above
        if(n == MODE38_2_x2)   return result; else result+=1; // 5 -- uncommon, but works as fallback from MODE38_5
        if(n == MODE38_3_x2)   return result; else result+=1; // 6
        if(n == MODE38_4_x3)   return result; else result+=1; // 7
        //
        if(n >= 40 && n <= 47) return result; else result+=1; // 8 -- handled by a single case
        if(n >=100 && n <=107) return result; else result+=1; // 9 -- handled by a single case
        if(n == MODE48_5)      return result; else result+=1; // 10 -- fallback from above
        if(n == MODE48_2_x2)   return result; else result+=1; // 11 -- uncommon, but works as fallback from MODE48_5
        if(n == MODE48_3_x2)   return result; else result+=1; // 12
        if(n == MODE48_4_x3)   return result; else result+=1; // 13
        //
        if(n == 38)            return result; else result+=1; // 14 -- sets MODE_38_x
        if(n == 48)            return result; else result+=1; // 15 -- sets MODE_48_x
        if(n == 58)            return result; else result+=1; // 16 -- sets MODE_58_x
        if(n == MODE_38_x
        || n == MODE_48_x
        || n == MODE_58_x)     return result; else result+=1; // 17 -- handled by a single case
        //
        if((n >= MODE38_4 && n <= MODE38_4_x2)
        || (n >= MODE38_3 && n <= MODE38_3_x1)
        || (n >= MODE38_2 && n <= MODE38_2_x1)
        || (n >= MODE48_4 && n <= MODE48_4_x2)
        || (n >= MODE48_3 && n <= MODE48_3_x1)
        || (n >= MODE48_2 && n <= MODE48_2_x1)
        || (n >= MODE58_4 && n <= MODE58_4_x2)
        || (n >= MODE58_3 && n <= MODE58_3_x1)
        || (n >= MODE58_2 && n <= MODE58_2_x1)) return result; else result+=1; // 18 -- handled by a single case
        // Skip these, as they are not implemented:
        //if(n == MODE58_4_x3)                  return result; else result+=1;
        //if(n == MODE58_3_x2)                  return result; else result+=1;
        //if(n == MODE58_2_x2)                  return result; else result+=1;
        //if(n == MODE58_5)                     return result; else result+=1;
        //
        if(n == 39)            return result; else result+=1; // 19
        if(n == 49)            return result; else result+=1; // 20
        //
        if(n == 9)             return result; else result+=1; // 21
        if(n >= 2 && n <= 7)   return result+n-2; else result += 6; // 22..27
        //
        if(n == 27)            return result; else result+=1; // 28
        if(n == 29)            return result; else result+=1; // 29
        if(n == 53)            return result; else result+=1; // 30
        if(n >= 21 && n <= 25) return result+n-21; else result+=5; // 31..35
        if(n == 55)            return result; else result+=1; // 36
        //
        return result;
    };
    static constexpr unsigned char __ = M(~0u);
    static constexpr unsigned char translate[] =
    {
        #define a(n) M(n+0),M(n+1),M(n+2),M(n+3),M(n+4),M(n+5),M(n+6),M(n+7),M(n+8),M(n+9),
        a(0)a(10)a(20)a(30)a(40)a(50)a(60)a(70)a(80)a(90)a(100)a(110)
        #undef a
    };
    static constexpr unsigned char modetab[] = {MODE38_2,MODE38_3,MODE38_4,MODE38_5,
                                                MODE48_2,MODE48_3,MODE48_4,MODE48_5,
                                                MODE58_2,MODE58_3,MODE58_4,MODE58_5};

    /** ProcessSGR processes a SGR command. */
    auto ProcessSGR = [&](char32_t& c, unsigned a,
                          auto&& reset_attr,
                          auto&& change_attr)
    {
        /* c is a state code internal to this function, a is the input number from the SGR code.
         * Codes:
         *   0 = default state -- Will parse "a" (input number) as verbatim
         *   Other, see below:
         * Switch-case map (* = regular use, _ = UDEFINED, # = internal use):
         *     0-9     **********
         *    10-19    __________ (font commands, 0=default, 1-9=alt font)
         *    20-29    **********
         *    30-39    **********
         *    40-49    **********
         *    50-59    ******##**    56: MODE38_5[1]  57: MODE48_5[1]
         *    60-69    ******####    66: MODE38_4[4]
         *    70-79    ###***####    70: MODE38_2[3]  76: MODE48_4[4]
         *    80-89    ##########    80: MODE38_3[3]  83: MODE58_4[4] 87: MODE48_2[3]
         *    90-99    ********_#    99: MODE58_5[1]
         *   100-109   ********##    108: MODE_38_x[3]
         *   110-119   ##########    111: MODE48_3[3] 114: MODE58_2[3] 117: MODE58_3[3]
         *
         * Detailed layout on internal use codes --- what happens if "a" has that same value:
         *    56               mistakenly interpreted as 38;5;56
         *    57               mistakenly interpreted as 48;5;57
         *    99               mistakenly interpreted as 58;5;99
         *    72               mistakenly interpreted as 38;2;?;?;72
         *    82               mistakenly interpreted as 38;3;?;?;82
         *    89               mistakenly interpreted as 48;2;?;?;89
         *   113               mistakenly interpreted as 48;3;?;?;113
         *   116               mistakenly interpreted as 58;2;?;?;116
         *   119               mistakenly interpreted as 58;3;?;?;119
         *    69               mistakenly interpreted as 38;4;?;?;?;69
         *    79               mistakenly interpreted as 48;4;?;?;?;79
         *    86               mistakenly interpreted as 58;4;?;?;?;86
         *   108               mistakenly interpreted as 38;108 -- does nothing
         *   109               mistakenly interpreted as 48;109 -- does nothing
         *   110               mistakenly interpreted as 58;110 -- does nothing
         *   Anything else:    ignores 1 parameter and sets bold mode (as in 1)
         */
        unsigned switchval = c ? c : a;
        auto m = [](unsigned n) constexpr
        {
            return (n < sizeof(translate)) ? translate[n] : __;
        };
        switch(m(switchval))
        {
            #define set(what, v) change_attr([&](Cell& c, auto x){c.what = x;}, \
                                             [&](const Cell& c) { return c.what; }, \
                                             v)
            case m(0): reset_attr(); c = 0; break;
            case m(1): set(bold, true); c = 0; break;
            case m(2): set(dim, true); c = 0; break;
            case m(3): set(italic, true); c = 0; break;
            case m(4): set(underline, true); c = 0; break;
            case m(5): set(blink, 1); c = 0; break;
            case m(6): set(blink, 2); c = 0; break;
            case m(7): set(inverse, true); c = 0; break;
            //case m(8): set(conceal, true); c = 0; break;
            case m(9): set(overstrike, true); c = 0; break;
            //case m(20): set(fraktur, true); c = 0; break;
            case m(21): set(underline2, true); c = 0; break;
            case m(22): set(dim, false); set(bold, false); c = 0; break;
            case m(23): set(italic, false); set(fraktur, false); c = 0; break;
            case m(24): set(underline, false); set(underline2, false); c = 0; break;
            case m(25): set(blink, 0); c = 0; break;
            //case m(26): set(proportional, true); c = 0; break;
            case m(27): set(inverse, false); c = 0; break;
            //case m(28): set(conceal, false); c = 0; break;
            case m(29): set(overstrike, false); c = 0; break;
            case m(39): set(underline, false); set(underline2, false);
                        set(fgcolor, Cell{}.fgcolor); c = 0; break; // Set default foreground color
            case m(49): set(bgcolor, Cell{}.bgcolor); c = 0; break; // Set default background color
            //case m(59): c = 0; break; // Set default underline color
            //case m(50): set(proportional, false); c = 0; break;
            //case m(51): set(framed, true); c = 0; break;
            //case m(52): set(encircled, true); c = 0; break;
            case m(53): set(overlined, true); c = 0; break;
            //case m(54): set(framed, false); set(encircled, false); c = 0; break;
            case m(55): set(overlined, false); c = 0; break;
            //case m(60): set(ideo_underline, true); c = 0; break;
            //case m(61): set(ideo_underline2, true); c = 0; break;
            //case m(62): set(ideo_overline, true); c = 0; break;
            //case m(63): set(ideo_overline2, true); c = 0; break;
            //case m(64): set(ideo_stress, true); c = 0; break;
            //case m(65): set(ideo_underline, false); set(ideo_underline2, false);
            //            set(ideo_overline, false); set(ideo_overline2, false);
            //            set(ideo_stress, false); c = 0; break;
            //case m(73): set(scriptsize, 1); c = 0; break;
            //case m(74): set(scriptsize, 2); c = 0; break;
            //case m(75): set(scriptsize, 0); c = 0; break;
            case m(38): c = MODE_38_x; break;
            case m(48): c = MODE_48_x; break;
            case m(58): c = MODE_58_x; break;

            // MODE_38_x, MODE_48_x, MODE_58_x are handled by single case.
            case m(MODE_38_x):/*
            case m(MODE_48_x):
            case m(MODE_58_x):*/
            {
                color = 0;
                if(a >= 2 && a <= 5) { c = modetab[(c-MODE_38_x)*4 + a - 2]; break; }
                c     = 0;
                break;
            }

            // All of these are handled by a single case. They update color and increment mode number.
            case m(MODE38_4): /*//color = (color << 8) + a; c+=1; break; // 38;4;n
            case m(MODE38_3): //color = (color << 8) + a; c+=1; break; // 38;3;n
            case m(MODE38_2): //color = (color << 8) + a; c+=1; break; // 38;2;n
            case m(MODE48_4): //color = (color << 8) + a; c+=1; break; // 48;4;n
            case m(MODE48_3): //color = (color << 8) + a; c+=1; break; // 48;3;n
            case m(MODE48_2): //color = (color << 8) + a; c+=1; break; // 48;2;n
            case m(MODE58_4): //color = (color << 8) + a; c+=1; break; // 58;4;n
            case m(MODE58_3): //color = (color << 8) + a; c+=1; break; // 58;3;n
            case m(MODE58_2): //color = (color << 8) + a; c+=1; break; // 58;2;n
            case m(MODE38_4_x1): //color = (color << 8) + a; c+=1; break; // 38;4;#;n
            case m(MODE38_3_x1)://color = (color << 8) + a; c+=1; break; // 38;3;#;n
            case m(MODE38_2_x1)://color = (color << 8) + a; c+=1; break; // 38;2;#;n
            case m(MODE48_4_x1)://color = (color << 8) + a; c+=1; break; // 48;4;#;n
            case m(MODE48_3_x1)://color = (color << 8) + a; c+=1; break; // 48;3;#;n
            case m(MODE48_2_x1)://color = (color << 8) + a; c+=1; break; // 48;2;#;n
            case m(MODE58_4_x1)://color = (color << 8) + a; c+=1; break; // 58;4;#;n
            case m(MODE58_3_x1)://color = (color << 8) + a; c+=1; break; // 58;3;#;n
            case m(MODE58_2_x1)://color = (color << 8) + a; c+=1; break; // 58;2;#;n
            case m(MODE38_4_x2)://color = (color << 8) + a; c+=1; break; // 38;4;#;#;n
            case m(MODE48_4_x2)://color = (color << 8) + a; c+=1; break; // 48;4;#;#;n
            case m(MODE58_4_x2):*/color = (color << 8) + a; c+=1; break; // 58;4;#;#;n

            // Foreground colors. 30..37 are handled by a single case, likewise 90..97
            case m(30):/*case m(31):case m(32):case m(33):
            case m(34):case m(35):case m(36):case m(37):*/     a -= 30; a += 90-8;  [[fallthrough]];
            case m(90):/*case m(91):case m(92):case m(93):
            case m(94):case m(95):case m(96):case m(97):*/     a -= 90-8;           [[fallthrough]];
            case m(MODE38_5):     color = 0; a = xterm256table[a & 0xFF];           [[fallthrough]];     // 38;5;n
            case m(MODE38_2_x2):  color = (color << 8) + a; set(fgcolor, color); c = 0; break;           // 38;2;#;#;n   (RGB24)
            case m(MODE38_3_x2):  color = (color << 8) + a; set(fgcolor, cmy2rgb(color));  c = 0; break; // 38;3;#;#;n   (CMY->RGB)
            case m(MODE38_4_x3):  color = (color << 8) + a; set(fgcolor, cmyk2rgb(color)); c = 0; break; // 38;4;#;#;#;n (CMYK->RGB)

            // Background colors. 40..47 are handled by a single case, likewise 100..107
            case m(40):/*case m(41):case m(42):case m(43):
            case m(44):case m(45):case m(46):case m(47):*/     a -= 40; a += 100-8; [[fallthrough]];
            case m(100):/*case m(101):case m(102):case m(103):
            case m(104):case m(105):case m(106):case m(107):*/ a -= 100-8;          [[fallthrough]];
            case m(MODE48_5):     color = 0; a = xterm256table[a & 0xFF];           [[fallthrough]];     // 48;5;n
            case m(MODE48_2_x2):  color = (color << 8) + a; set(bgcolor, color);           c = 0; break; // 48;2;#;#;n   (RGB24)
            case m(MODE48_3_x2):  color = (color << 8) + a; set(bgcolor, cmy2rgb(color));  c = 0; break; // 48;3;#;#;n   (CMY->RGB)
            case m(MODE48_4_x3):  color = (color << 8) + a; set(bgcolor, cmyk2rgb(color)); c = 0; break; // 48;4;#;#;#;n (CMYK->RGB)

            // These don't do anything, so we just ignore them and reset the mode number c.
            //
            //case m(MODE58_5):  color = 0; a = xterm256table[a & 0xFF]; [[fallthrough]];           // 58;5;n
            //case m(MODE58_4_x3)://color = (color << 8) + a; /*IGNORED*/ c = 0; break;             // 58;4;#;#;#;n (TODO CMYK->RGB)
            //case m(MODE58_3_x2)://color = (color << 8) + a; /*IGNORED*/ c = 0; break;             // 58;3;#;#;n   (TODO CMY->RGB)
            //case m(MODE58_2_x2):  color = (color << 8) + a; /*IGNORED*/ c = 0; break;             // 58;2;#;#;n   (RGB24)

            default: case m(~0u): c = 0; break; // undefined
            #undef set
        }
    };

    if(bottom >= wnd.ysize) bottom = wnd.ysize-1;
    if(top    > bottom)     top = bottom;

    enum States: unsigned
    {
        st_default,
        st_esc,
        st_scs,         // esc ()*+-./
        st_scr,         // esc #
        st_esc_percent, // esc %
        st_csi,         // esc [
        st_csi_dec,     // csi ?
        st_csi_dec2,    // csi >
        st_csi_dec3,    // csi =
        st_csi_ex,      // csi !
        st_csi_quo,     // csi "
        st_csi_dol,     // csi $
        st_csi_star,    // csi *
        st_csi_dec_dol, // csi ? $
        st_string,
        st_string_str,
        //
        st_num_states
    };
    auto State = [](char32_t c, unsigned st) constexpr
    {
        return c*st_num_states + st;
    };
    auto GetParams = [&](unsigned min_params, bool change_zero_to_one)
    {
        if(p.size() < min_params) { p.resize(min_params); }
        if(change_zero_to_one) for(auto& v: p) if(!v) v = 1;
        state = st_default;
    };

    for(char32_t c: s)
    {
        switch(State(c,state))
        {
            #define CsiState(c)      State(c,st_csi):     case State(c,st_csi_dec2): \
                                case State(c,st_csi_dec): case State(c,st_csi_dec3): \
                                case State(c,st_string)
                                /* csi_quo, csi_dol, csi_dec_dol, csi_ex are excluded
                                 * from csistate because you can't have parameters
                                 * after this particular symbol.
                                 */
            #define AnyState(c)      State(c,st_default):    case State(c,st_esc): \
                                case State(c,st_scs):        case State(c,st_csi_ex): \
                                case State(c,st_string_str): case State(c,st_csi_quo): \
                                case State(c,st_scr):        case State(c,st_esc_percent): \
                                case State(c,st_csi_dol):    case State(c,st_csi_dec_dol): \
                                case CsiState(c)

            // Note: These escapes are recognized even in the middle of an ANSI/VT code.
            case AnyState(U'\b'):  { lastch=c; ClampedMoveX(wnd.cursx-1); break; }
            case AnyState(U'\t'):  { lastch=c; ClampedMoveX(wnd.cursx + 8 - (wnd.cursx & 7)); break; }
            case AnyState(U'\r'):  { lastch=c; ClampedMoveX(0); break; }
            case AnyState(U'\16'): { activeset = 1; break; }
            case AnyState(U'\17'): { activeset = 0; break; }
            case AnyState(U'\177'): { /* del - ignore */ break; }
            case AnyState(U'\30'): [[fallthrough]];
            case AnyState(U'\32'): [[fallthrough]];
            case AnyState(U'\u0080'): [[fallthrough]];
            case AnyState(U'\u0081'): [[fallthrough]];
            case AnyState(U'\u0082'): [[fallthrough]];
            case AnyState(U'\u0083'): [[fallthrough]];
            case AnyState(U'\u0086'): [[fallthrough]];
            case AnyState(U'\u0087'): [[fallthrough]];
            case AnyState(U'\u0089'): [[fallthrough]];
            case AnyState(U'\u008A'): [[fallthrough]];
            case AnyState(U'\u008B'): [[fallthrough]];
            case AnyState(U'\u008C'): [[fallthrough]];
            case AnyState(U'\u0091'): [[fallthrough]];
            case AnyState(U'\u0092'): [[fallthrough]];
            case AnyState(U'\u0093'): [[fallthrough]];
            case AnyState(U'\u0094'): [[fallthrough]];
            case AnyState(U'\u0095'): [[fallthrough]];
            case AnyState(U'\u0099'): { Ground: scs=0; state=st_default; break; }
            case State(U'\33', st_string): [[fallthrough]];
            case State(U'\33', st_string_str): state = st_esc; break; // don't clear params
            case State(U'\33', st_default): state = st_esc; p.clear(); break;
            case State(U'(', st_esc): scs = 0; state = st_scs; break; // esc (
            case State(U')', st_esc): scs = 1; state = st_scs; break; // esc )
            case State(U'*', st_esc): scs = 2; state = st_scs; break; // esc *
            case State(U'+', st_esc): scs = 3; state = st_scs; break; // esc +
            case State(U'-', st_esc): scs = 1; state = st_scs; break; // esc -
            case State(U'.', st_esc): scs = 2; state = st_scs; break; // esc .
            case State(U'/', st_esc): scs = 3; state = st_scs; break; // esc /
            case State(U'#', st_esc): state = st_scr; break;  // esc #
            case State(U'[', st_esc): state = st_csi; break;  // esc [
            case State(U'%', st_esc): state = st_esc_percent; break; // esc %
            case State(U'?', st_csi): state = st_csi_dec; break;  // csi ?
            case State(U'>', st_csi): state = st_csi_dec2; break; // csi >
            case State(U'=', st_csi): state = st_csi_dec3; break; // csi =
            case State(U'!', st_csi): state = st_csi_ex; break;  // csi ! (note: after numbers)
            case State(U'"', st_csi): state = st_csi_quo; break; // csi " (note: after numbers)
            case State(U'$', st_csi): state = st_csi_dol; break; // csi $ (note: after numbers)
            case State(U'*', st_csi): state = st_csi_star; break; // csi * (note: after numbers)
            case State(U'$', st_csi_dec): state = st_csi_dec_dol; break; // csi ? $ (note: after numbers)
            case AnyState(U'\7'):
            {
                if(state == st_string || state == st_string_str) // Treat as ST (string termination)
                {
            case AnyState(U'\u009C'): [[fallthrough]];
            case State(U'\\', st_esc): // String termination
                    switch(scs)
                    {
                        case 4: // Parse DCS
                            GetParams(1, false);
                            if(string.empty()) break;
                            switch(string[0])
                            {
                                case U'$':
                                    if(string.size() <= 1 || string[1] != 'q')
                                        EchoBack(U"\u0018");
                                    else
                                    {
                                        string.erase(0,2);
                                        std::string response;
                                        char Buf[40];
                                        if(string == U"r") response.assign(Buf, std::sprintf(Buf, "%zu;%zu", top,bottom));
                                        else if(string == U"m") response.assign(Buf, std::sprintf(Buf, "38;2;%u;%u;%u;48;2;%u;%u;%u",
                                                                    (wnd.blank.fgcolor >> 16)&0xFF,
                                                                    (wnd.blank.fgcolor >> 8)&0xFF,
                                                                    (wnd.blank.fgcolor >> 0)&0xFF,
                                                                    (wnd.blank.bgcolor >> 16)&0xFF,
                                                                    (wnd.blank.bgcolor >> 8)&0xFF,
                                                                    (wnd.blank.bgcolor >> 0)&0xFF));
                                        else if(string == U"t") response = std::to_string(std::max(25u, unsigned(wnd.ysize))-1);
                                        else if(string == U" q") response = "1"; //cursor type
                                        else if(string == U"\"q") response = wnd.blank.protect ? "1" : "0"; //protected mode
                                        else if(string == U"\"p") response = "64;1"; //vt index, 8-bit controls disabled
                                        else if(string == U"$|") response = std::to_string(wnd.xsize); //window width
                                        else if(string == U"*|") response.assign(Buf, std::sprintf(Buf, "%zu", wnd.ysize));
                                        else {fprintf(stderr, "Unrecognized DCS<%s>\n", ToUTF8(string).c_str());
                                            EchoBack(U"\033$P0\033\\");}
                                        if(!response.empty())
                                            EchoBack(U"\033$P1$r" + FromUTF8(response) + string + U"\033\\");
                                    }
                                    break;
                                case U'q': /* Sixel graphics */
                                {
                                    [[maybe_unused]] unsigned pad=1, pan=2, ph=1, pv=1, rep=1, x=0, y=0, color=3;
                                    [[maybe_unused]] bool trans=false;
                                    // Parse pre-q parameters
                                    if(p.size()>=1) pad = std::array<int,10>{2,2,5,4,4,3,3,2,2,1}[std::min(9u,p[0])];
                                    if(p.size()>=2) { trans=p[1]; }
                                    if(p.size()>=2) { pad = std::max(1u,pad*p[2]/10); pan = std::max(1u,pan*p[2]/10); }
                                    if(p.size()>=7) { pad=p[3]; pan=p[4]; ph=p[5]; pv=p[6]; }
                                    for(std::size_t b=0,a=0; a<string.size(); a = ++b)
                                    {
                                        // Parse parameters that follow the command
                                        for(p.clear(); (b+1) < string.size(); ++b)
                                            if(string[b+1] >= U';') p.emplace_back();
                                            else if(string[b+1] >= U'0' && string[b+1] <= U'9')
                                            {
                                                if(p.empty()) p.emplace_back();
                                                p.back() = p.back() * 10u + (string[b+1]-U'0');
                                            }
                                            else break;
                                        switch(string[a])
                                        {
                                            case '\u0034': GetParams(4,false);
                                                           pad = std::max(1u,p[0]);
                                                           pan = std::max(1u,p[1]);
                                                           if(p[2]) ph = p[2];
                                                           if(p[3]) pv = p[3];
                                                           break;
                                            case '\u0035': switch(GetParams(5,false); p[1])
                                                           {
                                                               case 0: color=p[0]; break; // choose color p[0]
                                                               case 1: break; // change color p[0] into hls: p[2..4]
                                                               case 2: break; // change color p[0] into rgb: p[2..4]
                                                           }
                                                           break;
                                            case '\u002D': y+=6; [[fallthrough]]; // TODO: scroll if necessary
                                            case '\u0036': x=0; break;
                                            case '\u0033': GetParams(1,false); rep = std::max(1u, p[0]); break;
                                            default:
                                                if(string[a] >= '\u003F' && string[a] <= '\u007E')
                                                {
                                                    unsigned bitmask = string[a] - '\u003F';
                                                    for(; rep-- > 0; ++x)
                                                        for(unsigned py=0; py<6; ++py)
                                                            if(bitmask & (1u << py))
                                                            {
                                                            }
                                                    rep = 1;
                                                }
                                        }
                                    }
                                    break;
                                }
                                case U'p': /* ReGIS graphics */ break;
                            }
                            break;
                        case 5: // Parse OSC
                        {
                            GetParams(2,false);
                            bool dfl = p[0] >= 100;
                            auto DoColor = [&](const char* params,unsigned idx, unsigned& color, unsigned dflcolor)
                            {
                                char Buf[32];
                                if(string == U"?")
                                {
                                    char st[3] = {'\33', char(c), '\0'};
                                    if(c != '\\') { st[0] = char(c); st[1] = '\0'; }
                                    // ^ Echo back using the same ST code
                                    EchoBack(FromUTF8({Buf, 0u+std::sprintf(Buf, "\33]%s%u;#%06X%s", params,idx, color, st)}));
                                }
                                else if(dfl) color = dflcolor;
                                else color = ParseColorName(string);
                            };
                            switch(unsigned p100 = p[0] % 100)
                            {
                                case 0:
                                case 1:
                                case 2:
                                {
                                    std::string utstring = ToUTF8(string);
                                    if(!(p100 & 2)) // 0 and 1: set icon name
                                        ui.SetIconName(utstring);
                                    if(!(p100 & 1)) // 0 and 2: set window title
                                        ui.SetWindowTitle(utstring);
                                    break;
                                }
                                case 6: break; // set or clear color{BD,UL,BL,RV,IT} mode
                                case 5:  p[1] += 256; [[fallthrough]];
                                case 4: {unsigned v = xterm256table[p[1]&0xFF];
                                         DoColor("4;",p[1]&0xFF, v, v);
                                         break;} // change color [p[1]] to string, or if ?, report the string
                                case 10: DoColor("",p[0], wnd.blank.fgcolor, Cell{}.fgcolor); break; // Change text foreground color
                                case 11: DoColor("",p[0], wnd.blank.bgcolor, Cell{}.bgcolor); break; // Change text background color
                                case 12: DoColor("",p[0], wnd.cursorcolor, 0xFFFFFF); break; // Change text cursor color
                                case 13: DoColor("",p[0], wnd.mousecolor1, 0xFFFFFF); break; // Change mouse foreground color
                                case 14: DoColor("",p[0], wnd.mousecolor2, 0xFFFFFF); break; // Change mouse background color
                                case 17: DoColor("",p[0], wnd.mouseselectcolor, 0xFFFFFF); break; // Change mouse select-text background color
                                case 50: break; // set font
                            }
                            break;
                        }
                        default:
                            break;
                    }
                    goto Ground;
                }
                lastch=c; ui.BeepOn(); break;
            }
            case AnyState(U'\u0090'): p.clear(); [[fallthrough]];
            case State(U'P', st_esc): state = st_string; scs = 4; string.clear(); break; // DCS
            case AnyState(U'\u009D'): p.clear(); [[fallthrough]];
            case State(U']', st_esc): state = st_string; scs = 5; string.clear(); break; // OSC
            case AnyState(U'\u009E'): p.clear(); [[fallthrough]];
            case State(U'^', st_esc): state = st_string; scs = 6; string.clear(); break; // PM
            case AnyState(U'\u009F'): p.clear(); [[fallthrough]];
            case State(U'_', st_esc): state = st_string; scs = 7; string.clear(); break; // APC
            case AnyState(U'\u0098'): p.clear(); [[fallthrough]];
            case State(U'X', st_esc): state = st_string; scs = 8; string.clear(); break; // SOS

            case CsiState(U'0'): case CsiState(U'1'):
            case CsiState(U'2'): case CsiState(U'3'):
            case CsiState(U'4'): case CsiState(U'5'):
            case CsiState(U'6'): case CsiState(U'7'):
            case CsiState(U'8'): case CsiState(U'9'):
                if(p.empty()) p.emplace_back();
                p.back() = p.back() * 10u + (c - U'0');
                break;
            case CsiState(U':'): case CsiState(U';'):
                p.emplace_back();
                break;

            //case AnyState(U'\u0085'): // CASE_NEL
            case State(U'E', st_esc): // esc E = CR+LF
                ClampedMoveX(0);
                state = st_default;
                [[fallthrough]];
            case AnyState(10):
            case AnyState(11):
            case AnyState(12):
            //case AnyState(U'\u0084'):
            case State(U'D', st_esc): // esc D = CASE_IND
                lastch = U'\n';
                /* Within window: move cursor down; scroll the window up if at bottom */
                Lf();
                goto Ground;

            //case AnyState(U'\u008D'):
            case State(U'M', st_esc): // esc M = CASE_RI
                /* Within window: move cursor up; scroll the window down if at top */
                if(wnd.cursy == top)
                    YScrollDown(top, bottom, 1);
                else
                    ClampedMoveY(wnd.cursy-1);
                goto Ground;
            case State(U'q', st_csi_quo): // DECSCA: if param0=1, do CASE_SPA; if 0 or 2, do CASE_EPA. Otherwise ignore.
            {
                GetParams(1,false);
                if(p[0]==1)
                {
            //case AnyState(U'\u0096'):
            case State(U'V', st_esc): // esc V = SPA: start protected area
                    wnd.blank.protect = true;
                }
                else if(p[0]==0 || p[0]==2)
                {
            //case AnyState(U'\u0097'):
            case State(U'W', st_esc): // esc W = EPA: end protected area
                    wnd.blank.protect = false;
                }
                goto Ground;
            }
            /*//case AnyState(U'\u0088'): [[fallthrough]];
            case State(U'H', st_esc): // esc H = CASE_HTS: horizontal tab set
                goto Ground;
            //case AnyState(U'\u0090'): [[fallthrough]];
            case State(U'P', st_esc): // esc P = CASE_DCS (starts string)
                goto Ground;
            //case AnyState(U'\u009D'): [[fallthrough]];
            case State(U']', st_esc): // esc ] = CASE_OSC (starts string)
                goto Ground;
            //case AnyState(U'\u009E'): [[fallthrough]];
            case State(U'^', st_esc): // esc ^ = CASE_PM (starts string)
                goto Ground;
            //case AnyState(U'\u009F'): [[fallthrough]];
            case State(U'_', st_esc): // esc _ = CASE_APC (starts string)
                goto Ground;
            //case AnyState(U'\u0098'): [[fallthrough]];
            case State(U'X', st_esc): // esc X = CASE_SOS: start of string
                goto Ground;
            //case AnyState(U'\u009C'): [[fallthrough]];
            case State(U'\\', st_esc):// esc \\ = CASE_ST: end of string
                goto Ground;*/

            case State(U'c', st_esc): ResetAttr(); Reset(); goto Ground;         // esc c   (RI - full reset)
            case State(U'p', st_csi_ex): ResetAttr(); Reset(false); goto Ground; // CSI ! p (DECSTR - CSI reset)

            case State(U'7', st_esc): [[fallthrough]]; // esc 7 (DECSC), csi s (ANSI_SC)
            case State(U's', st_csi): SaveCur(); goto Ground;
            case State(U'8', st_esc): [[fallthrough]]; // esc 8 (DECRC), csi u (ANSI_RC)
            case State(U'u', st_csi): RestoreCur(); goto Ground;
            case State(U'0', st_scs): gset[scs&3] = 1; goto Ground; // DEC graphics (TODO)
            case State(U'1', st_scs): gset[scs&3] = 0; goto Ground; // DEC alt chars?
            case State(U'2', st_scs): gset[scs&3] = 0; goto Ground; // DEC alt gfx?
            case State(U'`', st_scs): gset[scs&3] = 0; goto Ground; // nor/dan?
            case State(U'4', st_scs): gset[scs&3] = 0; goto Ground; // dut?
            case State(U'5', st_scs): gset[scs&3] = 0; goto Ground; // fin?
            case State(U'6', st_scs): gset[scs&3] = 0; goto Ground; // nor/dan3?
            case State(U'7', st_scs): gset[scs&3] = 0; goto Ground; // swe?
            case State(U'9', st_scs): gset[scs&3] = 0; goto Ground; // fre/can2?
            case State(U'A', st_scs): gset[scs&3] = 0; goto Ground; // british?
            case State(U'B', st_scs): gset[scs&3] = 0; goto Ground; // ASCII (TODO)
            case State(U'C', st_scs): gset[scs&3] = 0; goto Ground; // fin2?
            case State(U'E', st_scs): gset[scs&3] = 0; goto Ground; // nor/dan2?
            case State(U'f', st_scs): gset[scs&3] = 0; goto Ground; // fre2?
            case State(U'F', st_scs): gset[scs&3] = 0; goto Ground; // iso greek supp?
            case State(U'H', st_scs): gset[scs&3] = 0; goto Ground; // swe2? iso hebrew supp?
            case State(U'K', st_scs): gset[scs&3] = 0; goto Ground; // ger?
            case State(U'L', st_scs): gset[scs&3] = 0; goto Ground; // iso cyr?
            case State(U'M', st_scs): gset[scs&3] = 0; goto Ground; // iso5 supp?
            case State(U'Q', st_scs): gset[scs&3] = 0; goto Ground; // fre/can?
            case State(U'R', st_scs): gset[scs&3] = 0; goto Ground; // fre?
            case State(U'<', st_scs): gset[scs&3] = 0; goto Ground; // DEC supp?
            case State(U'=', st_scs): gset[scs&3] = 0; goto Ground; // swiss?
            case State(U'>', st_scs): gset[scs&3] = 0; goto Ground; // DEC technical
            case State(U'U', st_scs): gset[scs&3] = 0; goto Ground; // linux pc?
            case State(U'Y', st_scs): gset[scs&3] = 0; goto Ground; // ita?
            case State(U'Z', st_scs): gset[scs&3] = 0; goto Ground; // spa?
            case State(U'3', st_scr): wnd.LineSetRenderSize(2); goto Ground; // DECDHL top
            case State(U'4', st_scr): wnd.LineSetRenderSize(3); goto Ground; // DECDHL bottom
            case State(U'5', st_scr): wnd.LineSetRenderSize(0); goto Ground; // DECSWL
            case State(U'6', st_scr): wnd.LineSetRenderSize(1); goto Ground; // DECDWL
            case State(U'8', st_scr): // clear screen with 'E' // esc # 8
                wnd.blank.ch = U'E';
                wnd.FillBox(0,0, wnd.xsize,wnd.ysize);
                wnd.blank.ch = U' ';
                wnd.cursx = wnd.cursy = 0;
                goto Ground;
            case State(U'@', st_esc_percent): utfmode = 0; goto Ground; // esc % @
            case State(U'G', st_esc_percent): [[fallthrough]];          // esc % G
            case State(U'8', st_esc_percent): utfmode = 1; goto Ground; // esc % 8
            case State(U'g', st_csi): /* TODO: set tab stops */ goto Ground;
            case State(U'q', st_csi): /* TODO: set leds */ goto Ground;
            case State(U'G', st_csi): [[fallthrough]];
            case State(U'`', st_csi): { GetParams(1,true); ClampedMoveX(p[0]-1);        break; } // absolute hpos
            case State(U'd', st_csi): { GetParams(1,true); ClampedMoveY(p[0]-1, false); break; } // absolute vpos
            case State(U'F', st_csi): ClampedMoveX(0); [[fallthrough]];
            case State(U'A', st_csi): { GetParams(1,true); ClampedMoveY(wnd.cursy-p[0]); break; }
            case State(U'E', st_csi): ClampedMoveX(0); [[fallthrough]];
            case State(U'e', st_csi): [[fallthrough]];
            case State(U'B', st_csi): { GetParams(1,true); ClampedMoveY(wnd.cursy+p[0]); break; }
            case State(U'a', st_csi): [[fallthrough]];
            case State(U'C', st_csi): { GetParams(1,true); ClampedMoveX(wnd.cursx+p[0]); break; }
            case State(U'D', st_csi): { GetParams(1,true); ClampedMoveX(wnd.cursx-p[0]); break; }
            case State(U'H', st_csi): [[fallthrough]];
            case State(U'f', st_csi): { GetParams(2,true); ClampedMove(p[1]-1, p[0]-1, false); break; }
            case State(U'J', st_csi):
            case State(U'J', st_csi_quo):
                GetParams(1,false);
                switch(p[0])
                {
                    case 0: // erase from cursor to end of display
                        if(wnd.cursy < wnd.ysize-1)
                            wnd.FillBox(0,wnd.cursy+1, wnd.xsize, wnd.ysize-wnd.cursy-1, wnd.blank);
                        goto clreol;
                    case 1: // erase from start to cursor
                        if(wnd.cursy > 0) wnd.FillBox(0,0, wnd.xsize,wnd.cursy, wnd.blank);
                        goto clrbol;
                    case 2: // erase whole display
                        wnd.FillBox(0,0, wnd.xsize,wnd.ysize, wnd.blank);
                        break;
                }
                break;
            case State(U'K', st_csi):
            case State(U'K', st_csi_quo):
                GetParams(1,false);
                // 0: erase from cursor to end of line
                // 1: erase from start of line to cursor
                // 2: erase whole line
                switch(p[0])
                {
                    case 0: clreol: wnd.FillBox(wnd.cursx,wnd.cursy, wnd.xsize-wnd.cursx, 1, wnd.blank); break;
                    case 1: clrbol: wnd.FillBox(0,        wnd.cursy, wnd.cursx+1,  1, wnd.blank); break;
                    case 2: wnd.FillBox(0, wnd.cursy, wnd.xsize,    1, wnd.blank); break;
                }
                break;
            case State(U'M', st_csi):
                GetParams(1,true);
                YScrollUp(wnd.cursy, bottom, p[0]);
                break;
            case State(U'L', st_csi):
                GetParams(1,true);
                // scroll the rest of window c lines down,
                // including where cursor is. Don't move cursor.
                YScrollDown(wnd.cursy, bottom, p[0]);
                break;
            case State(U'S', st_csi): // xterm version?
                GetParams(1,true);
                YScrollUp(top, bottom, p[0]);
                break;
            case State(U'T', st_csi): // csi T, track mouse
                if(p.size() > 1 || p.empty() || p[0]==0)
                {
                    // mouse track
                    goto Ground;
                }
                [[fallthrough]];
            case State(U'^', st_csi): // csi ^ , scroll down
                GetParams(1,true);
                // Reverse scrolling by N lines
                // scroll the entire of window c lines down. Don't move cursor.
                YScrollDown(top, bottom, p[0]);
                break;
            case State(U'P', st_csi):
                GetParams(1,true); c = std::min(p[0], unsigned(wnd.xsize-wnd.cursx));
                // insert c black holes at cursor (eat c characters
                // and scroll line horizontally to left)
                if(c)
                {
                    unsigned remain = wnd.xsize - (wnd.cursx+c);
                    wnd.CopyText(wnd.cursx,wnd.cursy, wnd.xsize-remain,wnd.cursy, remain,1);
                    wnd.FillBox(wnd.xsize-c,wnd.cursy, c,1);
                }
                break;
            case State(U'X', st_csi): // (ECH)
                GetParams(1,true);
                // write c spaces at cursor (overwrite)
                wnd.FillBox(wnd.cursx,wnd.cursy, std::min(std::size_t(p[0]), wnd.xsize-wnd.cursx), 1);
                break;
            case State(U'@', st_csi):
                GetParams(1,true); c = std::min(p[0], unsigned(wnd.xsize-wnd.cursx));
                // insert c spaces at cursor
                if(c)
                {
                    unsigned remain = wnd.xsize - (wnd.cursx+c);
                    wnd.CopyText(wnd.cursx+c,wnd.cursy, wnd.cursx,wnd.cursy, remain,1);
                    wnd.FillBox(wnd.cursx,wnd.cursy, c,1);
                }
                break;
            case State(U'r', st_csi): // CSI r
                GetParams(2,false);
                if(!p[0]) p[0]=1;
                if(!p[1]) p[1]=wnd.ysize;
                if(p[0] < p[1] && p[1] <= wnd.ysize)
                {
                    top = p[0]-1; bottom = p[1]-1;
                    //fprintf(stderr, "Creating a window with top=%zu, bottom=%zu\n", top,bottom);
                    ClampedMove(0, top, false);
                }
                break;
            case State(U'n', st_csi):
                GetParams(1,false);
                switch(p[0])
                {
                    char Buf[32];
                    case 5: EchoBack(U"\33[0n"); break;
                    case 6: EchoBack(FromUTF8(std::string_view{Buf, (std::size_t)std::sprintf(Buf, "\33[%zu;%zuR", wnd.cursy+1, wnd.cursx+1)})); break;
                }
                break;
            case State(U'c', st_csi_dec3): // csi = 0 c, Tertiary device attributes (printer?)
                GetParams(1,false); // Tertiary device attributes (printer?)
                // Example response: <ESC> P ! | 0 <ST>
                if(!p[0]) EchoBack(U"\33P!|00000000\x9C"); // Note: DCS response
                break;
            case State(U'c', st_csi_dec2): // csi > 0 c, Secondary device attributes (terminal)
                GetParams(1,false);
                // Example response: ^[[>41;344;0c  (middle=firmware version)
                if(!p[0]) EchoBack(U"\33[>1;1;0c");
                break;
            case State(U'c', st_csi): // csi 0 c // Primary device attributes (host computer)
                GetParams(1,false);
                if(!p[0])
                {
            //case AnyState(U'\u009A'): [[fallthrough]];
            case State(U'Z', st_esc): // esc Z = CASE_DECID
                    EchoBack(U"\33[?65;1;6;8;15;22c");
                }
                // Example response: ^[[?64;1;2;6;9;15;18;21;22c
                //  1=132 columns, 2=printer port, 4=sixel extension
                //  6=selective erase, 7=DRCS, 8=user-defines keys
                //  9=national replacement charsets, 12=SCS extension
                // 15=technical charset, 18=windowing capability, 21=horiz scrolling,
                // 22=ansi color/vt525, 29=ansi text locator
                // 23=greek ext, 24=turkish ext, 42=latin2 cset, 44=pcterm,
                // 45=softkeymap, 46=ascii emulation
                // 62..69 = VT level (62=VT200, 63=VT300, 64=VT400)
                break;
            case State(U'h', st_csi_dec): // csi ? h, misc modes on
            case State(U'l', st_csi_dec): // csi ? l, misc modes off
            case State(U'p', st_csi_dec_dol): // csi ? $p, query
            {
                bool set = c == U'h', query = c == U'p';
                char Buf[32];
                // 1=CKM, 2=ANM, 3=COLM, 4=SCLM, 5=SCNM, 6=OM, 7=AWM, 8=ARM,
                // 18=PFF, 19=PEX, 25=TCEM, 40=132COLS, 42=NRCM,
                // 44=MARGINBELL, ...
                //   6 puts cursor at (0,0) both set,clear
                //  25 enables/disables cursor visibility
                //  40 enables/disables 80/132 mode (note: if enabled, RESET changes to one of these)
                //   3 sets width at 132(enable), 80(disable) if "40" is enabled
                //   5 = screenwide inverse color
                GetParams(0, false);
                for(auto a: p)
                {
                    unsigned value = 0; // unsupported
                    switch(a)
                    {
                        case 6:  if(query) value = 2; else ClampedMove(0, top, false); break;
                        case 25: if(query) value = wnd.cursorvis?1:2; else wnd.cursorvis = set; break;
                        case 5:  if(query) value = wnd.inverse?1:2;   else wnd.inverse   = set; break;
                    }
                    if(query)
                        EchoBack(FromUTF8(std::string_view{Buf, (std::size_t)
                            std::sprintf(Buf, "\33[?%u;%u$y", a, value)}));
                }
                break;
            }
            case State(U'h', st_csi): // csi h, ansi modes on
            case State(U'l', st_csi): // csi l, ansi modes off
            case State(U'p', st_csi_dol): // csi $p, query
            {
                bool /*set = c == U'h',*/ query = c == U'p';
                char Buf[32];
                // 2 = keyboard locked
                // 4 = insert mode
                // 12 = no local echo
                // 20 = auto linefeed
                GetParams(0, false);
                for(auto a: p)
                {
                    unsigned value = 0; // unsupported
                    switch(a)
                    {
                        case 2: value = 4; break; // permanently unset
                        case 4: value = 4; break; // permanently unset
                        case 12: value = 4; break; // permanently unset
                        case 20: value = 4; break; // permanently unset
                    }
                    if(query)
                        EchoBack(FromUTF8(std::string_view{Buf, (std::size_t)
                            std::sprintf(Buf, "\33[%u;%u$y", a, value)}));
                }
                break;
            }
            case State(U'v', st_csi_dol): // csi $v (DECCRA): copy rectangular area
            {
                GetParams(7,true);
                unsigned pts=p[0], pls=p[1], pbs=p[2], prs=p[3], ptd=p[5], pld=p[6];
                // Ignores [4] = source page number, [7] = target page number.
                // Note: Xterm parses params from right to left, meaning that
                //       for xterm, our [3] is actually [n-5]
                if(pbs > wnd.ysize) pbs = wnd.ysize;
                if(prs > wnd.xsize) prs = wnd.xsize;
                if(ptd > wnd.ysize) ptd = wnd.ysize;
                if(pld > wnd.xsize) pld = wnd.ysize;
                if(pts <= pbs && pls <= prs)
                {
                    unsigned width = prs-pls+1, height = pbs-pts+1;
                    --pld;--ptd;--pls;--pts;
                    if(pld+width > wnd.xsize) width = wnd.xsize-pld;
                    if(ptd+height > wnd.ysize) height = wnd.ysize-ptd;
                    if(width && height)
                        wnd.CopyText(pld,ptd, pls,pts, width,height);
                }
                break;
            }
            case State(U'z', st_csi_dol): // csi $z: fill rectangular area with space
            {                             //         but don't touch attributes.
                GetParams(4,true);
                unsigned pts=p[0], pls=p[1], pbs=p[2], prs=p[3];
                if(pbs > wnd.ysize) pbs = wnd.ysize;
                if(prs > wnd.xsize) prs = wnd.xsize;
                for(unsigned y=pts; y<=pbs; ++y)
                    for(unsigned x=pls; x<=prs; ++x)
                        wnd.PutCh_KeepAttr(x-1, y-1, U' ', false);
                break;
            }
            case State(U'x', st_csi_dol): // csi $x: fill rectangular area with given char
            {
                GetParams(5,true);
                unsigned ch=p[0], pts=p[1], pls=p[2], pbs=p[3], prs=p[4];
                if(pbs > wnd.ysize) pbs = wnd.ysize;
                if(prs > wnd.xsize) prs = wnd.xsize;
                bool dbl = isdouble(ch);
                for(unsigned y=pts; y<=pbs; ++y)
                    for(unsigned x=pls; x<=prs; ++x)
                        wnd.PutCh(x-1, y-1, ch, dbl);
                break;
            }
            case State(U'r', st_csi_dol): // csi $r: DECCARA: change attributes in rectangular area
            case State(U't', st_csi_dol): // csi $t: DECRARA: toggle attributes in rectangular area
            {
                bool toggle = (c == U't');
                GetParams(5, true); // Make sure there is at least 1 SGR param
                unsigned pts=p[0], pls=p[1], pbs=p[2], prs=p[3];
                if(pbs > wnd.ysize) pbs = wnd.ysize;
                if(prs > wnd.xsize) prs = wnd.xsize;
                --pts;--pbs;--pls;--prs;
                for(unsigned y=pts; y<=pbs; ++y)
                    for(unsigned x=pls; x<=prs; ++x)
                    {
                        auto& cell = wnd.cells[y*wnd.xsize+x];
                        auto temp = cell;
                        c=0;
                        for(auto ai = std::next(p.cbegin(), 4); ai != p.end(); ++ai)
                            ProcessSGR(c, *ai,
                                [&]() { if(!toggle) temp = Cell{}; },
                                [&](auto&& setter, auto&& getter, auto newvalue)
                                {
                                    if(toggle)
                                    {
                                        // Note: toggling using a SGR command
                                        //       that clears the attribute does nothing,
                                        //       because value XOR 0 is value.
                                        //       This matches what XTerm does.
                                        newvalue ^= getter(temp);
                                    }
                                    setter(temp, newvalue);
                                }
                                      );
                        temp.ch = cell.ch;
                        if(temp != cell)
                        {
                            cell       = temp;
                            cell.dirty = true;
                        }
                    }
                break;
            }
            case State(U'b', st_csi):
            {
                GetParams(1,true);
                // Repeat last printed character n times
                bool dbl = isdouble(lastch);
                for(unsigned m = std::min(p[0], unsigned(wnd.xsize*wnd.ysize)), c=0; c<m; ++c)
                {
                    PutC(lastch, dbl);
                }
                break;
            }
            case State(U'm', st_csi): // csi m (SGR)
            {
                GetParams(1, false); // Make sure there is at least 1 param
                c=0;
                for(auto a: p)
                    ProcessSGR(c,a,
                        [&]() { ResetAttr(); },
                        [&](auto&& setter, auto&& /*getter*/, auto newvalue)
                            { setter(wnd.blank, newvalue); }
                              );
                break;
            }

            default:
                if(state == st_string) state = st_string_str;
                if(state == st_string_str)
                {
                    string += c;
                    break;
                }
                if(state != st_default) goto Ground;
                PutC(lastch = c, isdouble(c));
                break;
        }
    }
    #undef AnyState
}

void TerminalWindow::EchoBack(std::u32string_view buffer)
{
    //Write(buffer, size); // DEBUGGING
    OutBuffer.insert(OutBuffer.end(), buffer.begin(), buffer.end());
}

void TerminalWindow::SaveCur()
{
    backup.cx = wnd.cursx;
    backup.cy = wnd.cursy;
    backup.attr = wnd.blank;
}

void TerminalWindow::RestoreCur()
{
    wnd.cursx = backup.cx;
    wnd.cursy = backup.cy;
    wnd.blank = backup.attr;
}

void TerminalWindow::Resize(std::size_t newsx, std::size_t newsy)
{
    if(bottom == wnd.ysize-1)
    {
        bottom = newsy-1;
        //fprintf(stderr, "Creating a window with top=%zu, bottom=%zu\n", top,bottom);
    }
    wnd.Resize(newsx, newsy);
}

#ifdef RUN_TESTS
template<typename F>
static auto TerminalTest(int w, int h, std::u32string_view sample, F&& test)
{
    Window wnd(w, h);
    TerminalWindow term(wnd);
    term.Write(sample);
    return test(wnd);
}
template<typename F>
static auto TerminalTest(std::u32string_view sample, F&& test)
{
    return TerminalTest(80, 25, sample, test);
}
static auto TerminalTestSGR(std::u32string_view sample)
{
    return TerminalTest(sample, [&](auto& wnd) { return wnd.blank; });
}
static auto TerminalTestCursor(int w, int h, std::u32string_view sample)
{
    return TerminalTest(w,h, sample, [&](auto& wnd)
    {
        // Return the following:
        // Cursor location (x, y)
        // Location of first symbol non-space within window
        // Location of last symbol non-space within window
        // Number of non-space symbols within window
        std::array<unsigned,5> result{ (unsigned)wnd.cursx, (unsigned)wnd.cursy, ~0u, 0, 0 };
        for(unsigned p=0; p<wnd.cells.size(); ++p)
            if(wnd.cells[p].ch != U' ')
            {
                if(result[2] == ~0u) result[2] = p;
                result[3] = p;
                ++result[4];
            }
        if(result[2] == ~0u) result[2] = 0u;
        return result;
    });
}


TEST(terminal, text_rendering_works)
{
    EXPECT_TRUE(TerminalTest(U"", [&](auto& wnd)
    {
        return wnd.cells[0].ch == U' ';
    }));
    EXPECT_TRUE(TerminalTest(U"OK", [&](auto& wnd)
    {
        return wnd.cells[0].ch == U'O' && wnd.cells[1].ch == U'K' && wnd.cells[2].ch == U' ';
    }));
    EXPECT_TRUE(TerminalTest(U"O\rK", [&](auto& wnd)
    {
        return wnd.cells[0].ch == U'K' && wnd.cells[1].ch == U' ';
    }));
    EXPECT_TRUE(TerminalTest(U"OK\b!", [&](auto& wnd)
    {
        return wnd.cells[0].ch == U'O' && wnd.cells[1].ch == U'!';
    }));
    EXPECT_TRUE(TerminalTest(U"O\tK", [&](auto& wnd)
    {
        return wnd.cells[0].ch == U'O' && wnd.cells[8].ch == U'K';
    }));
    EXPECT_TRUE(TerminalTest(U" O\tK", [&](auto& wnd)
    {
        return wnd.cells[1].ch == U'O' && wnd.cells[8].ch == U'K';
    }));
    EXPECT_TRUE(TerminalTest(U"O\nK", [&](auto& wnd)
    {
        return wnd.cells[0].ch == U'O' && wnd.cells[81].ch == U'K';
    }));
}
TEST(terminal, sgr_attributes)
{
    EXPECT_EQ(TerminalTestSGR(U"\33[0m"), []{Cell tmp; return tmp; }());
    EXPECT_EQ(TerminalTestSGR(U"\33[1m"), []{Cell tmp; tmp.bold=true; return tmp; }());
    EXPECT_EQ(TerminalTestSGR(U"\33[2m"), []{Cell tmp; tmp.dim=true; return tmp; }());
    EXPECT_EQ(TerminalTestSGR(U"\33[3m"), []{Cell tmp; tmp.italic=true; return tmp; }());
    EXPECT_EQ(TerminalTestSGR(U"\33[4m"), []{Cell tmp; tmp.underline=true; return tmp; }());
    EXPECT_EQ(TerminalTestSGR(U"\33[5m"), []{Cell tmp; tmp.blink=1; return tmp; }());
    EXPECT_EQ(TerminalTestSGR(U"\33[6m"), []{Cell tmp; tmp.blink=2; return tmp; }());
    EXPECT_EQ(TerminalTestSGR(U"\33[7m"), []{Cell tmp; tmp.inverse=true; return tmp; }());
    EXPECT_EQ(TerminalTestSGR(U"\33[9m"), []{Cell tmp; tmp.overstrike=true; return tmp; }());
    EXPECT_EQ(TerminalTestSGR(U"\33[21m"), []{Cell tmp; tmp.underline2=true; return tmp; }());
    EXPECT_EQ(TerminalTestSGR(U"\33[53m"), []{Cell tmp; tmp.overlined=true; return tmp; }());

    EXPECT_EQ(TerminalTestSGR(U"\33[1;2;3;4;7;21;22m"), []{Cell tmp; tmp.bold=tmp.dim=tmp.italic=tmp.underline=tmp.underline2=tmp.inverse=1; tmp.dim=tmp.bold=0; return tmp; }());
    EXPECT_EQ(TerminalTestSGR(U"\33[1;2;3;4;7;21;23m"), []{Cell tmp; tmp.bold=tmp.dim=tmp.italic=tmp.underline=tmp.underline2=tmp.inverse=1; tmp.italic=tmp.fraktur=0; return tmp; }());
    EXPECT_EQ(TerminalTestSGR(U"\33[1;2;3;4;7;21;24m"), []{Cell tmp; tmp.bold=tmp.dim=tmp.italic=tmp.underline=tmp.underline2=tmp.inverse=1; tmp.underline=tmp.underline2=0; return tmp; }());
    EXPECT_EQ(TerminalTestSGR(U"\33[1;2;3;4;7;21;25m"), []{Cell tmp; tmp.bold=tmp.dim=tmp.italic=tmp.underline=tmp.underline2=tmp.inverse=1; tmp.blink=0; return tmp; }());
    EXPECT_EQ(TerminalTestSGR(U"\33[1;2;3;4;7;21;27m"), []{Cell tmp; tmp.bold=tmp.dim=tmp.italic=tmp.underline=tmp.underline2=tmp.inverse=1; tmp.inverse=0; return tmp; }());
    EXPECT_EQ(TerminalTestSGR(U"\33[1;2;9;4;7;21;29m"), []{Cell tmp; tmp.bold=tmp.dim=tmp.overstrike=tmp.underline=tmp.underline2=tmp.inverse=1; tmp.overstrike=0; return tmp; }());
    EXPECT_EQ(TerminalTestSGR(U"\33[1;5;53;2;55m"),     []{Cell tmp; tmp.blink=tmp.bold=tmp.overlined=tmp.dim=1; tmp.overlined=0; return tmp; }());
}
TEST(terminal, sgr_colors)
{
    char Buf[32];
    for(unsigned n=0; n<8; ++n)
    {
        std::sprintf(Buf, "\33[%dm", 30+n);
        EXPECT_EQ(TerminalTestSGR(FromUTF8(Buf)), [n]{Cell tmp; tmp.fgcolor=xterm256table[n]; return tmp; }());
        std::sprintf(Buf, "\33[%dm", 40+n);
        EXPECT_EQ(TerminalTestSGR(FromUTF8(Buf)), [n]{Cell tmp; tmp.bgcolor=xterm256table[n]; return tmp; }());
        std::sprintf(Buf, "\33[%dm", 90+n);
        EXPECT_EQ(TerminalTestSGR(FromUTF8(Buf)), [n]{Cell tmp; tmp.fgcolor=xterm256table[n+8]; return tmp; }());
        std::sprintf(Buf, "\33[%dm", 100+n);
        EXPECT_EQ(TerminalTestSGR(FromUTF8(Buf)), [n]{Cell tmp; tmp.bgcolor=xterm256table[n+8]; return tmp; }());
    }
    for(unsigned n=0; n<256; ++n)
    {
        std::sprintf(Buf, "\33[38;5;%d;48;5;%dm", n, 255-n);
        EXPECT_EQ(TerminalTestSGR(FromUTF8(Buf)), [n]{Cell tmp; tmp.fgcolor=xterm256table[n];
                                                                tmp.bgcolor=xterm256table[255-n]; return tmp; }());
    }
    // RGB24
    EXPECT_EQ(TerminalTestSGR(U"\33[38;2;0;0;0;48;2;255;255;255m"),
                                                  []{Cell tmp; tmp.fgcolor=0x000000;
                                                               tmp.bgcolor=0xFFFFFF; return tmp; }());
    // CMY
    EXPECT_EQ(TerminalTestSGR(U"\33[38;3;0;0;0;48;3;255;255;255m"),
                                                  []{Cell tmp; tmp.fgcolor=cmy2rgb(0x000000);
                                                               tmp.bgcolor=cmy2rgb(0xFFFFFF); return tmp; }());
    // CMYK
    EXPECT_EQ(TerminalTestSGR(U"\33[38;4;0;0;0;0;48;4;255;255;255;255m"),
                                                  []{Cell tmp; tmp.fgcolor=cmyk2rgb(0x00000000);
                                                               tmp.bgcolor=cmyk2rgb(0xFFFFFFFF); return tmp; }());
    // Make sure it gets rendered by this color
    EXPECT_TRUE(TerminalTest(U"\33[32mA", [&](Window& wnd)
    {
        return wnd.cells[0].fgcolor == xterm256table[2];
    }));
}
TEST(terminal, save_restore)
{
    EXPECT_TRUE(TerminalTest(U"ABC" U"\33[s" U"\33[32mDE\33[34mF" U"\33[u" U"G\33[35mH", [&](Window& wnd)
    {
        return wnd.cells[0].ch == 'A' && wnd.cells[1].ch == 'B' && wnd.cells[2].ch == 'C'
        &&     wnd.cells[3].ch == 'G' && wnd.cells[4].ch == 'H' && wnd.cells[5].ch == 'F'
        &&     wnd.cells[3].fgcolor == Cell{}.fgcolor
        &&     wnd.cells[4].fgcolor == xterm256table[5]
        &&     wnd.cells[5].fgcolor == xterm256table[4];
    }));
}
TEST(terminal, cursor_movements)
{
    // CR, LF....
    EXPECT_EQ(TerminalTestCursor(8,4, U"ABC"),      (std::array{3u,0u, 0u,2u,3u}));
    EXPECT_EQ(TerminalTestCursor(8,4, U"AB\rC"),    (std::array{1u,0u, 0u,1u,2u})); // CR
    EXPECT_EQ(TerminalTestCursor(8,4, U"AB\nC"),    (std::array{3u,1u, 0u,10u,3u})); // LF
    EXPECT_EQ(TerminalTestCursor(8,4, U"AB\013C"),  (std::array{3u,1u, 0u,10u,3u})); // LF, alias
    EXPECT_EQ(TerminalTestCursor(8,4, U"AB\014C"),  (std::array{3u,1u, 0u,10u,3u})); // LF, alias
    EXPECT_EQ(TerminalTestCursor(8,4, U"AB\033DC"), (std::array{3u,1u, 0u,10u,3u})); // LF, alias
    EXPECT_EQ(TerminalTestCursor(8,4, U"AB\033EC"), (std::array{1u,1u, 0u,8u,3u})); // CR+LF
    EXPECT_EQ(TerminalTestCursor(8,4, U"AB\033EC"), (std::array{1u,1u, 0u,8u,3u})); // CR+LF
    EXPECT_EQ(TerminalTestCursor(8,4, U"AAAAAAAA"), (std::array{7u,0u, 0u,7u,8u})); // test word wrap
    EXPECT_EQ(TerminalTestCursor(8,4, U"AAAAAAAAA"),(std::array{1u,1u, 0u,8u,9u})); // test word wrap+1
    EXPECT_EQ(TerminalTestCursor(8,4, U"A\r\nB\r\nC\r\nAAAAAAAA"),(std::array{7u,3u, 0u,037u,11u})); // right-bottom column
    EXPECT_EQ(TerminalTestCursor(8,4, U"A\r\nB\r\nC\r\nD\r\n"),   (std::array{0u,3u, 0u,020u,3u}));  // scroll down
    // Absolute
    EXPECT_EQ(TerminalTestCursor(8,4, U"\33[2;2H"),         (std::array{1u,1u, 0u,0u,0u}));
    EXPECT_EQ(TerminalTestCursor(8,4, U"\33[1;1H"),         (std::array{0u,0u, 0u,0u,0u}));
    EXPECT_EQ(TerminalTestCursor(8,4, U"\33[2;2H\33[1;1H"), (std::array{0u,0u, 0u,0u,0u}));
    EXPECT_EQ(TerminalTestCursor(8,4, U"\33[8;1H"),         (std::array{0u,3u, 0u,0u,0u})); // clamped
    EXPECT_EQ(TerminalTestCursor(8,4, U"\33[1;8H"),         (std::array{7u,0u, 0u,0u,0u})); // clamped
    EXPECT_EQ(TerminalTestCursor(8,4, U"\33[2;4H"),         (std::array{3u,1u, 0u,0u,0u}));
    EXPECT_EQ(TerminalTestCursor(8,4, U"\33[2;4;5H"),       (std::array{3u,1u, 0u,0u,0u})); // extra param
    EXPECT_EQ(TerminalTestCursor(8,4, U"\33[2;2H\33[H"),    (std::array{0u,0u, 0u,0u,0u})); // no params
    EXPECT_EQ(TerminalTestCursor(8,4, U"\33[2;2H\33[0;0H"), (std::array{0u,0u, 0u,0u,0u})); // zero params
    EXPECT_EQ(TerminalTestCursor(8,4, U"\33[4;8H"),         (std::array{7u,3u, 0u,0u,0u})); // bottom-right corner
    // Same set with 'f'
    EXPECT_EQ(TerminalTestCursor(8,4, U"\33[2;2f"),         (std::array{1u,1u, 0u,0u,0u}));
    EXPECT_EQ(TerminalTestCursor(8,4, U"\33[1;1f"),         (std::array{0u,0u, 0u,0u,0u}));
    EXPECT_EQ(TerminalTestCursor(8,4, U"\33[2;2H\33[1;1f"), (std::array{0u,0u, 0u,0u,0u}));
    EXPECT_EQ(TerminalTestCursor(8,4, U"\33[8;1f"),         (std::array{0u,3u, 0u,0u,0u})); // clamped
    EXPECT_EQ(TerminalTestCursor(8,4, U"\33[1;8f"),         (std::array{7u,0u, 0u,0u,0u})); // clamped
    EXPECT_EQ(TerminalTestCursor(8,4, U"\33[2;4f"),         (std::array{3u,1u, 0u,0u,0u}));
    EXPECT_EQ(TerminalTestCursor(8,4, U"\33[2;4;5f"),       (std::array{3u,1u, 0u,0u,0u})); // extra param
    EXPECT_EQ(TerminalTestCursor(8,4, U"\33[2;2H\33[f"),    (std::array{0u,0u, 0u,0u,0u})); // no params
    EXPECT_EQ(TerminalTestCursor(8,4, U"\33[2;2H\33[0;0f"), (std::array{0u,0u, 0u,0u,0u})); // zero params
    EXPECT_EQ(TerminalTestCursor(8,4, U"\33[4;8f"),         (std::array{7u,3u, 0u,0u,0u})); // bottom-right corner
    // Right
    EXPECT_EQ(TerminalTestCursor(8,4, U"\33[C"),     (std::array{1u,0u, 0u,0u,0u}));
    EXPECT_EQ(TerminalTestCursor(8,4, U"\33[1C"),    (std::array{1u,0u, 0u,0u,0u}));
    EXPECT_EQ(TerminalTestCursor(8,4, U"\33[4C"),    (std::array{4u,0u, 0u,0u,0u}));
    EXPECT_EQ(TerminalTestCursor(8,4, U"\33[8C"),    (std::array{7u,0u, 0u,0u,0u}));
    EXPECT_EQ(TerminalTestCursor(8,4, U"\33[2;3H\33[2a"), (std::array{4u,1u, 0u,0u,0u})); // a = alias to C
    // Down
    EXPECT_EQ(TerminalTestCursor(8,4, U"\33[B"),     (std::array{0u,1u, 0u,0u,0u}));
    EXPECT_EQ(TerminalTestCursor(8,4, U"\33[1B"),    (std::array{0u,1u, 0u,0u,0u}));
    EXPECT_EQ(TerminalTestCursor(8,4, U"\33[2B"),    (std::array{0u,2u, 0u,0u,0u}));
    EXPECT_EQ(TerminalTestCursor(8,4, U"\33[8B"),    (std::array{0u,3u, 0u,0u,0u}));
    EXPECT_EQ(TerminalTestCursor(8,4, U"\33[2;3H\33[2e"), (std::array{2u,3u, 0u,0u,0u})); // e = alias to B
    EXPECT_EQ(TerminalTestCursor(8,4, U"\33[2;3H\33[2E"), (std::array{0u,3u, 0u,0u,0u})); // E = alias to e, resets X to 0
    // Up
    EXPECT_EQ(TerminalTestCursor(8,4, U"\33[4;8H\33[A"),     (std::array{7u,2u, 0u,0u,0u}));
    EXPECT_EQ(TerminalTestCursor(8,4, U"\33[4;8H\33[1A"),    (std::array{7u,2u, 0u,0u,0u}));
    EXPECT_EQ(TerminalTestCursor(8,4, U"\33[4;8H\33[2A"),    (std::array{7u,1u, 0u,0u,0u}));
    EXPECT_EQ(TerminalTestCursor(8,4, U"\33[4;8H\33[8A"),    (std::array{7u,0u, 0u,0u,0u}));
    EXPECT_EQ(TerminalTestCursor(8,4, U"\33[4;8H\33[1F"),    (std::array{0u,2u, 0u,0u,0u})); // F = same as A, puts X to 0
    // Left
    EXPECT_EQ(TerminalTestCursor(8,4, U"\33[4;8H\33[D"),     (std::array{6u,3u, 0u,0u,0u}));
    EXPECT_EQ(TerminalTestCursor(8,4, U"\33[4;8H\33[1D"),    (std::array{6u,3u, 0u,0u,0u}));
    EXPECT_EQ(TerminalTestCursor(8,4, U"\33[4;8H\33[2D"),    (std::array{5u,3u, 0u,0u,0u}));
    EXPECT_EQ(TerminalTestCursor(8,4, U"\33[4;8H\33[8D"),    (std::array{0u,3u, 0u,0u,0u}));
}
TEST(terminal, scs)
{
    char32_t pattern[] = U"\033)0\016z\017";
    char32_t symbols[] = U"012`45679ABCEfFHKLMQR<=>UYZ";
    for(char32_t c: symbols)
    {
        char32_t expect = 'z';
        pattern[2] = c;
        if(c == U'0') expect = U'\u2265';
        if(c)
        {
            EXPECT_EQ(TerminalTest(pattern, [&](auto& wnd) { return wnd.cells[0].ch; }), expect);
        }
    }
}

#include <fstream>
TEST(terminal, stress_test)
{
    Window wnd(80,25);
    TerminalWindow term(wnd);

    // Issue the patterns from the following files:
    for(std::string s: {"test/escapes.txt", "test/xterm-symbols.txt", "test/xterm-symbols2.txt",
                        "test/ocs_dcs.txt"})
    {
        std::ifstream f(s);
        for(std::string line; std::getline(f,line), f; )
            term.Write(FromUTF8(line));
    }
    std::u32string result(term.OutBuffer.begin(), term.OutBuffer.end());
    EXPECT_EQ(result,
        U"\33[?65;1;6;8;15;22c\33[?65;1;6;8;15;22c\33[0n\33[1;1R\033P!|00000000\x9C\33[>1;1;0c\33[?65;1;6;8;15;22c\33[?25;1$y\33[12;4$y\33[20;4$y"
        U"\x18\033$P1$r0;24r\033\\\033$P1$r38;2;204;204;204;48;2;0;0;0m\033\\\033$P1$r24t\033\\\033$P1$r1 q\033\\\033$P1$r0\"q\033\\\033$P1$r64;1\"p\033\\\033$P1$r80$|\033\\\033$P1$r25*|\033\\"
    );
}

#endif
