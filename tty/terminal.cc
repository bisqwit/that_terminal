#include <array>
#include <cstdio> // sprintf

#include <SDL.h> // for window title

#include "terminal.hh"
#include "beeper.hh"
#include "ctype.hh"
#include "256color.hh"
#include "color.hh"

void termwindow::ResetFG()
{
    wnd.blank.fgcolor = Cell{}.fgcolor;
}
void termwindow::ResetBG()
{
    wnd.blank.bgcolor = Cell{}.bgcolor;
}
void termwindow::ResetAttr()
{
    bool prot = wnd.blank.protect;
    wnd.blank = Cell{};
    wnd.blank.protect = prot;
    //ResetFG();
    //ResetBG();
}
void termwindow::Reset(bool full)
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
        wnd.fillbox(0,0, wnd.xsize,wnd.ysize, wnd.blank); // Clear screen
    }
}

void termwindow::yscroll_down(unsigned y1, unsigned y2, int amount) const
{
    if(amount <= 0) return;
    unsigned hei = y2-y1+1;
    if(unsigned(amount) > hei) amount = hei;
    //fprintf(stderr, "Height=%d, amount=%d, scrolling DOWN by %d lines\n", hei,amount, hei-amount);
    wnd.copytext(0,y1+amount, 0,y1, wnd.xsize,hei-amount);
    wnd.fillbox(0,y1, wnd.xsize,amount);
}

void termwindow::yscroll_up(unsigned y1, unsigned y2, int amount) const
{
    if(amount <= 0) return;
    unsigned hei = y2-y1+1;
    if(unsigned(amount) > hei) amount = hei;
    //fprintf(stderr, "Height=%d, amount=%d, scrolling UP by %d lines\n", hei,amount, hei-amount);
    wnd.copytext(0,y1, 0,y1+amount, wnd.xsize,hei-amount);
    wnd.fillbox(0,y2-amount+1, wnd.xsize,amount);
}


