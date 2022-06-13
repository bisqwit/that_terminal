#ifdef RUN_TESTS
# include <gtest/gtest.h>
#endif
/** @file keysym.cc
 * @brief Defines InterpretInput()
 */

#include <unordered_map>
#include <cstdio> // sprintf

#include "keysym.hh"
#include "ctype.hh"

/** Define mappings between SDL keys and ESC[ sequences */
static const std::unordered_map<int, std::pair<int/*num*/,char/*letter*/>> lore
{
    { SDLK_F1,       {1,'P'} },  { SDLK_LEFT,     {1,'D'} },
    { SDLK_F2,       {1,'Q'} },  { SDLK_RIGHT,    {1,'C'} },
    { SDLK_F3,       {1,'R'} },  { SDLK_UP,       {1,'A'} },
    { SDLK_F4,       {1,'S'} },  { SDLK_DOWN,     {1,'B'} },
    { SDLK_F5,       {15,'~'} }, { SDLK_HOME,     {1,'H'} },
    { SDLK_F6,       {17,'~'} }, { SDLK_END,      {1,'F'} },
    { SDLK_F7,       {18,'~'} }, { SDLK_INSERT,   {2,'~'} },
    { SDLK_F8,       {19,'~'} }, { SDLK_DELETE,   {3,'~'} },
    { SDLK_F9,       {20,'~'} }, { SDLK_PAGEUP,   {5,'~'} },
    { SDLK_F10,      {21,'~'} }, { SDLK_PAGEDOWN, {6,'~'} },
    { SDLK_F11,      {23,'~'} },
    { SDLK_F12,      {24,'~'} },
};
/** Mappings between SDL keys and ASCII sequences */
static const std::unordered_map<int, char> lore2
{
    { SDLK_a, 'a' }, { SDLK_b, 'b' }, { SDLK_c, 'c' }, { SDLK_d, 'd' },
    { SDLK_e, 'e' }, { SDLK_f, 'f' }, { SDLK_g, 'g' }, { SDLK_h, 'h' },
    { SDLK_i, 'i' }, { SDLK_j, 'j' }, { SDLK_k, 'k' }, { SDLK_l, 'l' },
    { SDLK_m, 'm' }, { SDLK_n, 'n' }, { SDLK_o, 'o' }, { SDLK_p, 'p' },
    { SDLK_q, 'q' }, { SDLK_r, 'r' }, { SDLK_s, 's' }, { SDLK_t, 't' },
    { SDLK_u, 'u' }, { SDLK_v, 'v' }, { SDLK_w, 'w' }, { SDLK_x, 'x' },
    { SDLK_y, 'y' }, { SDLK_z, 'z' }, { SDLK_ESCAPE, '\33' },
    { SDLK_0, '0' }, { SDLK_1, '1' }, { SDLK_2, '2' }, { SDLK_3, '3' },
    { SDLK_4, '4' }, { SDLK_5, '5' }, { SDLK_6, '6' }, { SDLK_7, '7' },
    { SDLK_8, '8' }, { SDLK_9, '9' }, { SDLK_PERIOD, '.' },
    { SDLK_COMMA, ',' }, { SDLK_SLASH, '-' },
    { SDLK_RETURN, '\r' }, { SDLK_BACKSPACE, '\177' }, { SDLK_TAB, '\t' },
    { SDLK_SPACE, ' ' }, { SDLK_KP_ENTER, '\r' },
};

std::string InterpretInput(bool shift, bool alt, bool ctrl, SDL_Keycode sym)
{
    if(auto i = lore.find(sym); i != lore.end())
    {
        const auto& d = i->second;
        unsigned delta = 1 + shift*1 + alt*2 + ctrl*4, len;
        char bracket = '[', Buf[16];
        if(d.second >= 'P' && d.second <= 'S') bracket = 'O';
        if(d.second >= 'A' && d.second <= 'D' && delta == 1) bracket = 'O'; // less requires this for up&down, alsamixer requires this for left&right
        if(delta != 1)
            len = std::sprintf(Buf, "\33%c%d;%d%c", bracket, d.first, delta, d.second);
        else if(d.first == 1)
            len = std::sprintf(Buf, "\33%c%c", bracket, d.second);
        else
            len = std::sprintf(Buf, "\33%c%d%c", bracket, d.first, d.second);
        return std::string(Buf,len);
    }

    if(auto i = lore2.find(sym); i != lore2.end())
    {
        char32_t cval = i->second;
        bool digit = cval >= '0' && cval <= '9', alpha = cval >= 'a' && cval <= 'z';
        if(shift && alpha) cval &= ~0x20; // Turn uppercase
        if(ctrl && digit) cval = "01\0\33\34\35\36\37\1779"[cval-'0'];
        if(ctrl && i->second=='\177') cval = '\b';
        else if(ctrl && !digit) cval &= 0x1F; // Turn into a control character
        // CTRL a..z becomes 01..1A
        // CTRL 0..9 becomes 10..19, should become xx,xx,00,1B-1F,7F,xx
        if(alt) cval |= 0x80;  // Add ALT
        if((!alpha && !digit) || ctrl||alt)
        {
            //std::fprintf(stderr, "lore input(%c)(%d) with keysym=%d\n", char(cval), int(cval), sym);
            if(shift && cval == '\t')
                return std::string("\33[Z", 3);
            else
                return ToUTF8(std::u32string_view(&cval,1));
        }
    }

    return {};
}


#ifdef RUN_TESTS
TEST(keysym, control_keys)
{
    EXPECT_EQ(InterpretInput(false,false,false, SDLK_F1), "\033OP");
    EXPECT_EQ(InterpretInput(true, false,false, SDLK_F1), "\033O1;2P");
    EXPECT_EQ(InterpretInput(false,false, true, SDLK_F1), "\033O1;5P");
    EXPECT_EQ(InterpretInput(true, false, true, SDLK_F1), "\033O1;6P");
    EXPECT_EQ(InterpretInput(true,  true, true, SDLK_F1), "\033O1;8P");
    EXPECT_EQ(InterpretInput(false,false,false, SDLK_INSERT),   "\033[2~");
    EXPECT_EQ(InterpretInput(false,false,false, SDLK_DELETE),   "\033[3~");
    EXPECT_EQ(InterpretInput(false,false,false, SDLK_PAGEUP),   "\033[5~");
    EXPECT_EQ(InterpretInput(false,false,false, SDLK_PAGEDOWN), "\033[6~");
}
TEST(keysym, regular_keys)
{
    // Alphabetic input is disabled in order to not conflict with SDL TextInput
    EXPECT_EQ(InterpretInput(false,false,false, SDLK_a), "");
    EXPECT_EQ(InterpretInput(true, false,false, SDLK_a), "");
    EXPECT_EQ(InterpretInput(false,false, true, SDLK_c), "\3"); // ctrl-c
    EXPECT_EQ(InterpretInput(false,false,false, SDLK_TAB),    "\t");
    EXPECT_EQ(InterpretInput(true,false,false, SDLK_TAB),     "\033[Z");
    EXPECT_EQ(InterpretInput(false,false,false, SDLK_RETURN), "\r");
    EXPECT_EQ(InterpretInput(false,false,false, SDLK_SPACE),  " ");
}
#endif
