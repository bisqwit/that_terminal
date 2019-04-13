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

private:
    std::size_t top, bottom;

    struct backup
    {
        std::size_t top,bottom;
        std::size_t cx,cy;
        Cell attr;
    } backup;

    char g0set, g1set, activeset, utfmode, translate;
    char32_t lastch = U' ';
    unsigned utflength;
    unsigned long utfvalue;

    std::vector<unsigned> p;
    unsigned              state=0;

private:
    std::u32string buf{};
    std::size_t    fill_req=0;

private:
    void ResetFG();
    void ResetBG();
    void ResetAttr();
    void Reset();

    void FixCoord();
    void yscroll_down(unsigned y1, unsigned y2, int amount) const;
    void yscroll_up(unsigned y1, unsigned y2, int amount) const;

    void save_cur();
    void restore_cur();
public:
    void Write(std::u32string_view s);

    termwindow(Window& w): wnd(w)
    {
        Reset();
        ResetAttr();
        save_cur();
    }
    void EchoBack(std::u32string_view buffer);
};

#endif