void termwindow::Write(std::u32string_view s)
{
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

    auto GetParams = [&](unsigned min_params, bool change_zero_to_one)
    {
        if(p.size() < min_params) { p.resize(min_params); }
        if(change_zero_to_one) for(auto& v: p) if(!v) v = 1;
        state = st_default;
    };

    unsigned color = 0;
    auto State = [](char32_t c, unsigned st) constexpr
    {
        return c*st_num_states + st;
    };
    auto ClampedMoveX = [&](int tgtx)
    {
        wnd.cursx = std::min(std::size_t(std::max(0,tgtx)), wnd.xsize-1);
        edgeflag = false;
    };
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
    auto ClampedMove = [&](int tgtx, int tgty, bool strict = true)
    {
        ClampedMoveX(tgtx);
        ClampedMoveY(tgty, strict);
    };
    auto Lf = [&]
    {
        if(wnd.cursy == bottom)
            yscroll_up(top, bottom, 1);
        else
            ClampedMoveY(wnd.cursy+1);
    };
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
    auto ProcessSGR = [&](char32_t& c, unsigned a,
                          auto&& reset_attr,
                          auto&& change_attr)
    {
        /* c is a state code internal to this function, a is the input number from the SGR code.
         * Codes:
         *   0 = default state          -- Will parse "a" (input number) as verbatim
         *   1 = previous was 38
         *                              -- Will parse 2 as 82+(n-2)+(c-1)*4 = 78 = flag(FLAG38_x,2)                  - 38;2
         *                              -- Will parse 3 as 82+(n-2)+(c-1)*4 = 79 = flag(FLAG38_x,3)                  - 38;3
         *                              -- Will parse 4 as 82+(n-2)+(c-1)*4 = 80 = flag(FLAG38_x,4)                  - 38;4
         *                              -- Will parse 5 as 82+(n-2)+(c-1)*4 = 81 = flag(FLAG38_x,5)                  - 38;5
         *                              -- Parses anything other as in state 0 (38 is ignored)
         *   2 = previous was 48
         *                              -- Will parse 2 as 82+(n-2)+(c-1)*4 = 82 = flag(FLAG48_x,2)                  - 48;2
         *                              -- Will parse 3 as 82+(n-2)+(c-1)*4 = 83 = flag(FLAG48_x,3)                  - 48;3
         *                              -- Will parse 4 as 82+(n-2)+(c-1)*4 = 84 = flag(FLAG48_x,4)                  - 48;4
         *                              -- Will parse 5 as 82+(n-2)+(c-1)*4 = 85 = flag(FLAG48_x,4)                  - 48;5
         *                              -- Parses anything other as in state 0 (48 is ignored)
         *   3 = previous was 58
         *                              -- Will parse 2 as 82+(n-2)+(c-1)*4 = 86 = flag(FLAG58_x,2)                  - 58;2
         *                              -- Will parse 3 as 82+(n-2)+(c-1)*4 = 87 = flag(FLAG58_x,3)                  - 58;3
         *                              -- Will parse 4 as 82+(n-2)+(c-1)*4 = 88 = flag(FLAG58_x,4)                  - 58;4
         *                              -- Will parse 5 as 82+(n-2)+(c-1)*4 = 89 = flag(FLAG58_x,4)                  - 58;5
         *                              -- Parses anything other as in state 0 (58 is ignored)
         *   4 = previous two were 38;4 -- Will parse as 108+4-4 = 108; 117; 126; 135= mode(MODE38_4 .. MODE38_4_x3) - CMYK fg
         *   5 = previous two were 38;3 -- Will parse as 108+5-4 = 109; 118; 127     = mode(MODE38_3 .. MODE38_3_x2) - CMY fg
         *   6 = previous two were 38;2 -- Will parse as 108+6-4 = 110; 119; 128     = mode(MODE38_2 .. MODE38_2_x2) - RGB fg
         *   7 = previous two were 48;4 -- Will parse as 108+7-4 = 111; 120; 129; 138= mode(MODE48_4 .. MODE48_4_x3) - CMYK bg
         *   8 = previous two were 48;3 -- Will parse as 108+8-4 = 112; 121; 130     = mode(MODE48_3 .. MODE48_3_x2) - CMY bg
         *   9 = previous two were 48;2 -- Will parse as 108+9-4 = 113; 122; 131     = mode(MODE48_2 .. MODE48_2_x2) - RGB bg
         *  10 = previous two were 58;4 -- Will parse as 108+10-4= 114; 123; 132; 141= mode(MODE48_4 .. MODE48_4_x3) - CMYK underline (UNSUP)
         *  11 = previous two were 58;3 -- Will parse as 108+11-4= 115; 124; 133     = mode(MODE48_3 .. MODE48_3_x2) - CMY underline (UNSUP)
         *  12 = previous two were 58;2 -- Will parse as 108+12-4= 116; 125; 134     = mode(MODE48_2 .. MODE48_2_x2) - RGB underline (UNSUP)
         *  32 = previous two were 38;5 -- Will parse as 108+32-4 = 136              = mode(MODE38_5)                - Xterm256 fg
         *  33 = previous two were 48;5 -- Will parse as 108+33-4 = 137              = mode(MODE48_5)                - Xterm256 bg
         *  35 = previous two were 58;5 -- Will parse as 108+35-4 = 139              = mode(MODE58_5)                - Xterm256 underline (UNSUP)
         *                mode() translates the state code into such numbers that does not conflict normally expected values.
         *
         * Regular switch-values:
         *    0-9, 20-24, 27-49, 51-55, 73-75, 90-97, 100-107
         * Codes that only indicate internal state changes and will cause wonky things to happen, if encountered in SGR when c is 0-2:
         *    78                                     mistakenly interpreted as 38;2
         *    79                                     mistakenly interpreted as 38;3
         *    80                                     mistakenly interpreted as 38;4
         *    81                                     mistakenly interpreted as 38;5
         *    82, 108-134, 127-128, 130-131, 133-134 mistakenly interpreted as 48;2 -- expects 2 more values
         *    83                                     mistakenly interpreted as 48;3
         *    84                                     mistakenly interpreted as 48;4
         *    85                                     mistakenly interpreted as 48;5
         *    86                                     mistakenly interpreted as 58;2
         *    87                                     mistakenly interpreted as 58;3
         *    88                                     mistakenly interpreted as 58;4
         *    89                                     mistakenly interpreted as 58;5
         *   126                                     mistakenly interpreted as 38;4;?;?;?;126 -- sets color to undefined value
         *   129                                     mistakenly interpreted as 48;4;?;?;?;129 -- sets color to undefined value
         *   136                                     mistakenly interpreted as 38;5;136
         *   137                                     mistakenly interpreted as 48;5;137
         *   139                                     mistakenly interpreted as 58;5;139
         * Unused/unsupported switch-values:
         *    10      primary (default) font
         *    11-19   alternative font n-10
         *    56-57   UNDEFINED
         *    66-72   UNDEFINED
         *    76-77   UNDEFINED
         *    98-99   UNDEFINED
         *    140     UNDEFINED
         *    142-    UNDEFINED
         */
        constexpr unsigned MODE_BASE = 108;
        constexpr unsigned MODE38_5  = 136 + 4 - MODE_BASE; // Which value for C to use to get 136
        constexpr unsigned MODE48_5  = 137 + 4 - MODE_BASE; // Which value for C to use to get 137
        constexpr unsigned MODE58_5  = 139 + 4 - MODE_BASE; // Which value for C to use to get 139
        constexpr unsigned MODE38_4  = 4, MODE38_4_x1 = MODE38_4+9, MODE38_4_x2 = MODE38_4+9*2, MODE38_4_x3 = MODE38_4+9*3;
        constexpr unsigned MODE38_3  = 5, MODE38_3_x1 = MODE38_3+9, MODE38_3_x2 = MODE38_3+9*2;
        constexpr unsigned MODE38_2  = 6, MODE38_2_x1 = MODE38_2+9, MODE38_2_x2 = MODE38_2+9*2;
        constexpr unsigned MODE48_4  = 7, MODE48_4_x1 = MODE48_4+9, MODE48_4_x2 = MODE48_4+9*2, MODE48_4_x3 = MODE48_4+9*3;
        constexpr unsigned MODE48_3  = 8, MODE48_3_x1 = MODE48_3+9, MODE48_3_x2 = MODE48_3+9*2;
        constexpr unsigned MODE48_2  = 9, MODE48_2_x1 = MODE48_2+9, MODE48_2_x2 = MODE48_2+9*2;
        constexpr unsigned MODE58_4  =10, MODE58_4_x1 = MODE58_4+9, MODE58_4_x2 = MODE58_4+9*2, MODE58_4_x3 = MODE58_4+9*3;
        constexpr unsigned MODE58_3  =11, MODE58_3_x1 = MODE58_3+9, MODE58_3_x2 = MODE58_3+9*2;
        constexpr unsigned MODE58_2  =12, MODE58_2_x1 = MODE58_2+9, MODE58_2_x2 = MODE58_2+9*2;
        constexpr unsigned FLAG_BASE = 78;
        constexpr unsigned FLAG_38_x = 1, FLAG_48_x = 2, FLAG_58_x = 3;

        auto mode = [](unsigned n)            constexpr { return MODE_BASE + n - 4; };
        auto flag = [](unsigned c,unsigned n) constexpr { return FLAG_BASE + (n - 2) + (c-FLAG_38_x)*4; };
        unsigned switchval = a;
        switch(c)
        {
            case 0:         [[likely]] break;
            case FLAG_38_x: [[fallthrough]];
            case FLAG_48_x:
            case FLAG_58_x: if(a >= 2 && a <= 5) [[likely]] switchval = flag(c,a); else c = 0;
                            break;
            default:        switchval = mode(c);
        }
        auto m = [](unsigned n) constexpr
        {
            //if((n>=10 && n<=19) || (n>=56 && n<=81) || n>=130) n=26;
            //if(n>=82) n = n+56-82;
            //if(n>=20) n = n+10-20;
            return n;
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
            case m(8): set(conceal, true); c = 0; break;
            case m(9): set(overstrike, true); c = 0; break;
            case m(20): set(fraktur, true); c = 0; break;
            case m(21): set(underline2, true); c = 0; break;
            case m(22): set(dim, false); set(bold, false); c = 0; break;
            case m(23): set(italic, false); set(fraktur, false); c = 0; break;
            case m(24): set(underline, false); set(underline2, false); c = 0; break;
            case m(25): set(blink, 0); c = 0; break;
            case m(26): set(proportional, true); c = 0; break;
            case m(27): set(inverse, false); c = 0; break;
            case m(28): set(conceal, false); c = 0; break;
            case m(29): set(overstrike, false); c = 0; break;
            case m(39): set(underline, false); set(underline2, false);
                        set(fgcolor, Cell{}.fgcolor); c = 0; break; // Set default foreground color
            case m(49): set(bgcolor, Cell{}.bgcolor); c = 0; break; // Set default background color
            case m(59): c = 0; break; // Set default underline color
            case m(50): set(proportional, false); c = 0; break;
            case m(51): set(framed, true); c = 0; break;
            case m(52): set(encircled, true); c = 0; break;
            case m(53): set(overlined, true); c = 0; break;
            case m(54): set(framed, false); set(encircled, false); c = 0; break;
            case m(55): set(overlined, false); c = 0; break;
            case m(60): set(ideo_underline, true); c = 0; break;
            case m(61): set(ideo_underline2, true); c = 0; break;
            case m(62): set(ideo_overline, true); c = 0; break;
            case m(63): set(ideo_overline2, true); c = 0; break;
            case m(64): set(ideo_stress, true); c = 0; break;
            case m(65): set(ideo_underline, false); set(ideo_underline2, false);
                        set(ideo_overline, false); set(ideo_overline2, false);
                        set(ideo_stress, false); c = 0; break;
            case m(73): set(scriptsize, 1); c = 0; break;
            case m(74): set(scriptsize, 2); c = 0; break;
            case m(75): set(scriptsize, 0); c = 0; break;
            case m(38): c = FLAG_38_x; break;
            case m(48): c = FLAG_48_x; break;
            case m(58): c = FLAG_58_x; break;
            case m(flag(FLAG_38_x,2)): c=MODE38_2; color=0; break; // 38;2
            case m(flag(FLAG_38_x,3)): c=MODE38_3; color=0; break; // 38;3
            case m(flag(FLAG_38_x,4)): c=MODE38_4; color=0; break; // 38;4
            case m(flag(FLAG_38_x,5)): c=MODE38_5; break;          // 38;5
            case m(flag(FLAG_48_x,2)): c=MODE48_2; color=0; break; // 48;2
            case m(flag(FLAG_48_x,3)): c=MODE48_3; color=0; break; // 48;3
            case m(flag(FLAG_48_x,4)): c=MODE48_4; color=0; break; // 48;4
            case m(flag(FLAG_48_x,5)): c=MODE48_5; break;          // 48;5
            case m(flag(FLAG_58_x,2)): c=MODE58_2; color=0; break; // 58;2
            case m(flag(FLAG_58_x,3)): c=MODE58_3; color=0; break; // 58;3
            case m(flag(FLAG_58_x,4)): c=MODE58_4; color=0; break; // 58;4
            case m(flag(FLAG_58_x,5)): c=MODE58_5; break;          // 58;5
            case m(mode(MODE38_4)): //color = (color << 8) + a; c+=9; break; // 38;4;n
            case m(mode(MODE38_3)): //color = (color << 8) + a; c+=9; break; // 38;3;n
            case m(mode(MODE38_2)): //color = (color << 8) + a; c+=9; break; // 38;2;n
            case m(mode(MODE48_4)): //color = (color << 8) + a; c+=9; break; // 48;4;n
            case m(mode(MODE48_3)): //color = (color << 8) + a; c+=9; break; // 48;3;n
            case m(mode(MODE48_2)): //color = (color << 8) + a; c+=9; break; // 48;2;n
            case m(mode(MODE58_4)): //color = (color << 8) + a; c+=9; break; // 58;4;n
            case m(mode(MODE58_3)): //color = (color << 8) + a; c+=9; break; // 58;3;n
            case m(mode(MODE58_2)): //color = (color << 8) + a; c+=9; break; // 58;2;n
            case m(mode(MODE38_4_x1)): //color = (color << 8) + a; c+=9; break; // 38;4;#;n
            case m(mode(MODE38_3_x1))://color = (color << 8) + a; c+=9; break; // 38;3;#;n
            case m(mode(MODE38_2_x1))://color = (color << 8) + a; c+=9; break; // 38;2;#;n
            case m(mode(MODE48_4_x1))://color = (color << 8) + a; c+=9; break; // 48;4;#;n
            case m(mode(MODE48_3_x1))://color = (color << 8) + a; c+=9; break; // 48;3;#;n
            case m(mode(MODE48_2_x1))://color = (color << 8) + a; c+=9; break; // 48;2;#;n
            case m(mode(MODE58_4_x1))://color = (color << 8) + a; c+=9; break; // 58;4;#;n
            case m(mode(MODE58_3_x1))://color = (color << 8) + a; c+=9; break; // 58;3;#;n
            case m(mode(MODE58_2_x1))://color = (color << 8) + a; c+=9; break; // 58;2;#;n
            case m(mode(MODE38_4_x2))://color = (color << 8) + a; c+=9; break; // 38;4;#;#;n
            case m(mode(MODE48_4_x2))://color = (color << 8) + a; c+=9; break; // 48;4;#;#;n
            case m(mode(MODE58_4_x2)):  color = (color << 8) + a; c+=9; break; // 58;4;#;#;n

            case m(30):case m(31):case m(32):case m(33):case m(34):case m(35):
            case m(36):case m(37):    a -= 30; a += 90-8;                 [[fallthrough]];
            case m(90):case m(91):case m(92):case m(93):case m(94):case m(95):
            case m(96):case m(97):    a -= 90-8;                          [[fallthrough]];
            case m(mode(MODE38_5)):  color = 0; a = xterm256table[a & 0xFF]; [[fallthrough]];           // 38;5;n
            case m(mode(MODE38_4_x3))://color = (color << 8) + a; set(fgcolor, color); c = 0; break; // 38;4;#;#;#;n (TODO CMYK->RGB)
            case m(mode(MODE38_3_x2))://color = (color << 8) + a; set(fgcolor, color); c = 0; break; // 38;3;#;#;n   (TODO CMY->RGB)
            case m(mode(MODE38_2_x2)):  color = (color << 8) + a; set(fgcolor, color); c = 0; break; // 38;2;#;#;n   (RGB24)

            case m(40):case m(41):case m(42):case m(43):case m(44):case m(45):
            case m(46):case m(47):    a -= 40; a += 100-8;                [[fallthrough]];
            case m(100):case m(101):case m(102):case m(103):case m(104):case m(105):
            case m(106):case m(107):  a -= 100-8;                         [[fallthrough]];
            case m(mode(MODE48_5)):  color = 0; a = xterm256table[a & 0xFF]; [[fallthrough]];           // 48;5;n
            case m(mode(MODE48_4_x3))://color = (color << 8) + a; set(bgcolor, color); c = 0; break; // 48;4;#;#;#;n (TODO CMYK->RGB)
            case m(mode(MODE48_3_x2))://color = (color << 8) + a; set(bgcolor, color); c = 0; break; // 48;3;#;#;n   (TODO CMY->RGB)
            case m(mode(MODE48_2_x2)):  color = (color << 8) + a; set(bgcolor, color); c = 0; break; // 48;2;#;#;n   (RGB24)

            case m(mode(MODE58_5)):  color = 0; a = xterm256table[a & 0xFF]; [[fallthrough]];           // 58;5;n
            case m(mode(MODE58_4_x3))://color = (color << 8) + a; /*IGNORED*/ c = 0; break;             // 58;4;#;#;#;n (TODO CMYK->RGB)
            case m(mode(MODE58_3_x2))://color = (color << 8) + a; /*IGNORED*/ c = 0; break;             // 58;3;#;#;n   (TODO CMY->RGB)
            case m(mode(MODE58_2_x2)):  color = (color << 8) + a; /*IGNORED*/ c = 0; break;             // 58;2;#;#;n   (RGB24)

            default: c = 0; break;
            #undef set
        }
    };

    if(bottom >= wnd.ysize) bottom = wnd.ysize-1;
    if(top    > bottom)     top = bottom;

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
            #define AnyState(c)      State(c,st_default): case State(c,st_esc): \
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
            case State(U'*', st_esc): scs = 2; state = st_scs; break; // esc )
            case State(U'+', st_esc): scs = 3; state = st_scs; break; // esc )
            case State(U'-', st_esc): scs = 1; state = st_scs; break; // esc )
            case State(U'.', st_esc): scs = 2; state = st_scs; break; // esc )
            case State(U'/', st_esc): scs = 3; state = st_scs; break; // esc )
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
                                    bool     trans=false;
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
                            extern SDL_Window* window;
                            switch(p[0] % 100)
                            {
                                case 0:
                                {
                                    SDL_SetWindowTitle(window, ToUTF8(string).c_str());
                                    break; // set icon name and window title
                                }
                                case 1: break; // set icon name
                                case 2:
                                {
                                    SDL_SetWindowTitle(window, ToUTF8(string).c_str());
                                    break; // set window title
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
                lastch=c; BeepOn(); break;
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
                break;

            //case AnyState(U'\u008D'):
            case State(U'M', st_esc): // esc M = CASE_RI
                /* Within window: move cursor up; scroll the window down if at top */
                if(wnd.cursy == top)
                    yscroll_down(top, bottom, 1);
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
            case State(U's', st_csi): save_cur(); goto Ground;
            case State(U'8', st_esc): [[fallthrough]]; // esc 8 (DECRC), csi u (ANSI_RC)
            case State(U'u', st_csi): restore_cur(); goto Ground;
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
                wnd.fillbox(0,0, wnd.xsize,wnd.ysize);
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
                            wnd.fillbox(0,wnd.cursy+1, wnd.xsize, wnd.ysize-wnd.cursy-1, wnd.blank);
                        goto clreol;
                    case 1: // erase from start to cursor
                        if(wnd.cursy > 0) wnd.fillbox(0,0, wnd.xsize,wnd.cursy, wnd.blank);
                        goto clrbol;
                    case 2: // erase whole display
                        wnd.fillbox(0,0, wnd.xsize,wnd.ysize, wnd.blank);
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
                    case 0: clreol: wnd.fillbox(wnd.cursx,wnd.cursy, wnd.xsize-wnd.cursx, 1, wnd.blank); break;
                    case 1: clrbol: wnd.fillbox(0,        wnd.cursy, wnd.cursx+1,  1, wnd.blank); break;
                    case 2: wnd.fillbox(0, wnd.cursy, wnd.xsize,    1, wnd.blank); break;
                }
                break;
            case State(U'M', st_csi):
                GetParams(1,true);
                yscroll_up(wnd.cursy, bottom, p[0]);
                break;
            case State(U'L', st_csi):
                GetParams(1,true);
                // scroll the rest of window c lines down,
                // including where cursor is. Don't move cursor.
                yscroll_down(wnd.cursy, bottom, p[0]);
                break;
            case State(U'S', st_csi): // xterm version?
                GetParams(1,true);
                yscroll_up(top, bottom, p[0]);
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
                yscroll_down(top, bottom, p[0]);
                break;
            case State(U'P', st_csi):
                GetParams(1,true); c = std::min(p[0], unsigned(wnd.xsize-wnd.cursx));
                // insert c black holes at cursor (eat c characters
                // and scroll line horizontally to left)
                if(c)
                {
                    unsigned remain = wnd.xsize - (wnd.cursx+c);
                    wnd.copytext(wnd.cursx,wnd.cursy, wnd.xsize-remain,wnd.cursy, remain,1);
                    wnd.fillbox(wnd.xsize-c,wnd.cursy, c,1);
                }
                break;
            case State(U'X', st_csi): // (ECH)
                GetParams(1,true);
                // write c spaces at cursor (overwrite)
                wnd.fillbox(wnd.cursx,wnd.cursy, std::min(std::size_t(p[0]), wnd.xsize-wnd.cursx), 1);
                break;
            case State(U'@', st_csi):
                GetParams(1,true); c = std::min(p[0], unsigned(wnd.xsize-wnd.cursx));
                // insert c spaces at cursor
                if(c)
                {
                    unsigned remain = wnd.xsize - (wnd.cursx+c);
                    wnd.copytext(wnd.cursx+c,wnd.cursy, wnd.cursx,wnd.cursy, remain,1);
                    wnd.fillbox(wnd.cursx,wnd.cursy, c,1);
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
                        wnd.copytext(pld,ptd, pls,pts, width,height);
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

void termwindow::EchoBack(std::u32string_view buffer)
{
    //Write(buffer, size); // DEBUGGING
    OutBuffer.insert(OutBuffer.end(), buffer.begin(), buffer.end());
}

void termwindow::save_cur()
{
    backup.cx = wnd.cursx;
    backup.cy = wnd.cursy;
    backup.attr = wnd.blank;
}

void termwindow::restore_cur()
{
    wnd.cursx = backup.cx;
    wnd.cursy = backup.cy;
    wnd.blank = backup.attr;
}

void termwindow::Resize(std::size_t newsx, std::size_t newsy)
{
    if(bottom == wnd.ysize-1)
    {
        bottom = newsy-1;
        //fprintf(stderr, "Creating a window with top=%zu, bottom=%zu\n", top,bottom);
    }
    wnd.Resize(newsx, newsy);
}
