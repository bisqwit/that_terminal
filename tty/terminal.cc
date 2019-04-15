#include <array>
#include <cstdio> // sprintf

#include "terminal.hh"
#include "beeper.hh"
#include "ctype.hh"
#include "256color.hh"

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
    wnd.blank = Cell{};
    ResetFG();
    ResetBG();
}
void termwindow::Reset()
{
    top    = 0;
    bottom = wnd.ysize-1;

    gset = {0,0,0,0}; activeset = 0; scs = 0;
    utfmode = 0;
    lastch = U' ';

    state = 0;
    p.clear();
    edgeflag = false;

    wnd.cursx = 0;
    wnd.cursy = 0;
    wnd.reverse   = false;
    wnd.cursorvis = true;
    wnd.fillbox(0,0, wnd.xsize,wnd.ysize, wnd.blank); // Clear screen
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
        st_string,
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
    auto PutC = [&](char32_t c)
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
    };

    if(bottom >= wnd.ysize) bottom = wnd.ysize-1;
    if(top    > bottom)     top = bottom;

    for(char32_t c: s)
    {
        switch(State(c,state))
        {
            #define CsiState(c)      State(c,st_csi):     case State(c,st_csi_dec2): \
                                case State(c,st_csi_dec): case State(c,st_csi_dec3): \
                                case State(c,st_csi_ex)
            #define AnyState(c)      State(c,st_default): case State(c,st_esc): \
                                case State(c,st_scs): \
                                case State(c,st_string):  case State(c,st_csi_quo): \
                                case State(c,st_scr):     case State(c,st_esc_percent): \
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
            case AnyState(U'\u0099'): { Ground: state=st_default; break; }
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
            case State(U'!', st_csi): state = st_csi_ex; break;  // csi !
            case State(U'"', st_csi): state = st_csi_quo; break; // csi " (note: after numbers)
            case AnyState(U'\7'):
            {
                if(state == st_string) // Treat as ST (string termination)
                {
            case AnyState(U'\u009C'): [[fallthrough]];
            case State(U'\\', st_esc): // String termination
                    switch(scs)
                    {
                        case 4: // TODO: parse DCS
                        case 5: // TODO: parse OSC
                        default:
                            break;
                    }
                    // TODO: clear accumulated string
                    goto Ground;
                }
                lastch=c; BeepOn(); break;
            }
            case AnyState(U'\u0090'): [[fallthrough]];
            case State(U'P', st_esc): state = st_string; scs = 4; break; // DCS
            case AnyState(U'\u009D'): [[fallthrough]];
            case State(U']', st_esc): state = st_string; scs = 5; break; // OSC
            case AnyState(U'\u009E'): [[fallthrough]];
            case State(U'^', st_esc): state = st_string; scs = 6; break; // PM
            case AnyState(U'\u009F'): [[fallthrough]];
            case State(U'_', st_esc): state = st_string; scs = 7; break; // APC
            case AnyState(U'\u0098'): [[fallthrough]];
            case State(U'X', st_esc): state = st_string; scs = 8; break; // SOS

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
            case State(U'q', st_csi_quo): // TODO: if param0=1, do CASE_SPA; if 0 or 2, do CASE_EPA. Otherwise ignore.
            {
                GetParams(1,false);
                if(p[0]==1)
                {
            //case AnyState(U'\u0096'):
            case State(U'V', st_esc): // esc V = CASE_SPA: start protected area
                        ;
                }
                else if(p[0]==0 || p[0]==2)
                {
            //case AnyState(U'\u0097'):
            case State(U'W', st_esc): // esc W = CASE_EPA: end protected area
                        ;
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

            case State(U'c', st_esc): ResetAttr(); Reset(); goto Ground; // esc c
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
            case State(U'3', st_scr): wnd.LineSetHeightAttr(1); goto Ground; // DECDHL top
            case State(U'4', st_scr): wnd.LineSetHeightAttr(2); goto Ground; // DECDHL bottom
            case State(U'5', st_scr): wnd.LineSetWidthAttr(false); goto Ground; // DECSWL
            case State(U'6', st_scr): wnd.LineSetWidthAttr(true); goto Ground; // DECDWL
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
                GetParams(1,false);
                // 0: erase from cursor to end of line
                // 1: erase from start of line to cursor
                // 2: erase whole line
                switch(p[0])
                {
                    case 0: clreol: wnd.fillbox(wnd.cursx,wnd.cursy, wnd.xsize-wnd.cursx, 1); break;
                    case 1: clrbol: wnd.fillbox(0,        wnd.cursy, wnd.cursx+1,  1); break;
                    case 2: wnd.fillbox(0, wnd.cursy, wnd.xsize,    1); break;
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
            case State(U'X', st_csi):
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
            case State(U'r', st_csi): [[fallthrough]]; // CSI r
            case State(U'p', st_csi_ex):               // CSI ! p
                GetParams(2,false);
                if(!p[0]) p[0]=1;
                if(!p[1]) p[1]=wnd.ysize;
                if(p[0] < p[1] && p[1] <= wnd.ysize)
                {
                    top = p[0]-1; bottom = p[1]-1;
                    fprintf(stderr, "Creating a window with top=%zu, bottom=%zu\n", top,bottom);
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
                // Example response: ^[P!|0^[ (backslash) 
                if(!p[0]) EchoBack(U"\33P!|00000000\x9C");
                break;
            case State(U'c', st_csi_dec2): // csi > 0 c, Secondary device attributes (terminal)
                GetParams(1,false);
                // Example response: ^[[>41;330;0c  (middle=firmware version)
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
                        case 6:  ClampedMove(0, top, false); break;
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
                    PutC(lastch);
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
                        case 5: wnd.blank.blink = 1; c = 0; break;
                        case 6: wnd.blank.blink = 2; c = 0; break;
                        case 7: wnd.blank.reverse = true; c = 0; break;
                        case 8: wnd.blank.conceal = true; c = 0; break;
                        case 9: wnd.blank.overstrike = true; c = 0; break;
                        case 20: wnd.blank.fraktur = true; c = 0; break;
                        case 21: wnd.blank.underline2 = true; c = 0; break;
                        case 22: wnd.blank.dim = false; wnd.blank.bold = false; c = 0; break;
                        case 23: wnd.blank.italic = false; wnd.blank.fraktur = false; c = 0; break;
                        case 24: wnd.blank.underline = false; wnd.blank.underline2 = false; c = 0; break;
                        case 25: wnd.blank.blink = 0; c = 0; break;
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
                if(state == st_string)
                {
                    // TODO: accumulate string
                    break;
                }
                if(state != st_default) goto Ground;
                PutC(lastch = c);
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
