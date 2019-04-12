#include <array>
#include <cstdio> // sprintf

#include "terminal.hh"
#include "beeper.hh"
#include "ctype.hh"

static constexpr unsigned Make16(unsigned r,unsigned g,unsigned b)
{
    return (((((unsigned(b))&0x1F)*255/31))
         | ((((unsigned(g)<<1)&0x3F)*255/63)<<8)
         | ((((unsigned(r))&0x1F)*255/31)<<16));
}
static constexpr unsigned char grayramp[24] = { 1,2,3,5,6,7,8,9,11,12,13,14,16,17,18,19,20,22,23,24,25,27,28,29 };
static constexpr unsigned char colorramp[6] = { 0,12,16,21,26,31 };
static constexpr std::array<unsigned,256> xterm256init()
{
    std::array<unsigned,256> result =
    { Make16(0,0,0), Make16(21,0,0), Make16(0,21,0), Make16(21,10,0),
      Make16(0,0,21), Make16(21,0,21), Make16(0,21,21), Make16(21,21,21),
      Make16(15,15,15), Make16(31,10,10), Make16(5,31,10), Make16(31,31,10),
      Make16(10,10,31), Make16(31,10,31), Make16(5,31,31), Make16(31,31,31) };
    for(unsigned n=0; n<216; ++n) { result[16+n] = Make16(colorramp[(n/36)%6], colorramp[(n/6)%6], colorramp[(n)%6]); }
    for(unsigned n=0; n<24; ++n)  { result[232 + n] = Make16(grayramp[n],grayramp[n],grayramp[n]); }
    return result;
}
static constexpr std::array<unsigned,256> xterm256table = xterm256init();

void termwindow::ResetFG()
{
    wnd.blank.fgcolor = xterm256table[7];
}
void termwindow::ResetBG()
{
    wnd.blank.bgcolor = xterm256table[0];
}
void termwindow::ResetAttr()
{
    wnd.blank = Cell{};
    ResetFG();
    ResetBG();
}
void termwindow::Reset()
{
    cx = cy = top = 0;
    bottom = wnd.ysize-1;

    g0set = 0; g1set = 1; activeset = 0; translate = g0set;
    utfmode = 0;
    wnd.fillbox(0,0, wnd.xsize,wnd.ysize); // Clear screen
    state = 0;
    p.clear();
}

void termwindow::Lf()
{
    if(cy >= bottom)
    {
        /* scroll the window up */
        yscroll_up(top, bottom, 1);
    }
    else
    {
        ++cy;
    }
}

void termwindow::yscroll_down(unsigned y1, unsigned y2, int amount) const
{
    unsigned hei = y2-y1+1;
    if(unsigned(amount) > hei) amount = hei;
    wnd.copytext(0,y1+amount, 0,y1, wnd.xsize,(y2-(y1+amount))+1);
    wnd.fillbox(0,y1, wnd.xsize,amount);
}

void termwindow::yscroll_up(unsigned y1, unsigned y2, int amount) const
{
    unsigned hei = y2-y1+1;
    if(unsigned(amount) > hei) amount = hei;
    wnd.copytext(0,y1, 0,y1+amount, wnd.xsize,(y2-amount-y1)+1);
    wnd.fillbox(0,y2-amount+1, wnd.xsize,amount);
}


