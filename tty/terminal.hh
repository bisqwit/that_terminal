#ifndef bqtTTYterminalHH
#define bqtTTYterminalHH

#include <deque>
#include <string>
#include <vector>

#include "screen.hh"

class termwindow
{
private:
    Window& wnd;
public:
    std::deque<char32_t> OutBuffer;
    int cx, cy;

private:
    int top, bottom;
    signed char intensity, underline;
    bool italic, blink, reverse, bold, overstrike;
    unsigned fgc, bgc;

    struct backup
    {
        signed char cx,cy, i,u;
        int top,bottom, f,g;
        bool I,b,r,B,o;
    } backup;

    char g0set, g1set, activeset, utfmode, translate;
    unsigned utflength;
    unsigned long utfvalue;

    enum { ESnormal, ESesc, ESsquare, ESgetpars, ESgotpars,
           EShash, ESignore, ESescnext, ESnonstd, ESsetG0,
           ESsetG1, ESpercent } state;
    std::vector<int> par;
    char extramark;

private:
    void ResetFG();
    void ResetBG();
    void ResetAttr();
    void BuildAttr();
    void Reset();

    void FixCoord();
    void ScrollFix();
    void yscroll_down(unsigned y1, unsigned y2, int amount) const;
    void yscroll_up(unsigned y1, unsigned y2, int amount) const;

    void csi_at(unsigned c) const;
    inline void csi_X(unsigned c) const;
    inline void csi_P(unsigned c) const;
    inline void csi_J(unsigned c) const;
    void csi_K(unsigned c) const;
    void csi_L(int c) const
    {
        // scroll the rest of window c lines down,
        // including where cursor is. Don't move cursor.
        yscroll_down(cy, bottom, c);
    }
    inline void csi_M(int c) const
    {
        yscroll_up(cy, bottom, c);
    }
    void Lf();
    void Ri();
    void save_cur();
    void restore_cur();
public:
    void Write(std::u32string_view s);

    termwindow(Window& w): wnd(w)
    {
        Reset();
        save_cur();
    }
    void EchoBack(std::u32string_view buffer);
};

#endif
