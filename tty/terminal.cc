#include <array>
#include <cstdio> // sprintf

#include "terminal.hh"
#include "beeper.hh"
#include "ctype.hh"
#include "256color.hh"

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
    lastch = U' ';
    wnd.reverse   = false;
    wnd.cursorvis = true;
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
    fprintf(stderr, "Height=%d, amount=%d, scrolling DOWN by %d lines\n", hei,amount, hei-amount);
    wnd.copytext(0,y1+amount, 0,y1, wnd.xsize,hei-amount);
    wnd.fillbox(0,y1, wnd.xsize,amount);
}

void termwindow::yscroll_up(unsigned y1, unsigned y2, int amount) const
{
    unsigned hei = y2-y1+1;
    if(unsigned(amount) > hei) amount = hei;
    fprintf(stderr, "Height=%d, amount=%d, scrolling UP by %d lines\n", hei,amount, hei-amount);
    wnd.copytext(0,y1, 0,y1+amount, wnd.xsize,hei-amount);
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
        st_csi_ex,      // csi !
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
    for(char32_t c: s)
        switch(State(c,state))
        {
            #define CsiState(c)      State(c,st_csi):     case State(c,st_csi_dec2): \
                                case State(c,st_csi_dec): case State(c,st_csi_dec3): \
                                case State(c,st_csi_ex)
            #define AnyState(c)      State(c,st_default): case State(c,st_esc): \
                                case State(c,st_scs0):    case State(c,st_scs1): \
                                case State(c,st_scr):     case State(c,st_esc_percent): \
                                case CsiState(c)

            // Note: These escapes are recognized even in the middle of an ANSI/VT code.
            case AnyState(U'\7'):  { lastch=c; BeepOn(); break; }
            case AnyState(U'\b'):  { lastch=c; ScrollFix(); if(cx>0) { --cx; } break; }
            case AnyState(U'\t'):  { lastch=c; ScrollFix(); cx += 8 - (cx & 7); cmov: FixCoord(); break; }
            case AnyState(U'\r'):  { lastch=c; cx=0; break; }
            case AnyState(U'\16'): { activeset = 1; translate = g1set; break; }
            case AnyState(U'\17'): { activeset = 0; translate = g0set; break; }
            case AnyState(U'\177'): { /* del - ignore */ break; }
            case AnyState(U'\30'): [[fallthrough]];
            case AnyState(U'\32'): { Ground: state=st_default; break; }
            case State(U'\33', st_default): state = st_esc; p.clear(); break;
            case State(U'(', st_esc): state = st_scs0; break; // esc (
            case State(U')', st_esc): state = st_scs1; break; // esc )
            case State(U'#', st_esc): state = st_scr; break;  // esc #
            case State(U'[', st_esc): state = st_csi; break;  // esc [
            case State(U'%', st_esc): state = st_esc_percent; break; // esc %
            case State(U'?', st_csi): state = st_csi_dec; break;  // csi ?
            case State(U'>', st_csi): state = st_csi_dec2; break; // csi >
            case State(U'=', st_csi): state = st_csi_dec3; break; // csi =
            case State(U'!', st_csi): state = st_csi_ex; break; // csi !

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

            case State(U'E', st_esc): // esc E = CR+LF
                cx = 0;
                state = st_default;
                [[fallthrough]];
            case AnyState(10):
            case AnyState(11):
            case AnyState(12):
                lastch = c;
                ScrollFix();
                Lf();
                break;

            case State(U'M', st_esc): // esc M, Ri (FIXME verify that this is right?)
                /* Within window: move cursor up; scroll the window down if at top */
                if(cy > top)
                    --cy;
                else
                    yscroll_down(top, bottom, 1);
                goto Ground;
            case State(U'c', st_esc): Reset(); break; // esc c
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
            case State(U'8', st_scr): // clear screen with 'E' // esc # 8
                wnd.blank.ch = U'E';
                wnd.fillbox(0,0, wnd.xsize,wnd.ysize);
                wnd.blank.ch = U' ';
                goto Ground;
            case State(U'@', st_esc_percent): utfmode = 0; goto Ground; // esc % @
            case State(U'G', st_esc_percent): [[fallthrough]];          // esc % G
            case State(U'8', st_esc_percent): utfmode = 1; goto Ground; // esc % 8
            case State(U'g', st_csi): /* TODO: set tab stops */ goto Ground;
            case State(U'q', st_csi): /* TODO: set leds */ goto Ground;
            case State(U'G', st_csi): [[fallthrough]];
            case State(U'`', st_csi): { GetParams(1,true); cx=p[0]-1; goto cmov; } // absolute hpos
            case State(U'd', st_csi): { GetParams(1,true); cy=p[0]-1; goto cmov; } // absolute vpos
            case State(U'F', st_csi): cx=0; [[fallthrough]];
            case State(U'A', st_csi): { GetParams(1,true); cy-=p[0];  goto cmov; }
            case State(U'E', st_csi): cx=0; [[fallthrough]];
            case State(U'e', st_csi): [[fallthrough]];
            case State(U'B', st_csi): { GetParams(1,true); cy+=p[0];  goto cmov; }
            case State(U'a', st_csi): [[fallthrough]];
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
                wnd.fillbox(cx,cy, p[0],1);
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
            case State(U'r', st_csi): [[fallthrough]]; // CSI r
            case State(U'p', st_csi_ex):               // CSI ! p
                GetParams(2,false);
                if(!p[0]) p[0]=1;
                if(!p[1]) p[1]=wnd.ysize;
                if(p[0] < p[1] && p[1] <= wnd.ysize)
                {
                    top = p[0]-1; bottom = p[1]-1;
                    fprintf(stderr, "Creating a window with top=%d, bottom=%d\n", top,bottom);
                    cx=0; cy=top;
                    goto cmov;
                }
                break;
            case State(U'n', st_csi):
                GetParams(1,false);
                switch(p[0])
                {
                    char Buf[32];
                    case 5: EchoBack(U"\33[0n"); break;
                    case 6: EchoBack(FromUTF8(std::string_view{Buf, (std::size_t)std::sprintf(Buf, "\33[%d;%dR", cy+1, cx+1)})); break;
                }
                break;
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
            case State(U'h', st_csi_dec): // csi ? h, misc modes on
            case State(U'l', st_csi_dec): // csi ? l, misc modes off
            {
                bool set = c == U'h';
                // 1=CKM, 2=ANM, 3=COLM, 4=SCLM, 5=SCNM, 6=OM, 7=AWM, 8=ARM,
                // 18=PFF, 19=PEX, 25=TCEM, 40=132COLS, 42=NRCM,
                // 44=MARGINBELL, ...
                //   6 puts cursor at (0,0) both set,clear
                //  25 enables/disables cursor visibility
                //  40 enables/disables 80/132 mode (note: if enabled, RESET changes to one of these)
                //   3 sets width at 132(enable), 80(disable) if "40" is enabled
                //   5 = screenwide reverse color
                GetParams(0, false);
                for(auto a: p)
                    switch(a)
                    {
                        case 6:  cx=cy=0; FixCoord(); break;
                        case 25: wnd.cursorvis = set; break;
                        case 5:  wnd.reverse   = set; break;
                    }
                break;
            }
            case State(U'h', st_csi): // csi h, ansi modes on
            case State(U'l', st_csi): // csi l, ansi modes off
            {
                //bool set = c == U'h';
                // 2 = keyboard locked
                // 4 = insert mode
                // 12 = local echo
                // 20 = auto linefeed
                goto Ground;
            }
            case State(U'b', st_csi):
                GetParams(1,true);
                // Repeat last printed character n times
                for(unsigned m = std::min(p[0], unsigned(wnd.xsize*wnd.ysize)), c=0; c<m; ++c)
                {
                    ScrollFix();
                    wnd.PutCh(cx,cy, lastch, translate);
                    ++cx;
                }
                break;
            case State(U'm', st_csi): // csi m (SGR)
            {
                GetParams(1, false); // Make sure there is at least 1 param
                c=0;
                auto mode = [](unsigned n)            constexpr { return 68 + n - 3; };           // room for 22
                auto flag = [](unsigned c,unsigned n) constexpr { return 10 + (n-2) + (c-1)*4; }; // room for 8
                for(auto a: p)
                    switch(c<3 ? ((c && (a>=2 && a<=5)) ? flag(c,a) : a) : mode(c))
                    {
                        case 0: ResetAttr(); c = 0; break;
                        case 1: wnd.blank.bold = true; c = 0; break;
                        case 2: wnd.blank.dim = true; c = 0; break;
                        case 3: wnd.blank.italic = true; c = 0; break;
                        case 4: wnd.blank.underline = true; c = 0; break;
                        case 5: wnd.blank.blink = true; c = 0; break;
                        case 7: wnd.blank.reverse = true; c = 0; break;
                        case 8: wnd.blank.conceal = true; c = 0; break;
                        case 9: wnd.blank.overstrike = true; c = 0; break;
                        case 20: wnd.blank.fraktur = true; c = 0; break;
                        case 21: wnd.blank.underline2 = true; c = 0; break;
                        case 22: wnd.blank.dim = false; wnd.blank.bold = false; c = 0; break;
                        case 23: wnd.blank.italic = false; wnd.blank.fraktur = false; c = 0; break;
                        case 24: wnd.blank.underline = false; wnd.blank.underline2 = false; c = 0; break;
                        case 25: wnd.blank.blink = false; c = 0; break;
                        case 27: wnd.blank.reverse = false; c = 0; break;
                        case 28: wnd.blank.conceal = false; c = 0; break;
                        case 29: wnd.blank.overstrike = false; c = 0; break;
                        case 39: wnd.blank.underline = false; wnd.blank.underline2 = false; ResetFG(); c = 0; break; // Set default foreground color
                        case 49: ResetBG(); c = 0; break; // Set default background color
                        case 51: wnd.blank.framed = true; c = 0; break;
                        case 52: wnd.blank.encircled = true; c = 0; break;
                        case 53: wnd.blank.overlined = true; c = 0; break;
                        case 54: wnd.blank.framed = false; wnd.blank.encircled = false; c = 0; break;
                        case 55: wnd.blank.overlined = false; c = 0; break;
                        case 38: c = 1; break;
                        case 48: c = 2; break;
                        case flag(1,4): c=3; color=0; break; // 38;4
                        case flag(1,3): c=4; color=0; break; // 38;3
                        case flag(1,2): c=5; color=0; break; // 38;2
                        case flag(2,4): c=6; color=0; break; // 48;4
                        case flag(2,3): c=7; color=0; break; // 48;3
                        case flag(2,2): c=8; color=0; break; // 48;2
                        case flag(1,5): c=22; break;         // 38;5
                        case flag(2,5): c=23; break;         // 48;5
                        case mode(3): //color = (color << 8) + a; c+=6; break; // 38;4;n
                        case mode(4): //color = (color << 8) + a; c+=6; break; // 38;3;n
                        case mode(5): //color = (color << 8) + a; c+=6; break; // 38;2;n
                        case mode(6): //color = (color << 8) + a; c+=6; break; // 48;4;n
                        case mode(7): //color = (color << 8) + a; c+=6; break; // 48;3;n
                        case mode(8): //color = (color << 8) + a; c+=6; break; // 48;2;n
                        case mode(9): //color = (color << 8) + a; c+=6; break; // 38;4;#;n
                        case mode(10)://color = (color << 8) + a; c+=6; break; // 38;3;#;n
                        case mode(11)://color = (color << 8) + a; c+=6; break; // 38;2;#;n
                        case mode(12)://color = (color << 8) + a; c+=6; break; // 48;4;#;n
                        case mode(13)://color = (color << 8) + a; c+=6; break; // 48;3;#;n
                        case mode(14)://color = (color << 8) + a; c+=6; break; // 48;2;#;n
                        case mode(15)://color = (color << 8) + a; c+=6; break; // 38;4;#;#;n
                        case mode(18):  color = (color << 8) + a; c+=6; break; // 48;4;#;#;n

                        case 30:case 31:case 32:case 33:case 34:case 35:
                        case 36:case 37:    a -= 30; a += 90-8;                 [[fallthrough]];
                        case 90:case 91:case 92:case 93:case 94:case 95:
                        case 96:case 97:    a -= 90-8;                          [[fallthrough]];
                        case mode(22):  color = 0; a = xterm256table[a & 0xFF]; [[fallthrough]];           // 38;5;n
                        case mode(21)://color = (color << 8) + a; wnd.blank.fgcolor = color; c = 0; break; // 38;4;#;#;#;n (TODO CMYK->RGB)
                        case mode(16)://color = (color << 8) + a; wnd.blank.fgcolor = color; c = 0; break; // 38;3;#;#;n   (TODO CMY->RGB)
                        case mode(17):  color = (color << 8) + a; wnd.blank.fgcolor = color; c = 0; break; // 38;2;#;#;n   (RGB24)

                        case 40:case 41:case 42:case 43:case 44:case 45:
                        case 46:case 47:    a -= 40; a += 100-8;                [[fallthrough]];
                        case 100:case 101:case 102:case 103:case 104:case 105:
                        case 106:case 107:  a -= 100-8;                         [[fallthrough]];
                        case mode(23):  color = 0; a = xterm256table[a & 0xFF]; [[fallthrough]];           // 48;5;n
                        case mode(24)://color = (color << 8) + a; wnd.blank.bgcolor = color; c = 0; break; // 48;4;#;#;#;n (TODO CMYK->RGB)
                        case mode(19)://color = (color << 8) + a; wnd.blank.bgcolor = color; c = 0; break; // 48;3;#;#;n   (TODO CMY->RGB)
                        case mode(20):  color = (color << 8) + a; wnd.blank.bgcolor = color; c = 0; break; // 48;2;#;#;n   (RGB24)

                        default: c = 0; break;
                    }
                break;
            }

            default:
                if(state != st_default) goto Ground;
                ScrollFix();
                lastch = c;
                wnd.PutCh(cx,cy, c, translate);
                ++cx;
                break;
        }
        #undef AnyState

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
    if(top>bottom-1)top=bottom-1;
    if(top<0)top=0;
    if(bottom<top+1)bottom=top+1;
    if(cx<0)cx=0;
    if(cy<0)cy=0;
    if(cx>=int(wnd.xsize))cx=wnd.xsize-1;
    if(cy>=bottom)cy=bottom;
}
void termwindow::ScrollFix()
{
    if(cx >= int(wnd.xsize)) { cx = 0; Lf(); }
    if(cy >= int(wnd.ysize)) cy = wnd.ysize-1;
}
