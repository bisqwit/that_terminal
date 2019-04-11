#include <sstream>

#include "terminal.hh"
#include "beeper.hh"

#define Make16(r,g,b) (((((unsigned(b))&0x1F)*255/31)) \
                     | ((((unsigned(g)<<1)&0x3F)*255/63)<<8) \
                     | ((((unsigned(r))&0x1F)*255/31)<<16))
static unsigned xterm256table[256] =
    { Make16(0,0,0), Make16(21,0,0), Make16(0,21,0), Make16(21,5,0),
      Make16(0,0,21), Make16(21,0,21), Make16(0,21,21), Make16(21,21,21),
      Make16(7,7,7), Make16(31,5,5), Make16(5,31,5), Make16(31,31,5),
      Make16(5,5,31), Make16(31,5,31), Make16(5,31,31), Make16(31,31,31) };
static struct xterm256init { xterm256init() {
    static const unsigned char grayramp[24] = { 1,2,3,5,6,7,8,9,11,12,13,14,16,17,18,19,20,22,23,24,25,27,28,29 };
    static const unsigned char colorramp[6] = { 0,12,16,21,26,31 };
    for(unsigned n=0; n<216; ++n) { xterm256table[16+n] = Make16(colorramp[(n/36)%6], colorramp[(n/6)%6], colorramp[(n)%6]); }
    for(unsigned n=0; n<24; ++n)  { xterm256table[232 + n] = Make16(grayramp[n],grayramp[n],grayramp[n]); }
} } xterm256initializer;

static unsigned Translate16Color(unsigned c)
{
    fprintf(stderr, "16-color for %u is 0x%06X\n", c, xterm256table[c]);
    return xterm256table[c];
}
static unsigned Translate256Color(unsigned c)
{
    return xterm256table[c];
}

