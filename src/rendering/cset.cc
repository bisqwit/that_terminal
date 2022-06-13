#ifdef RUN_TESTS
# include <gtest/gtest.h>
#endif
/** @file rendering/cset.cc
 * @brief Defines TranslateCSet()
 */

#include "cset.hh"

char32_t TranslateCSet(char32_t c, int cset)
{
    switch(cset)
    {
        case 0:
        default:
            return c;
        case 1: // DEC Graphics
        {
            static const unsigned short table[32] =
            {
                0x25ae, /* black vertical rectangle           */
                0x25c6, /* black diamond                      */
                0x2592, /* medium shade                       */
                0x2409, /* symbol for horizontal tabulation   */
                0x240c, /* symbol for form feed               */
                0x240d, /* symbol for carriage return         */
                0x240a, /* symbol for line feed               */
                0x00b0, /* degree sign                        */
                0x00b1, /* plus-minus sign                    */
                0x2424, /* symbol for newline                 */
                0x240b, /* symbol for vertical tabulation     */
                0x2518, /* box drawings light up and left     */
                0x2510, /* box drawings light down and left   */
                0x250c, /* box drawings light down and right  */
                0x2514, /* box drawings light up and right    */
                0x253c, /* box drawings light vertical and horizontal */
                0x23ba, /* box drawings scan 1                */
                0x23bb, /* box drawings scan 3                */
                0x2500, /* box drawings light horizontal      */
                0x23bc, /* box drawings scan 7                */
                0x23bd, /* box drawings scan 9                */
                0x251c, /* box drawings light vertical and right      */
                0x2524, /* box drawings light vertical and left       */
                0x2534, /* box drawings light up and horizontal       */
                0x252c, /* box drawings light down and horizontal     */
                0x2502, /* box drawings light vertical        */
                0x2264, /* less-than or equal to              */
                0x2265, /* greater-than or equal to           */
                0x03c0, /* greek small letter pi              */
                0x2260, /* not equal to                       */
                0x00a3, /* pound sign                         */
                0x00b7, /* middle dot                         */
            };
            return (c >= 0x5F && c <= 0x7E) ? table[c - 0x5F] : c;
        }
    }
}

#ifdef RUN_TESTS
TEST(TranslateCSet, zero_ok)
{
    for(unsigned a=0; a<512; ++a) EXPECT_EQ(TranslateCSet(a,0), a);
}
TEST(TranslateCSet, one_ok)
{
    for(unsigned a=0; a<0x5F; ++a) EXPECT_EQ(TranslateCSet(a,1), a);
    for(unsigned a=0x7F; a<512; ++a) EXPECT_EQ(TranslateCSet(a,1), a);
    for(unsigned a=0x5F; a<0x7E; ++a) EXPECT_NE(TranslateCSet(a,1), a);
    EXPECT_EQ(TranslateCSet(0x61, 1), 0x2592);
}
#endif