void termwindow::Write(std::u32string_view s)
{
    enum States: unsigned
    {
        st_default,
        st_esc,
        st_scs0,        // esc (
        st_scs1,        // esc )
        st_scr,         // esc #
        st_esc_percent, // esc %
        st_csi,         // esc [
        st_csi_dec,     // csi ?
        st_csi_dec2,    // csi >
        st_csi_dec3,    // csi =
        //
        st_num_states
    };

    auto GetParams = [&](unsigned min_params, bool change_zero_to_one)
    {
        if(p.size() < min_params) { p.resize(min_params); }
        if(change_zero_to_one) for(auto& v: p) if(!v) v = 1;
        state = st_default;
    };

    for(char32_t c: s)
        #define State(c,st) ((std::uint_fast32_t(c) * st_num_states) + unsigned(st))
        switch(State(c,state))
        {
            #define AnyState(c)      State(c,st_default): case State(c,st_esc): \
                                case State(c,st_scs0):    case State(c,st_scs1): \
                                case State(c,st_scr):     case State(c,st_esc_percent): \
                                case State(c,st_csi):     case State(c,st_csi_dec2): \
                                case State(c,st_csi_dec): case State(c,st_csi_dec3)

            case AnyState(U'\7'):  { BeepOn(); break; }
            case AnyState(U'\b'):  { ScrollFix(); if(cx>0) { --cx; } break; }
            case AnyState(U'\t'):  { ScrollFix(); cx += 8 - (cx & 7); cmov: FixCoord(); break; }
            case AnyState(U'\r'):  { cx=0; break; }
            case AnyState(U'\16'): { activeset = 1; translate = g1set; break; }
            case AnyState(U'\17'): { activeset = 0; translate = g0set; break; }
            case AnyState(U'\177'): { /* del - ignore */ break; }
            case AnyState(U'\30'): [[fallthrough]];
            case AnyState(U'\32'): { Ground: state=st_default; break; }
            case State(U'\33', st_default): state = st_esc; p.clear(); break;
            case State(U'c', st_esc): Reset(); break; // esc c
            case State(U'(', st_esc): state = st_scs0; break; // esc (
            case State(U')', st_esc): state = st_scs1; break; // esc )
            case State(U'#', st_esc): state = st_scr; break; // esc #
            case State(U'[', st_esc): state = st_csi; break; // esc [
            case State(U'%', st_esc): state = st_esc_percent; break; // esc %

            case State(U'0', st_csi): case State(U'1', st_csi):
            case State(U'2', st_csi): case State(U'3', st_csi):
            case State(U'4', st_csi): case State(U'5', st_csi):
            case State(U'6', st_csi): case State(U'7', st_csi):
            case State(U'8', st_csi): case State(U'9', st_csi):
                if(p.empty()) p.emplace_back();
                p.back() = p.back() * 10 + (c - U'0');
                break;
            case State(U':', st_csi): case State(U';', st_csi):
                p.emplace_back();
                break;

            case State(U'E', st_esc): // esc E
                cx = 0;
                state = st_default;
                [[fallthrough]];
            case State(10, st_default):
            case State(11, st_default):
            case State(12, st_default):
                ScrollFix();
                if(cy != bottom)
                    Lf();
                else
                {
                    unsigned pending_linefeeds = 1;
                    yscroll_up(top, bottom, pending_linefeeds);
                    cy -= pending_linefeeds-1;
                    if(cy < top) cy = top;
                }
                break;

            case State(U'M', st_esc): // esc M, Ri
                if(cy <= top)
                {
                    /* scroll the window down */
                    yscroll_down(top, bottom, 1);
                }
                else
                {
                    --cy;
                }
                goto Ground;
            case State(U'Z', st_esc): [[fallthrough]]; // esc Z
            case State(U'c', st_csi): // csi 0 c // Primary device attributes (host computer)
                GetParams(1,false);
                if(!p[0]) EchoBack(U"\33[?65;1;6;8;15;22c");
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
            case State(U'7', st_esc): [[fallthrough]]; // esc 7, csi s
            case State(U's', st_csi): save_cur(); goto Ground;
            case State(U'8', st_esc): [[fallthrough]]; // esc 8, csi u
            case State(U'u', st_csi): restore_cur(); goto Ground;
            case State(U'B', st_scs0): // esc ( B
                g0set = 0; ActG0: if(activeset==0) translate=g0set;
                goto Ground;
            case State(U'0', st_scs0): g0set = 1; goto ActG0; // esc ( 0
            case State(U'U', st_scs0): g0set = 2; goto ActG0; // esc ( U
            case State(U'K', st_scs0): g0set = 3; goto ActG0; // esc ( K
            case State(U'B', st_scs1): // esc ) B
                g1set = 0; ActG1: if(activeset==1) translate=g1set;
                goto Ground;
            case State(U'0', st_scs1): g1set = 1; goto ActG1; // esc ) 0
            case State(U'U', st_scs1): g1set = 2; goto ActG1; // esc ) U
            case State(U'K', st_scs1): g1set = 3; goto ActG1; // esc ) K
            case State(U'8', st_scr): /* TODO: clear screen with 'E' */ goto Ground; // esc # 8
            case State(U'@', st_esc_percent): utfmode = 0; goto Ground; // esc % @
            case State(U'G', st_esc_percent): [[fallthrough]];  // esc % G
            case State(U'8', st_esc_percent): utfmode = 1; goto Ground; // esc % 8
            case State(U'g', st_csi): /* TODO: set tab stops */ goto Ground;
            case State(U'q', st_csi): /* TODO: set leds */ goto Ground;
            case State(U'G', st_csi): [[fallthrough]];
            case State(U'`', st_csi): { GetParams(1,true); cx=p[0]-1; goto cmov; }
            case State(U'd', st_csi): { GetParams(1,true); cy=p[0]-1; goto cmov; }
            case State(U'F', st_csi): cx=0; [[fallthrough]];
            case State(U'A', st_csi): { GetParams(1,true); cy-=p[0];  goto cmov; }
            case State(U'E', st_csi): cx=0; [[fallthrough]];
            case State(U'B', st_csi): { GetParams(1,true); cy+=p[0];  goto cmov; }
            case State(U'C', st_csi): { GetParams(1,true); cx+=p[0];  goto cmov; }
            case State(U'D', st_csi): { GetParams(1,true); cx-=p[0];  goto cmov; }
            case State(U'H', st_csi): [[fallthrough]];
            case State(U'f', st_csi): { GetParams(2,true); cx=p[1]-1; cy=p[0]-1; goto cmov; }
            case State(U'J', st_csi):
                GetParams(1,false);
                switch(p[0])
                {
                    case 0: // erase from cursor to end of display
                        if(unsigned(cy) < wnd.ysize-1)
                            wnd.fillbox(0,cy+1, wnd.xsize, wnd.ysize-cy-1);
                        goto clreol;
                    case 1: // erase from start to cursor
                        if(cy > 0) wnd.fillbox(0,0, wnd.xsize,cy);
                        goto clrbol;
                    case 2: // erase whole display
                        wnd.fillbox(0,0, wnd.xsize,wnd.ysize);
                        break;
                }
                break;
            case State(U'K', st_csi):
                GetParams(1,false);
                // 0: erase from cursor to end of line
                // 1: erase from start of line to cursor
                // 2: erase whole line
                switch(p[0])
                {
                    case 0: clreol: wnd.fillbox(cx,cy, wnd.xsize-cx, 1); break;
                    case 1: clrbol: wnd.fillbox(0, cy, cx+1,         1); break;
                    case 2: wnd.fillbox(0, cy, wnd.xsize,    1); break;
                }
                break;
            case State(U'L', st_csi):
                GetParams(1,true);
                // scroll the rest of window c lines down,
                // including where cursor is. Don't move cursor.
                yscroll_down(cy, bottom, p[0]);
                break;
            case State(U'M', st_csi):
                GetParams(1,true);
                yscroll_up(cy, bottom, p[0]);
                break;
            case State(U'P', st_csi):
                GetParams(1,true); c = p[0];
                // insert c black holes at cursor (eat c characters
                // and scroll line horizontally to left)
                if(cx + c > wnd.xsize) c = wnd.xsize-cx;
                if(c)
                {
                    unsigned remain = wnd.xsize - (cx+c);
                    wnd.copytext(cx,cy, wnd.xsize-remain,cy, remain,1);
                    wnd.fillbox(wnd.xsize-c,cy, c,1);
                }
                break;
            case State(U'X', st_csi):
                GetParams(1,true);
                // write c spaces at cursor (overwrite)
                wnd.fillbox(cx,cy+top, p[0],1);
                break;
            case State(U'@', st_csi):
                GetParams(1,true); c = p[0];
                // insert c spaces at cursor
                if(cx + c > wnd.xsize) c = wnd.xsize-cx;
                if(c)
                {
                    unsigned remain = wnd.xsize - (cx+c);
                    wnd.copytext(cx+c,cy, cx,cy, remain,1);
                    wnd.fillbox(cx,cy, c,1);
                }
                break;
            case State(U'r', st_csi):
                GetParams(2,false);
                if(!p[0]) p[0]=1;
                if(!p[1]) p[1]=wnd.ysize;
                if(p[0] < p[1] && p[1] <= wnd.ysize)
                {
                    top = p[0]-1; bottom = p[1]-1;
                    cx=0; cy=top;
                    goto cmov;
                }
                break;
            case State(U'n', st_csi):
            {
                GetParams(1,false);
                char Buf[32];
                switch(p[0])
                {
                    case 5: EchoBack(U"\33[0n"); break;
                    case 6: EchoBack(FromUTF8(std::string_view{Buf, (std::size_t)std::sprintf(Buf, "\33[%d;%dR", cy+1, cx+1)})); break;
                }
                break;
            }
            case State(U'c', st_csi_dec3): // csi = 0 c, Tertiary device attributes (printer?)
                GetParams(1,false); // Tertiary device attributes (printer?)
                // Example response: ^[P!|0^[ (backslash) 
                if(!p[0]) EchoBack(U"\33P!|00000000\x9C");
                break;
            case State(U'c', st_csi_dec2): // csi > 0 c, Secondary device attributes (terminal)
                GetParams(1,false);
                // Example response: ^[[>41;330;0c  (middle=firmware version)
                if(!p[0]) EchoBack(U"\33[>1;1;0c");
                break;
            case State(U'h', st_csi_dec): // csi ? h, misc modes on
                goto Ground;
            case State(U'l', st_csi_dec): // csi ? l, misc modes off
                /* esc[?3;4l = monitor off, insert mode off */
                /* esc>      = numeric keypad off */
                goto Ground;
            case State(U'b', st_csi):
                GetParams(1,true);
                // TODO: Repeat last printed character n times
                break;
            case State(U'm', st_csi): // csi m
                GetParams(1, false); // Make sure there is at least 1 param
                c=0;
                for(auto a: p)
                    switch(c*256 + (a & 0xFFu))
                    {
                        #define case4(n) case n:case n+1:case n+2:case n+3
                        #define case16(n) case4(n):case4(n+4):case4(n+8):case4(n+12)
                        #define case64(n) case16(n):case16(n+16):case16(n+32):case16(n+48)
                        #define casen_256(m) case64(m*256+0):case64(m*256+64):case64(m*256+128):case64(m*256+192)
                        #define case012(n) case n: case 1*256+n: case 2*256+n
                        #define case012_7(n) case012(n+0):case012(n+1):case012(n+2):case012(n+3):\
                                             case012(n+4):case012(n+5):case012(n+6):case012(n+7)
                        case012(0): ResetAttr(); break;
                        case012(1): wnd.blank.bold = true; break;
                        case (2): wnd.blank.dim = true; break;
                        case (3): wnd.blank.italic = true; break;
                        case (4): wnd.blank.underline = true; break;
                        case (5): wnd.blank.blink = true; break;
                        case012(7): wnd.blank.reverse = true; break;
                        case012(8): wnd.blank.conceal = true; break;
                        case012(9): wnd.blank.overstrike = true; break;
                        case012(20): wnd.blank.fraktur = true; break;
                        case012(21): wnd.blank.underline2 = true; break;
                        case012(22): wnd.blank.dim = false; wnd.blank.bold = false; break;
                        case012(23): wnd.blank.italic = false; wnd.blank.fraktur = false; break;
                        case012(24): wnd.blank.underline = false; wnd.blank.underline2 = false; break;
                        case012(25): wnd.blank.blink = false; break;
                        case012(27): wnd.blank.reverse = false; break;
                        case012(28): wnd.blank.conceal = false; break;
                        case012(29): wnd.blank.overstrike = false; break;
                        case012_7(30): wnd.blank.fgcolor = xterm256table[a-30]; break;
                        case012(38): c = 1; break;
                        case012(39): wnd.blank.underline = false; wnd.blank.underline2 = false; ResetFG(); break; // Set default foreground color
                        case012_7(40): wnd.blank.bgcolor = xterm256table[a-40]; break;
                        case012(48): c = 2; break;
                        case012(49): ResetBG(); break; // Set default background color
                        case012(51): wnd.blank.framed = true; break;
                        case012(52): wnd.blank.encircled = true; break;
                        case012(53): wnd.blank.overlined = true; break;
                        case012(54): wnd.blank.framed = false; wnd.blank.encircled = false; break;
                        case012(55): wnd.blank.overlined = false; break;
                        case012_7(90): wnd.blank.fgcolor = xterm256table[a-90 + 8]; break;
                        case012_7(100): wnd.blank.bgcolor = xterm256table[a-100 + 8]; break;
                        case 1*256+5: c=3; break; // 38;5
                            casen_256(3): wnd.blank.fgcolor = xterm256table[a&0xFF]; c = 0; break; // 38;5;n
                        case 1*256+2: c=4; break; // 38;2
                        case 1*256+3: c=5; break; // 38;3
                        case 1*256+4: c=6; break; // 38;4
                            casen_256(4): //fgc = a << 16; c+=3; break; // 38;2;n
                            casen_256(5): //fgc = a << 16; c+=3; break; // 38;3;n
                            casen_256(6): wnd.blank.fgcolor = a << 16; c+=3; break; // 38;4;n
                            casen_256(7): //fgc += a << 8; c+=3; break; // 38;2;#;n
                            casen_256(8): //fgc += a << 8; c+=3; break; // 38;3;#;n
                            casen_256(9): wnd.blank.fgcolor += a << 8; c+=3; break; // 38;4;#;n
                            casen_256(10): //fgc += a << 0; c = 0; break; // 38;2;#;#;n (RGB24)
                            casen_256(11): wnd.blank.fgcolor += a << 0; c = 0; break; // 38;3;#;#;n (TODO CMY->RGB)
                            casen_256(12): wnd.blank.fgcolor += a << 0; ++c; break;   // 38;4;#;#;n
                            casen_256(13): c = 0; break;                // 38;4;#;#;#;n  (TODO CMYK->RGB)
                        case 2*256+5: c=14; break; // 48;5
                            casen_256(14): wnd.blank.bgcolor = xterm256table[a&0xFF]; c = 0; break; // 48;5;n
                        case 2*256+2: c=15; break; // 48;2
                        case 2*256+3: c=16; break; // 48;3
                        case 2*256+4: c=17; break; // 48;4
                            casen_256(15): //bgc = a << 16; c+=3; break; // 48;2;n
                            casen_256(16): //bgc = a << 16; c+=3; break; // 48;3;n
                            casen_256(17): wnd.blank.bgcolor = a << 16; c+=3; break; // 48;4;n
                            casen_256(18): //bgc += a << 8; c+=3; break; // 48;2;#;n
                            casen_256(19): //bgc += a << 8; c+=3; break; // 48;3;#;n
                            casen_256(20): wnd.blank.bgcolor += a << 8; c+=3; break; // 48;4;#;n
                            casen_256(21):// bgc += a << 0; c = 0; break; // 48;2;#;#;n (RGB24)
                            casen_256(22): wnd.blank.bgcolor += a << 0; c = 0; break; // 48;3;#;#;n (TODO CMY->RGB)
                            casen_256(23): wnd.blank.bgcolor += a << 0; ++c; break;   // 48;4;#;#;n
                            casen_256(24): c = 0; break;                // 48;4;#;#;#;n  (TODO CMYK->RGB)
                        default: c = 0; break;
                        #undef case4
                        #undef case16
                        #undef case64
                        #undef casen_256
                        #undef case012
                        #undef case012_7
                    }
                break;

            default:
                if(state != st_default) goto Ground;
                ScrollFix();
                wnd.PutCh(cx,cy, c, translate);
                ++cx;
                break;
        }
        #undef AnyState
        #undef State

    if((cx+1 != int(wnd.xsize)
     || cy+1 != int(wnd.ysize))
    )
    {
        wnd.cursx = cx;
        wnd.cursy = cy;
    }
}

void termwindow::EchoBack(std::u32string_view buffer)
{
    //Write(buffer, size); // DEBUGGING
    OutBuffer.insert(OutBuffer.end(), buffer.begin(), buffer.end());
}

void termwindow::save_cur()
{
    backup.cx = cx;
    backup.cy = cy;
    backup.top = top;
    backup.bottom = bottom;
    backup.attr = wnd.blank;
}

void termwindow::restore_cur()
{
    cx = backup.cx;
    cy = backup.cy;
    top = backup.top;
    bottom = backup.bottom;
    wnd.blank = backup.attr;
}

void termwindow::FixCoord()
{
    if(bottom>=int(wnd.ysize))bottom=wnd.ysize-1;
    if(top>bottom-2)top=bottom-2;
    if(top<0)top=0;
    if(bottom<top+2)bottom=top+2;
    if(cx<0)cx=0;
    if(cy<0)cy=0;
    if(cx>=int(wnd.xsize))cx=wnd.xsize-1;
    if(cy>=bottom)cy=bottom;
}
void termwindow::ScrollFix()
{
    if(cx >= int(wnd.xsize)) { cx = 0; Lf(); }
}
