#ifdef RUN_TESTS
#include <gtest/gtest.h>
#include <algorithm>
#endif
/**@file tty/256color.cc
 * @brief Defines xterm256table.
 */

#include "256color.hh"
#include "color.hh"

/** Converts 5-bit colors into 8-bit colors. */
constexpr unsigned Make16(unsigned r,unsigned g,unsigned b)
{
    return Repack( { r*255u/31u, g*255u/31u, b*255u/31u } );
}
inline constexpr unsigned char grayramp[24] = { 1,2,3,5,6,7,8,9,11,12,13,14,16,17,18,19,20,22,23,24,25,27,28,29 };
inline constexpr unsigned char colorramp[6] = { 0,12,16,21,26,31 };

/** Builds and defines the table of 256 colors. 8-bit index, 24-bit value. */
const constinit std::array<unsigned,256> xterm256table = []() consteval
{
    std::array<unsigned,256> result =
    {
        Make16(0,0,0), Make16(21,0,0), Make16(0,21,0), Make16(21,10,0),
        Make16(0,0,21), Make16(21,0,21), Make16(0,21,21), Make16(21,21,21),
        Make16(15,15,15), Make16(31,10,10), Make16(5,31,10), Make16(31,31,10),
        Make16(10,10,31), Make16(31,10,31), Make16(5,31,31), Make16(31,31,31)
    };
    for(unsigned n=0; n<216; ++n) //LCOV_EXCL_BR_LINE
    {
        result[16+n] = Make16(colorramp[(n/36)%6], colorramp[(n/6)%6], colorramp[(n)%6]);
    }
    for(unsigned n=0; n<24; ++n) //LCOV_EXCL_BR_LINE
    {
        result[232 + n] = Make16(grayramp[n],grayramp[n],grayramp[n]);
    }
    return result;
}();


#ifdef RUN_TESTS
TEST(xterm256color, contains_two_blacks)
{
    volatile unsigned r = 0;
    Make16(r,1,2);
    EXPECT_EQ(2, std::count(xterm256table.begin(), xterm256table.end(), 0x000000));
}
TEST(xterm256color, contains_two_whites)
{
    volatile unsigned r = 3;
    EXPECT_EQ(2, std::count(xterm256table.begin(), xterm256table.end(), 0xFFFFFF));
    // Dummy function calls for coverage reasons (gcov counts inline functions):
    Mix(Repack(Unpack(r)),cmyk2rgb(r),r,r,cmy2rgb(r));
}
#endif
