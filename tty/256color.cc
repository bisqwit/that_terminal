#include "256color.hh"
#include "color.hh"

constexpr unsigned Make16(unsigned r,unsigned g,unsigned b)
{
    return Repack( { r*255u/31u, g*255u/31u, b*255u/31u } );
}
inline constexpr unsigned char grayramp[24] = { 1,2,3,5,6,7,8,9,11,12,13,14,16,17,18,19,20,22,23,24,25,27,28,29 };
inline constexpr unsigned char colorramp[6] = { 0,12,16,21,26,31 };

std::array<unsigned,256> xterm256table = []() constexpr
{
    std::array<unsigned,256> result =
    {
        Make16(0,0,0), Make16(21,0,0), Make16(0,21,0), Make16(21,10,0),
        Make16(0,0,21), Make16(21,0,21), Make16(0,21,21), Make16(21,21,21),
        Make16(15,15,15), Make16(31,10,10), Make16(5,31,10), Make16(31,31,10),
        Make16(10,10,31), Make16(31,10,31), Make16(5,31,31), Make16(31,31,31)
    };
    for(unsigned n=0; n<216; ++n)
    {
        result[16+n] = Make16(colorramp[(n/36)%6], colorramp[(n/6)%6], colorramp[(n)%6]);
    }
    for(unsigned n=0; n<24; ++n)
    {
        result[232 + n] = Make16(grayramp[n],grayramp[n],grayramp[n]);
    }
    return result;
}();