void termwindow::ResetAttr()
{
    intensity = italic = underline = blink = reverse = bold = 0;
    bgc = Translate16Color(0);
    fgc = Translate16Color(7);
}
void termwindow::Reset()
{
    cx = cy = top = 0;
    bottom = wnd.ysize-1;

    g0set = 0; g1set = 1; activeset = 0; translate = g0set;
    utfmode = 0; utflength = 0; utfvalue = 0;
    state = ESnormal;
    ques = 0;
    csi_J(2);
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

void termwindow::Ri()
{
    if(cy <= top)
    {
        /* scroll the window down */
        yscroll_down(top, bottom, 1);
    }
    else
    {
        --cy;
    }
}

void termwindow::yscroll_down(unsigned y1, unsigned y2, int amount) const
{
    unsigned hei = y2-y1+1;
    if(amount > hei) amount = hei;
    wnd.copytext(0,y1+amount, 0,y1, wnd.xsize,(y2-(y1+amount))+1);
    wnd.fillbox(0,y1, wnd.xsize,amount);
}

void termwindow::yscroll_up(unsigned y1, unsigned y2, int amount) const
{
    unsigned hei = y2-y1+1;
    if(amount > hei) amount = hei;
    wnd.copytext(0,y1, 0,y1+amount, wnd.xsize,(y2-amount-y1)+1);
    wnd.fillbox(0,y2-amount+1, wnd.xsize,amount);
}

void termwindow::csi_at(unsigned c) const
{
    // insert c spaces at cursor
    if(cx + c > wnd.xsize) c = wnd.xsize-cx;
    if(c == 0) return;
    unsigned remain = wnd.xsize - (cx+c);
    wnd.copytext(cx+c,cy, cx,cy, remain,1);
    wnd.fillbox(cx,cy, c,1);
}

void termwindow::csi_X(unsigned c) const
{
    // write c spaces at cursor (overwrite)
    wnd.fillbox(cx,cy+top, c,1);
}

void termwindow::csi_P(unsigned c) const
{
    // insert c black holes at cursor (eat c characters
    // and scroll line horizontally to left)
    if(cx + c > wnd.xsize) c = wnd.xsize-cx;
    if(c == 0) return;
    unsigned remain = wnd.xsize - (cx+c);
    wnd.copytext(cx,cy, wnd.xsize-remain,cy, remain,1);
    wnd.fillbox(wnd.xsize-c,cy, c,1);
}

void termwindow::csi_J(unsigned c) const
{
    switch(c)
    {
        case 0: // erase from cursor to end of display
            csi_K(0);
            if(cy < wnd.ysize-1)
                wnd.fillbox(0,cy+1, wnd.xsize, wnd.ysize-cy-1);
            break;
        case 1: // erase from start to cursor
            if(cy > 0) wnd.fillbox(0,0, wnd.xsize,cy);
            csi_K(1);
            break;
        case 2: // erase whole display
            wnd.fillbox(0,0, wnd.xsize,wnd.ysize);
            break;
    }
}

void termwindow::csi_K(unsigned c) const
{
    // 0: erase from cursor to end of line
    // 1: erase from start of line to cursor
    // 2: erase whole line
    switch(c)
    {
        case 0: wnd.fillbox(cx,cy, wnd.xsize-cx, 1); break;
        case 1: wnd.fillbox(0, cy, cx+1,       1); break;
        case 2: wnd.fillbox(0, cy, wnd.xsize,    1); break;
    }
}


void termwindow::Write(std::u32string_view s)
{
    // This is an indirect ripoff from
    // /usr/src/linux/drivers/char/console.c
    unsigned a, b=s.size();
    for(a=0; a<b; ++a)
    {
        char32_t c = s[a];
        switch(c)
        {
            case 7: BeepOn(); goto handled;
            case 8: ScrollFix(); if(cx>0) { --cx; } goto handled;
            case 9: ScrollFix(); cx += 8 - (cx & 7); FixCoord(); goto handled;
            case 10: case 11: case 12: Linefeed: ScrollFix();
            {
                if(cy == bottom)
                {
                    /* If the pending buffer has a few more linefeeds, do them
                     * at once to minimize the number of whole-screen scrolling
                     * operations done
                     */
                    int pending_linefeeds = 1;
                    for(unsigned c=a+1; c<b; ++c)
                        if(s[c] == 10 || s[c] == 11 || s[c] == 12)
                        { CalcLF:
                            pending_linefeeds += 1;
                            if(pending_linefeeds >= bottom-top+1) break;
                        }
                        else if(s[c] == U'\033' /*|| s[a] == U'\245'*/)
                        {
                            // If it's a color escape, ok; otherwise break
                            if(++c >= b) break;
                            if(s[c] == U'D' || s[c] == U'E') goto CalcLF;
                            if(s[c++] != U'[') break;
                            if(c < b && s[c] == U'?') ++c;
                            while(c < b && ((s[c]>=U'0' && s[c]<=U'9') || s[c]==U';')) ++c;
                            if(c >= b) break;

                            if(s[c] != U'm' // color setting
                            && s[c] != U'C' // horizontal cursor positioning
                            && s[c] != U'D' // horizontal cursor positioning
                            && s[c] != U'K' // horizontal line clearing
                            && s[c] != U'G' // horizontal cursor positioning
                              ) break;
                        }
                        else {}
                    /* Note: ircII or GNU screen seems to use
                     * a \33[H\33[47B sequence to reposition
                     * the cursor right before each linefeed.
                     * Without changing the sequence, it cannot
                     * be optimized like regular LFs.
                     */
                    yscroll_up(top, bottom, pending_linefeeds);
                    cy -= pending_linefeeds-1;
                    if(cy < top) cy = top;
                    goto handled;
                }
                Lf();
                goto handled;
            }
            case 13: cx=0; goto handled;
            case 14: activeset = 1; translate = g1set; goto handled;
            case 15: activeset = 0; translate = g0set; goto handled;
            case 24: case 26: state = ESnormal; goto handled;
            case 27: state = ESescnext; break;
            case 127: /* del - ignore */ goto handled;
            case 128+27: state = ESesc; break;
        }
        switch(state)
        {
            case ESescnext:
                state = ESesc;
                break;
            case ESnormal:
                // TODO: process utfmode
                ScrollFix();
                wnd.PutCh(cx,cy, c, translate);
                ++cx;
                break;
            case ESesc:
                state = ESnormal;
                switch(c)
                {
                    case U'[': state = ESsquare; break;
                    //case U']': state = ESnonstd; break;
                    case U'%': state = ESpercent; break;
                    case U'E': cx = 0; goto Linefeed;
                    case U'M': Ri(); break;
                    case U'D': goto Linefeed;
                    case U'Z': EchoBack(U"\33[?6c"); break;
                    case U'(': state = ESsetG0; break;
                    case U')': state = ESsetG1; break;
                    case U'#': state = EShash; break;
                    case U'7': save_cur(); break;
                    case U'8': restore_cur(); break;
                    case U'c': Reset(); break;
                }
                break;
            case ESsquare:
                par.clear(); par.push_back(0);
                state = ESgetpars;
                if(c==U'[') { state = ESignore; break; }
                if(c==U'?') { ques=1; break; }
                ques = 0;
                /* fallthru */
            case ESgetpars:
                if(c==U';') { par.push_back(0); break; }
                // TODO: Support U':' as separator
                if(c>=U'0' && c<=U'9') { par.back() = par.back()*10 + c-U'0'; break; }
                state = ESgotpars;
                [[fallthrough]];
            case ESgotpars:
                state = ESnormal;
                if(ques && (c==U'c' /* cursor type */
                         || c==U'm' /* complement mask */))
                {
                    /* UNIMPLEMENTED */
                    break;
                }

                switch(c)
                {
                    case U'h':
                        /* misc modes on */
                        break;
                    case U'l':
                        /* misc modes off */
                        /* esc[?3;4l = monitor off, insert mode off */
                        /* esc>      = numeric keypad off */
                        /* UNIMPLEMENTED */
                        break;
                    case U'n':
                        if(ques) break;
                        if(par[0]==5) EchoBack(U"\033[0n");
                        else if(par[0]==6)
                        {
                            std::basic_ostringstream<char32_t> esc;
                            esc << U"\33[" << (cy+1) << U';' << (cx+1) << U'R';
                            EchoBack(std::move(esc.str()));
                        }
                        break;
                    case U'G': case U'`': if(par[0]) --par[0]; cx=par[0]; goto cmov;
                    case U'd':           if(par[0]) --par[0]; cy=par[0]; goto cmov;
                    case U'F': cx=0; [[fallthrough]];
                    case U'A': if(!par[0]) par[0]=1; cy-=par[0]; goto cmov;
                    case U'E': cx=0; [[fallthrough]];
                    case U'B': if(!par[0]) par[0]=1; cy+=par[0]; goto cmov;
                    case U'C': if(!par[0]) par[0]=1; cx+=par[0]; goto cmov;
                    case U'D': if(!par[0]) par[0]=1; cx-=par[0];
                              cmov: FixCoord(); break;
                    case U'H': case U'f': par.resize(2);
                        if(par[0]) --par[0];
                        if(par[1]) --par[1];
                        cx=par[1]; cy=par[0];
                        break;
                    case U'J': csi_J(par[0]); break;
                    case U'K': csi_K(par[0]); break;
                    case U'L': csi_L(par[0]); break;
                    case U'M': csi_M(par[0]); break;
                    case U'P': csi_P(par[0]); break;
                    case U'X': csi_X(par[0]); break;
                    case U'@': csi_at(par[0]); break;
                    case U'c': if(!par[0]) EchoBack(U"\33[?6c"); break;
                    case U'g': /* set tab stops UNIMPLEMENTED */ break;
                    case U'q': /* set leds UNIMPLEMENTED */ break;
                    case U'm':
                    {
                        int mode256 = 0;
                        for(unsigned a=0; a<par.size(); ++a)
                            switch(par[a])
                            {
                                case 0: mode256 = 0; ResetAttr(); break;
                                case 1: bold = 1; break;
                                case 2:
                                    if(!mode256) intensity = -1;
                                    // Note: 24-bit color support.
                                    else if(mode256==1)
                                    {
                                        //TODO: Range checking
                                        fgc = (par[a+1]<<16)+(par[a+2]<<8)+par[a+3]; a+=3;
                                    }
                                    else if(mode256==2)
                                    {
                                        //TODO: Range checking
                                        bgc = (par[a+1]<<16)+(par[a+2]<<8)+par[a+3]; a+=3;
                                    }
                                    break;
                                case 3: italic = 1;
                                    // TODO: In mode256, parse CMY color
                                    break;
                                case 4: underline = 1;
                                    // TODO: In mode256, parse CMYK color
                                    break;
                                case 5:
                                    if(!mode256) blink=1;
                                    else if(mode256==1)
                                    {
                                        fgc = Translate256Color(par[++a]);
                                    }
                                    else if(mode256==2)
                                    {
                                        bgc = Translate256Color(par[++a]);
                                    }
                                    break;
                                case 7: reverse = 1; break;
                                case 21: underline = 2; break;
                                case 22: intensity = 0; bold = 0; break;
                                case 23: italic = 0; break;
                                case 24: underline = 0; break;
                                case 25: blink = 0; break;
                                case 27: reverse = 0; break;
                                case 38: mode256 = 1; break;
                                case 39: underline = 0; fgc = Translate16Color(7); break;
                                case 48: mode256 = 2; break;
                                case 49: bgc = Translate16Color(0); break;
                                default:
                                    /**/ if(par[a]>=30 && par[a]<=37)  fgc = Translate16Color(  (par[a]-30));
                                    else if(par[a]>=40 && par[a]<=47)  bgc = Translate16Color(  (par[a]-40));
                                    /**/ if(par[a]>=90 && par[a]<=97)  fgc = Translate16Color(8|(par[a]-90));
                                    else if(par[a]>=100&& par[a]<=107) bgc = Translate16Color(8|(par[a]-100));
                            }
                        BuildAttr();
                        break;
                    }
                    case U'r': par.resize(2);
                        if(!par[0]) par[0]=1;
                        if(!par[1]) par[1]=wnd.ysize;
                        if(par[0] < par[1] && par[1] <= wnd.ysize)
                        {
                            top=par[0]-1, bottom=par[1]-1;
                            cx=0; cy=top;
                        }
                        break;
                    case U's': save_cur(); break;
                    case U'u': restore_cur(); break;
                }
                break;
            case EShash:
                state = ESnormal;
                if(c == U'8') { /* clear screen with 'E' */ break; }
                break;
            case ESignore:
                state = ESnormal;
                break;
            case ESsetG0:
                if(c == U'0') g0set = 1;
                else if(c == U'B') g0set = 0;
                else if(c == U'U') g0set = 2;
                else if(c == U'K') g0set = 3;
                if(activeset == 0) translate = g0set;
                state = ESnormal;
                break;
            case ESsetG1:
                if(c == U'0') g1set = 1;
                else if(c == U'B') g1set = 0;
                else if(c == U'U') g1set = 2;
                else if(c == U'K') g1set = 3;
                if(activeset == 1) translate = g1set;
                state = ESnormal;
                break;
            case ESpercent:
                state = ESnormal;
                if(c == U'@') utfmode = 0;
                else if(c == U'G' || c == U'8') utfmode = 1;
                break;
        }
     handled:;
    }

    if((cx+1 != wnd.xsize
     || cy+1 != wnd.ysize)
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
    backup.i = intensity;
    backup.I = italic;
    backup.u = underline;
    backup.b = blink;
    backup.r = reverse;
    backup.B = bold;
    backup.f = fgc;
    backup.g = bgc;
    backup.top = top;
    backup.bottom = bottom;
}

void termwindow::restore_cur()
{
    cx = backup.cx;
    cy = backup.cy;
    intensity = backup.i;
    italic    = backup.I;
    underline = backup.u;
    blink = backup.b;
    reverse = backup.r;
    bold = backup.B;
    fgc = backup.f;
    bgc = backup.g;
    top = backup.top;
    bottom = backup.bottom;
}

void termwindow::FixCoord()
{
    if(bottom>=wnd.ysize)bottom=wnd.ysize-1;
    if(top>bottom-2)top=bottom-2;
    if(top<0)top=0;
    if(bottom<top+2)bottom=top+2;
    if(cx<0)cx=0;
    if(cy<0)cy=0;
    if(cx>=wnd.xsize)cx=wnd.xsize-1;
    if(cy>=bottom)cy=bottom;
}
void termwindow::ScrollFix()
{
    if(cx >= wnd.xsize) { cx = 0; Lf(); }
}

void termwindow::BuildAttr()
{
    wnd.blank.fgcolor = fgc;
    wnd.blank.bgcolor = bgc;
    wnd.blank.intense = intensity > 0;
    wnd.blank.bold    = bold;
    wnd.blank.dim     = intensity < 0;
    wnd.blank.italic  = italic;
    wnd.blank.underline  = underline==1;
    wnd.blank.underline2 = underline==2;
    wnd.blank.overstrike = 0;// FIXME
    wnd.blank.reverse = reverse;
}
