#include <string>
#include <cstdlib>
#include <cstring>
#include <utility>
#include <vector>
#include <algorithm>

#include "ctype.hh"
#include "color.hh"

unsigned ParseColorName(std::u32string_view s)
{
    return ParseColorName(ToUTF8(s));
}

/*
Generated with:

grep -v '!' /usr/share/X11/rgb.txt \
| perl -pe "s/(\d+)\s+(\d+)\s+(\d+)\s+(.*)/ {\"\\4\", Repack({\\1,\\2,\\3}) },/" \
| tr A-Z a-z

*/

static constexpr unsigned mod = 4088;
// Note: Not using std::string_view, because clang falsely reports it's not a built-in-constant when it is.
static constexpr unsigned hash(const char* s, std::size_t length)
{
    constexpr std::uint_least64_t lett = 0x42EA2DF6C524E; // maps a..z into 0..3 
    constexpr std::uint_least64_t map = 0x86240793516208; // maps everything into 0..9
    std::uint_least64_t result = 0x2306F37DE41BF7F4;
    for(std::size_t p=0; p<length; ++p)
    {
        unsigned z = s[p];
        // For the constant evaluation, repeat the steps done in PreFilterColor().
    #if defined(__GNUC__) || defined(__clang__)
        if(__builtin_constant_p(s))
    #endif
        { if(z == ' ') continue; if(z >= 'A' && z <= 'Z') z ^= 32; }

        unsigned code_lett = (lett >> 2*((z - 'a') & 31)) & 3 /* 0..3 */;
        unsigned code_dig  = (z + 4 - '0') & 15               /* 4..13 */;

        // z = (z&64) ? code_lett : code_dig; // Branchful version commented out
        // Branchless version of the same:
        // Bit 6 is set for letters, but not digits
        // Bit 5 is set for lowercase and digits, but not uppercase
        // Branchless version of the same:
        unsigned cond = -((z >> 6) & 1); // Test bit 6 to detect alphabet
        z = (code_lett & cond)  // True: want 3, false: want 0
          | (code_dig & ~cond); // True: want 0, false: want 15

        auto bits = 15 & (map >> 4*(z&15));
        result = result * 10 + bits;
    }
    return result % mod;
}

static constexpr const char _[] = "";
#define docolors(o) \
        o(   18, 0x3CB371, "medium sea green","MediumSeaGreen",_,_)\
        o(   55, 0xEE30A7, "maroon2",_,_,_) \
        o(   57, 0xFF34B3, "maroon1",_,_,_) \
        o(   59, 0x8B1C62, "maroon4",_,_,_) \
        o(   61, 0xCD2990, "maroon3",_,_,_) \
        o(   62, 0x8A2BE2, "blue violet","BlueViolet",_,_)\
        o(  114, 0x8FBC8F, "dark sea green","DarkSeaGreen",_,_)\
        o(  116, 0xD70751, "DebianRed",_,_,_) \
        o(  127, 0xAEEEEE, "PaleTurquoise2",_,_,_) \
        o(  129, 0xBBFFFF, "PaleTurquoise1",_,_,_) \
        o(  131, 0x668B8B, "PaleTurquoise4",_,_,_) \
        o(  133, 0x96CDCD, "PaleTurquoise3",_,_,_) \
        o(  135, 0xBCD2EE, "LightSteelBlue2",_,_,_) \
        o(  137, 0xCAE1FF, "LightSteelBlue1",_,_,_) \
        o(  139, 0x6E7B8B, "LightSteelBlue4",_,_,_) \
        o(  141, 0xA2B5CD, "LightSteelBlue3",_,_,_) \
        o(  146, 0x98FB98, "pale green","PaleGreen",_,_)\
        o(  155, 0xB9D3EE, "SlateGray2",_,_,_) \
        o(  157, 0xC6E2FF, "SlateGray1",_,_,_) \
        o(  159, 0x6C7B8B, "SlateGray4",_,_,_) \
        o(  161, 0x9FB6CD, "SlateGray3",_,_,_) \
        o(  182, 0xE9967A, "dark salmon","DarkSalmon",_,_)\
        o(  194, 0xA020F0, "purple",_,_,_) \
        o(  202, 0xFFFAFA, "snow",_,_,_) \
        o(  226, 0x228B22, "forest green","ForestGreen",_,_)\
        o(  228, 0x2F4F4F, "dark slate gray","DarkSlateGray","dark slate grey","DarkSlateGrey")\
        o(  263, 0xB2DFEE, "LightBlue2",_,_,_) \
        o(  265, 0xBFEFFF, "LightBlue1",_,_,_) \
        o(  267, 0x68838B, "LightBlue4",_,_,_) \
        o(  269, 0x9AC0CD, "LightBlue3",_,_,_) \
        o(  278, 0x5F9EA0, "cadet blue","CadetBlue",_,_)\
        o(  284, 0xE6E6FA, "lavender",_,_,_) \
        o(  286, 0xFF00FF, "magenta",_,_,_) \
        o(  322, 0x556B2F, "dark olive green","DarkOliveGreen",_,_)\
        o(  338, 0x7CFC00, "lawn green","LawnGreen",_,_)\
        o(  374, 0xFFC0CB, "pink",_,_,_) \
        o(  415, 0x4EEE94, "SeaGreen2",_,_,_) \
        o(  417, 0x54FF9F, "SeaGreen1",_,_,_) \
        o(  419, 0x2E8B57, "SeaGreen4",_,_,_) \
        o(  420, 0xF08080, "light coral","LightCoral",_,_)\
        o(  421, 0x43CD80, "SeaGreen3",_,_,_) \
        o(  422, 0xB0C4DE, "light steel blue","LightSteelBlue",_,_)\
        o(  423, 0x90EE90, "PaleGreen2",_,_,_) \
        o(  425, 0x9AFF9A, "PaleGreen1",_,_,_) \
        o(  427, 0x548B54, "PaleGreen4",_,_,_) \
        o(  429, 0x7CCD7C, "PaleGreen3",_,_,_) \
        o(  432, 0xDEB887, "burlywood",_,_,_) \
        o(  442, 0x20B2AA, "light sea green","LightSeaGreen",_,_)\
        o(  447, 0xEE7600, "DarkOrange2",_,_,_) \
        o(  449, 0xFF7F00, "DarkOrange1",_,_,_) \
        o(  451, 0x8B4500, "DarkOrange4",_,_,_) \
        o(  453, 0xCD6600, "DarkOrange3",_,_,_) \
        o(  487, 0xEEA2AD, "LightPink2",_,_,_) \
        o(  489, 0xFFAEB9, "LightPink1",_,_,_) \
        o(  491, 0x8B5F65, "LightPink4",_,_,_) \
        o(  492, 0xFFEBCD, "blanched almond","BlanchedAlmond",_,_)\
        o(  493, 0xCD8C95, "LightPink3",_,_,_) \
        o(  542, 0x00CED1, "dark turquoise","DarkTurquoise",_,_)\
        o(  570, 0xD8BFD8, "thistle",_,_,_) \
        o(  591, 0xEED2EE, "thistle2",_,_,_) \
        o(  593, 0xFFE1FF, "thistle1",_,_,_) \
        o(  595, 0x8B7B8B, "thistle4",_,_,_) \
        o(  597, 0xCDB5CD, "thistle3",_,_,_) \
        o(  607, 0x9F79EE, "MediumPurple2",_,_,_) \
        o(  609, 0xAB82FF, "MediumPurple1",_,_,_) \
        o(  611, 0x5D478B, "MediumPurple4",_,_,_) \
        o(  613, 0x8968CD, "MediumPurple3",_,_,_) \
        o(  654, 0xD2691E, "chocolate",_,_,_) \
        o(  656, 0xDCDCDC, "gainsboro",_,_,_) \
        o(  658, 0xFFFFE0, "light yellow","LightYellow",_,_)\
        o(  670, 0xFFE4C4, "bisque",_,_,_) \
        o(  672, 0xFFF5EE, "seashell",_,_,_) \
        o(  688, 0xBA55D3, "medium orchid","MediumOrchid",_,_)\
        o(  695, 0xEEE9BF, "LemonChiffon2",_,_,_) \
        o(  697, 0xFFFACD, "LemonChiffon1",_,_,_) \
        o(  699, 0x8B8970, "LemonChiffon4",_,_,_) \
        o(  701, 0xCDC9A5, "LemonChiffon3",_,_,_) \
        o(  742, 0xFFEFD5, "papaya whip","PapayaWhip",_,_)\
        o(  846, 0x7FFF00, "chartreuse",_,_,_) \
        o(  862, 0xFF1493, "deep pink","DeepPink",_,_)\
        o(  890, 0xE0FFFF, "light cyan","LightCyan",_,_)\
        o(  895, 0x00EEEE, "cyan2",_,_,_) \
        o(  897, 0x00FFFF, "cyan1",_,_,_) \
        o(  899, 0x008B8B, "cyan4",_,_,_) \
        o(  901, 0x00CDCD, "cyan3",_,_,_) \
        o(  914, 0xFFF8DC, "cornsilk",_,_,_) \
        o(  943, 0x76EEC6, "aquamarine2",_,_,_) \
        o(  945, 0x7FFFD4, "aquamarine1",_,_,_) \
        o(  947, 0x458B74, "aquamarine4",_,_,_) \
        o(  949, 0x66CDAA, "aquamarine3",_,_,_) \
        o(  954, 0x00FA9A, "medium spring green","MediumSpringGreen",_,_)\
        o(  962, 0x2E8B57, "sea green","SeaGreen",_,_)\
        o(  975, 0xEE7942, "sienna2",_,_,_) \
        o(  977, 0xFF8247, "sienna1",_,_,_) \
        o(  979, 0x8B4726, "sienna4",_,_,_) \
        o(  981, 0xCD6839, "sienna3",_,_,_) \
        o(  998, 0xFA8072, "salmon",_,_,_) \
        o( 1011, 0xEE5C42, "tomato2",_,_,_) \
        o( 1013, 0xFF6347, "tomato1",_,_,_) \
        o( 1015, 0x8B3626, "tomato4",_,_,_) \
        o( 1017, 0xCD4F39, "tomato3",_,_,_) \
        o( 1024, 0xDAA520, "goldenrod",_,_,_) \
        o( 1027, 0xEEB422, "goldenrod2",_,_,_) \
        o( 1029, 0xFFC125, "goldenrod1",_,_,_) \
        o( 1031, 0x8B6914, "goldenrod4",_,_,_) \
        o( 1033, 0xCD9B1D, "goldenrod3",_,_,_) \
        o( 1043, 0xEE7AE9, "orchid2",_,_,_) \
        o( 1045, 0xFF83FA, "orchid1",_,_,_) \
        o( 1047, 0x8B4789, "orchid4",_,_,_) \
        o( 1049, 0xCD69C9, "orchid3",_,_,_) \
        o( 1068, 0xD3D3D3, "light grey","LightGrey","light gray","LightGray")\
        o( 1098, 0xA52A2A, "brown",_,_,_) \
        o( 1166, 0x66CDAA, "medium aquamarine","MediumAquamarine",_,_)\
        o( 1170, 0xFF8C00, "dark orange","DarkOrange",_,_)\
        o( 1207, 0x436EEE, "RoyalBlue2",_,_,_) \
        o( 1209, 0x4876FF, "RoyalBlue1",_,_,_) \
        o( 1211, 0x27408B, "RoyalBlue4",_,_,_) \
        o( 1213, 0x3A5FCD, "RoyalBlue3",_,_,_) \
        o( 1250, 0xFAFAD2, "light goldenrod yellow","LightGoldenrodYellow",_,_)\
        o( 1251, 0x8DEEEE, "DarkSlateGray2",_,_,_) \
        o( 1253, 0x97FFFF, "DarkSlateGray1",_,_,_) \
        o( 1255, 0x528B8B, "DarkSlateGray4",_,_,_) \
        o( 1257, 0x79CDCD, "DarkSlateGray3",_,_,_) \
        o( 1295, 0x76EE00, "chartreuse2",_,_,_) \
        o( 1297, 0x7FFF00, "chartreuse1",_,_,_) \
        o( 1299, 0x458B00, "chartreuse4",_,_,_) \
        o( 1301, 0x66CD00, "chartreuse3",_,_,_) \
        o( 1310, 0xF0FFF0, "honeydew",_,_,_) \
        o( 1326, 0x8B008B, "dark magenta","DarkMagenta",_,_)\
        o( 1334, 0x483D8B, "dark slate blue","DarkSlateBlue",_,_)\
        o( 1342, 0xAFEEEE, "pale turquoise","PaleTurquoise",_,_)\
        o( 1410, 0x90EE90, "light green","LightGreen",_,_)\
        o( 1415, 0xEE7621, "chocolate2",_,_,_) \
        o( 1417, 0xFF7F24, "chocolate1",_,_,_) \
        o( 1419, 0x8B4513, "chocolate4",_,_,_) \
        o( 1421, 0xCD661D, "chocolate3",_,_,_) \
        o( 1460, 0x778899, "light slate gray","LightSlateGray","light slate grey","LightSlateGrey")\
        o( 1471, 0x0000EE, "blue2",_,_,_) \
        o( 1473, 0x0000FF, "blue1",_,_,_) \
        o( 1475, 0x00008B, "blue4",_,_,_) \
        o( 1477, 0x0000CD, "blue3",_,_,_) \
        o( 1482, 0x00FF7F, "spring green","SpringGreen",_,_)\
        o( 1494, 0xFF69B4, "hot pink","HotPink",_,_)\
        o( 1508, 0x696969, "dim gray","DimGray","dim grey","DimGrey")\
        o( 1511, 0xEEAEEE, "plum2",_,_,_) \
        o( 1513, 0xFFBBFF, "plum1",_,_,_) \
        o( 1515, 0x8B668B, "plum4",_,_,_) \
        o( 1517, 0xCD96CD, "plum3",_,_,_) \
        o( 1530, 0x000000, "black",_,_,_) \
        o( 1534, 0x4682B4, "steel blue","SteelBlue",_,_)\
        o( 1535, 0xEE9572, "LightSalmon2",_,_,_) \
        o( 1537, 0xFFA07A, "LightSalmon1",_,_,_) \
        o( 1539, 0x8B5742, "LightSalmon4",_,_,_) \
        o( 1541, 0xCD8162, "LightSalmon3",_,_,_) \
        o( 1558, 0xF0F8FF, "alice blue","AliceBlue",_,_)\
        o( 1579, 0xEE4000, "OrangeRed2",_,_,_) \
        o( 1581, 0xFF4500, "OrangeRed1",_,_,_) \
        o( 1583, 0x8B2500, "OrangeRed4",_,_,_) \
        o( 1585, 0xCD3700, "OrangeRed3",_,_,_) \
        o( 1594, 0x9370DB, "medium purple","MediumPurple",_,_)\
        o( 1603, 0xEEE5DE, "seashell2",_,_,_) \
        o( 1605, 0xFFF5EE, "seashell1",_,_,_) \
        o( 1607, 0x8B8682, "seashell4",_,_,_) \
        o( 1609, 0xCDC5BF, "seashell3",_,_,_) \
        o( 1628, 0xFF7F50, "coral",_,_,_) \
        o( 1630, 0xB0E0E6, "powder blue","PowderBlue",_,_)\
        o( 1647, 0x00EE00, "green2",_,_,_) \
        o( 1649, 0x00FF00, "green1",_,_,_) \
        o( 1651, 0x008B00, "green4",_,_,_) \
        o( 1653, 0x00CD00, "green3",_,_,_) \
        o( 1655, 0xEE6AA7, "HotPink2",_,_,_) \
        o( 1657, 0xFF6EB4, "HotPink1",_,_,_) \
        o( 1659, 0x8B3A62, "HotPink4",_,_,_) \
        o( 1661, 0xCD6090, "HotPink3",_,_,_) \
        o( 1668, 0xDB7093, "pale violet red","PaleVioletRed",_,_)\
        o( 1688, 0xFFD700, "gold",_,_,_) \
        o( 1695, 0xEEA9B8, "pink2",_,_,_) \
        o( 1697, 0xFFB5C5, "pink1",_,_,_) \
        o( 1699, 0x8B636C, "pink4",_,_,_) \
        o( 1701, 0xCD919E, "pink3",_,_,_) \
        o( 1714, 0xF5FFFA, "mint cream","MintCream",_,_)\
        o( 1715, 0xEE0000, "red2",_,_,_) \
        o( 1717, 0xFF0000, "red1",_,_,_) \
        o( 1718, 0x191970, "midnight blue","MidnightBlue",_,_)\
        o( 1719, 0x8B0000, "red4",_,_,_) \
        o( 1721, 0xCD0000, "red3",_,_,_) \
        o( 1734, 0xA0522D, "sienna",_,_,_) \
        o( 1742, 0x9400D3, "dark violet","DarkViolet",_,_)\
        o( 1743, 0x8EE5EE, "CadetBlue2",_,_,_) \
        o( 1745, 0x98F5FF, "CadetBlue1",_,_,_) \
        o( 1747, 0x53868B, "CadetBlue4",_,_,_) \
        o( 1749, 0x7AC5CD, "CadetBlue3",_,_,_) \
        o( 1778, 0x8B4513, "saddle brown","SaddleBrown",_,_)\
        o( 1791, 0xEE8262, "salmon2",_,_,_) \
        o( 1793, 0xFF8C69, "salmon1",_,_,_) \
        o( 1795, 0x8B4C39, "salmon4",_,_,_) \
        o( 1797, 0xCD7054, "salmon3",_,_,_) \
        o( 1803, 0xEECBAD, "PeachPuff2",_,_,_) \
        o( 1805, 0xFFDAB9, "PeachPuff1",_,_,_) \
        o( 1807, 0x8B7765, "PeachPuff4",_,_,_) \
        o( 1809, 0xCDAF95, "PeachPuff3",_,_,_) \
        o( 1827, 0xEE6363, "IndianRed2",_,_,_) \
        o( 1829, 0xFF6A6A, "IndianRed1",_,_,_) \
        o( 1831, 0x8B3A3A, "IndianRed4",_,_,_) \
        o( 1833, 0xCD5555, "IndianRed3",_,_,_) \
        o( 1839, 0xEE00EE, "magenta2",_,_,_) \
        o( 1841, 0xFF00FF, "magenta1",_,_,_) \
        o( 1843, 0x8B008B, "magenta4",_,_,_) \
        o( 1845, 0xCD00CD, "magenta3",_,_,_) \
        o( 1895, 0xFFFFFF, "gray100","grey100",_,_)\
        o( 1927, 0x912CEE, "purple2",_,_,_) \
        o( 1929, 0x9B30FF, "purple1",_,_,_) \
        o( 1931, 0x551A8B, "purple4",_,_,_) \
        o( 1933, 0x7D26CD, "purple3",_,_,_) \
        o( 1966, 0xF5F5F5, "white smoke","WhiteSmoke",_,_)\
        o( 1986, 0xFFA500, "orange",_,_,_) \
        o( 2039, 0x5CACEE, "SteelBlue2",_,_,_) \
        o( 2041, 0x63B8FF, "SteelBlue1",_,_,_) \
        o( 2043, 0x36648B, "SteelBlue4",_,_,_) \
        o( 2045, 0x4F94CD, "SteelBlue3",_,_,_) \
        o( 2063, 0x00B2EE, "DeepSkyBlue2",_,_,_) \
        o( 2065, 0x00BFFF, "DeepSkyBlue1",_,_,_) \
        o( 2067, 0x00688B, "DeepSkyBlue4",_,_,_) \
        o( 2069, 0x009ACD, "DeepSkyBlue3",_,_,_) \
        o( 2074, 0x6B8E23, "olive drab","OliveDrab",_,_)\
        o( 2083, 0xB23AEE, "DarkOrchid2",_,_,_) \
        o( 2085, 0xBF3EFF, "DarkOrchid1",_,_,_) \
        o( 2087, 0x68228B, "DarkOrchid4",_,_,_) \
        o( 2089, 0x9A32CD, "DarkOrchid3",_,_,_) \
        o( 2123, 0xEEAD0E, "DarkGoldenrod2",_,_,_) \
        o( 2125, 0xFFB90F, "DarkGoldenrod1",_,_,_) \
        o( 2127, 0x8B6508, "DarkGoldenrod4",_,_,_) \
        o( 2129, 0xCD950C, "DarkGoldenrod3",_,_,_) \
        o( 2146, 0x9ACD32, "yellow green","YellowGreen",_,_)\
        o( 2159, 0xB4EEB4, "DarkSeaGreen2",_,_,_) \
        o( 2161, 0xC1FFC1, "DarkSeaGreen1",_,_,_) \
        o( 2163, 0x698B69, "DarkSeaGreen4",_,_,_) \
        o( 2165, 0x9BCD9B, "DarkSeaGreen3",_,_,_) \
        o( 2174, 0xADD8E6, "light blue","LightBlue",_,_)\
        o( 2184, 0x8C8C8C, "gray55","grey55",_,_)\
        o( 2185, 0x7F7F7F, "gray50","grey50",_,_)\
        o( 2186, 0x919191, "gray57","grey57",_,_)\
        o( 2187, 0x858585, "gray52","grey52",_,_)\
        o( 2188, 0x8F8F8F, "gray56","grey56",_,_)\
        o( 2189, 0x828282, "gray51","grey51",_,_)\
        o( 2190, 0x949494, "gray58","grey58",_,_)\
        o( 2191, 0x8A8A8A, "gray54","grey54",_,_)\
        o( 2192, 0x969696, "gray59","grey59",_,_)\
        o( 2193, 0x878787, "gray53","grey53",_,_)\
        o( 2198, 0xFFA07A, "light salmon","LightSalmon",_,_)\
        o( 2204, 0xBFBFBF, "gray75","grey75",_,_)\
        o( 2205, 0xB3B3B3, "gray70","grey70",_,_)\
        o( 2206, 0xC4C4C4, "gray77","grey77",_,_)\
        o( 2207, 0xB8B8B8, "gray72","grey72",_,_)\
        o( 2208, 0xC2C2C2, "gray76","grey76",_,_)\
        o( 2209, 0xB5B5B5, "gray71","grey71",_,_)\
        o( 2210, 0xC7C7C7, "gray78","grey78",_,_)\
        o( 2211, 0xBDBDBD, "gray74","grey74",_,_)\
        o( 2212, 0xC9C9C9, "gray79","grey79",_,_)\
        o( 2213, 0xBABABA, "gray73","grey73",_,_)\
        o( 2214, 0x404040, "gray25","grey25",_,_)\
        o( 2215, 0x333333, "gray20","grey20",_,_)\
        o( 2216, 0x454545, "gray27","grey27",_,_)\
        o( 2217, 0x383838, "gray22","grey22",_,_)\
        o( 2218, 0x424242, "gray26","grey26",_,_)\
        o( 2219, 0x363636, "gray21","grey21",_,_)\
        o( 2220, 0x474747, "gray28","grey28",_,_)\
        o( 2221, 0x3D3D3D, "gray24","grey24",_,_)\
        o( 2222, 0x4A4A4A, "gray29","grey29",_,_)\
        o( 2223, 0x3B3B3B, "gray23","grey23",_,_)\
        o( 2224, 0xA6A6A6, "gray65","grey65",_,_)\
        o( 2225, 0x999999, "gray60","grey60",_,_)\
        o( 2226, 0xABABAB, "gray67","grey67",_,_)\
        o( 2227, 0x9E9E9E, "gray62","grey62",_,_)\
        o( 2228, 0xA8A8A8, "gray66","grey66",_,_)\
        o( 2229, 0x9C9C9C, "gray61","grey61",_,_)\
        o( 2230, 0xADADAD, "gray68","grey68",_,_)\
        o( 2231, 0xA3A3A3, "gray64","grey64",_,_)\
        o( 2232, 0xB0B0B0, "gray69","grey69",_,_)\
        o( 2233, 0xA1A1A1, "gray63","grey63",_,_)\
        o( 2234, 0x262626, "gray15","grey15",_,_)\
        o( 2235, 0x1A1A1A, "gray10","grey10",_,_)\
        o( 2236, 0x2B2B2B, "gray17","grey17",_,_)\
        o( 2237, 0x1F1F1F, "gray12","grey12",_,_)\
        o( 2238, 0x292929, "gray16","grey16",_,_)\
        o( 2239, 0x1C1C1C, "gray11","grey11",_,_)\
        o( 2240, 0x2E2E2E, "gray18","grey18",_,_)\
        o( 2241, 0x242424, "gray14","grey14",_,_)\
        o( 2242, 0x303030, "gray19","grey19",_,_)\
        o( 2243, 0x212121, "gray13","grey13",_,_)\
        o( 2244, 0xD9D9D9, "gray85","grey85",_,_)\
        o( 2245, 0xCCCCCC, "gray80","grey80",_,_)\
        o( 2246, 0xDEDEDE, "gray87","grey87",_,_)\
        o( 2247, 0xD1D1D1, "gray82","grey82",_,_)\
        o( 2248, 0xDBDBDB, "gray86","grey86",_,_)\
        o( 2249, 0xCFCFCF, "gray81","grey81",_,_)\
        o( 2250, 0xE0E0E0, "gray88","grey88",_,_)\
        o( 2251, 0xD6D6D6, "gray84","grey84",_,_)\
        o( 2252, 0xE3E3E3, "gray89","grey89",_,_)\
        o( 2253, 0xD4D4D4, "gray83","grey83",_,_)\
        o( 2254, 0x737373, "gray45","grey45",_,_)\
        o( 2255, 0x666666, "gray40","grey40",_,_)\
        o( 2256, 0x787878, "gray47","grey47",_,_)\
        o( 2257, 0x6B6B6B, "gray42","grey42",_,_)\
        o( 2258, 0x757575, "gray46","grey46",_,_)\
        o( 2259, 0x696969, "gray41","grey41",_,_)\
        o( 2260, 0x7A7A7A, "gray48","grey48",_,_)\
        o( 2261, 0x707070, "gray44","grey44",_,_)\
        o( 2262, 0x7D7D7D, "gray49","grey49",_,_)\
        o( 2263, 0x6E6E6E, "gray43","grey43",_,_)\
        o( 2264, 0xF2F2F2, "gray95","grey95",_,_)\
        o( 2265, 0xE5E5E5, "gray90","grey90",_,_)\
        o( 2266, 0xF7F7F7, "gray97","grey97",_,_)\
        o( 2267, 0xEBEBEB, "gray92","grey92",_,_)\
        o( 2268, 0xF5F5F5, "gray96","grey96",_,_)\
        o( 2269, 0xE8E8E8, "gray91","grey91",_,_)\
        o( 2270, 0xFAFAFA, "gray98","grey98",_,_)\
        o( 2271, 0xF0F0F0, "gray94","grey94",_,_)\
        o( 2272, 0xFCFCFC, "gray99","grey99",_,_)\
        o( 2273, 0xEDEDED, "gray93","grey93",_,_)\
        o( 2274, 0x595959, "gray35","grey35",_,_)\
        o( 2275, 0x4D4D4D, "gray30","grey30",_,_)\
        o( 2276, 0x5E5E5E, "gray37","grey37",_,_)\
        o( 2277, 0x525252, "gray32","grey32",_,_)\
        o( 2278, 0x5C5C5C, "gray36","grey36",_,_)\
        o( 2279, 0x4F4F4F, "gray31","grey31",_,_)\
        o( 2280, 0x616161, "gray38","grey38",_,_)\
        o( 2281, 0x575757, "gray34","grey34",_,_)\
        o( 2282, 0x636363, "gray39","grey39",_,_)\
        o( 2283, 0x545454, "gray33","grey33",_,_)\
        o( 2328, 0xFFDAB9, "peach puff","PeachPuff",_,_)\
        o( 2338, 0x00FFFF, "cyan",_,_,_) \
        o( 2350, 0xF5DEB3, "wheat",_,_,_) \
        o( 2351, 0xEEB4B4, "RosyBrown2",_,_,_) \
        o( 2353, 0xFFC1C1, "RosyBrown1",_,_,_) \
        o( 2355, 0x8B6969, "RosyBrown4",_,_,_) \
        o( 2356, 0xF0E68C, "khaki",_,_,_) \
        o( 2357, 0xCD9B9B, "RosyBrown3",_,_,_) \
        o( 2410, 0xF0FFFF, "azure",_,_,_) \
        o( 2446, 0x7FFFD4, "aquamarine",_,_,_) \
        o( 2447, 0xA4D3EE, "LightSkyBlue2",_,_,_) \
        o( 2449, 0xB0E2FF, "LightSkyBlue1",_,_,_) \
        o( 2451, 0x607B8B, "LightSkyBlue4",_,_,_) \
        o( 2453, 0x8DB6CD, "LightSkyBlue3",_,_,_) \
        o( 2454, 0xFFFAF0, "floral white","FloralWhite",_,_)\
        o( 2458, 0xF4A460, "sandy brown","SandyBrown",_,_)\
        o( 2466, 0xB22222, "firebrick",_,_,_) \
        o( 2467, 0xEE3A8C, "VioletRed2",_,_,_) \
        o( 2469, 0xFF3E96, "VioletRed1",_,_,_) \
        o( 2471, 0x8B2252, "VioletRed4",_,_,_) \
        o( 2473, 0xCD3278, "VioletRed3",_,_,_) \
        o( 2487, 0xEEEED1, "LightYellow2",_,_,_) \
        o( 2489, 0xFFFFE0, "LightYellow1",_,_,_) \
        o( 2491, 0x8B8B7A, "LightYellow4",_,_,_) \
        o( 2493, 0xCDCDB4, "LightYellow3",_,_,_) \
        o( 2516, 0xBEBEBE, "gray","grey",_,_)\
        o( 2542, 0xFFE4B5, "moccasin",_,_,_) \
        o( 2551, 0x00EE76, "SpringGreen2",_,_,_) \
        o( 2553, 0x00FF7F, "SpringGreen1",_,_,_) \
        o( 2555, 0x008B45, "SpringGreen4",_,_,_) \
        o( 2557, 0x00CD66, "SpringGreen3",_,_,_) \
        o( 2558, 0xEE82EE, "violet",_,_,_) \
        o( 2560, 0x9932CC, "dark orchid","DarkOrchid",_,_)\
        o( 2566, 0x8470FF, "light slate blue","LightSlateBlue",_,_)\
        o( 2571, 0xEEC900, "gold2",_,_,_) \
        o( 2572, 0x708090, "slate gray","SlateGray","slate grey","SlateGrey")\
        o( 2573, 0xFFD700, "gold1",_,_,_) \
        o( 2575, 0x8B7500, "gold4",_,_,_) \
        o( 2577, 0xCDAD00, "gold3",_,_,_) \
        o( 2599, 0xEED5B7, "bisque2",_,_,_) \
        o( 2601, 0xFFE4C4, "bisque1",_,_,_) \
        o( 2603, 0x8B7D6B, "bisque4",_,_,_) \
        o( 2605, 0xCDB79E, "bisque3",_,_,_) \
        o( 2618, 0x00FF00, "green",_,_,_) \
        o( 2646, 0xFAEBD7, "antique white","AntiqueWhite",_,_)\
        o( 2654, 0x48D1CC, "medium turquoise","MediumTurquoise",_,_)\
        o( 2672, 0x0D0D0D, "gray5","grey5",_,_)\
        o( 2673, 0x000000, "gray0","grey0",_,_)\
        o( 2674, 0x121212, "gray7","grey7",_,_)\
        o( 2675, 0x050505, "gray2","grey2",_,_)\
        o( 2676, 0x0F0F0F, "gray6","grey6",_,_)\
        o( 2677, 0x030303, "gray1","grey1",_,_)\
        o( 2678, 0x141414, "gray8","grey8",_,_)\
        o( 2679, 0x0A0A0A, "gray4","grey4",_,_)\
        o( 2680, 0x171717, "gray9","grey9",_,_)\
        o( 2681, 0x080808, "gray3","grey3",_,_)\
        o( 2694, 0x1E90FF, "dodger blue","DodgerBlue",_,_)\
        o( 2743, 0xEEEE00, "yellow2",_,_,_) \
        o( 2745, 0xFFFF00, "yellow1",_,_,_) \
        o( 2747, 0x8B8B00, "yellow4",_,_,_) \
        o( 2749, 0xCDCD00, "yellow3",_,_,_) \
        o( 2768, 0xB8860B, "dark goldenrod","DarkGoldenrod",_,_)\
        o( 2770, 0x008B8B, "dark cyan","DarkCyan",_,_)\
        o( 2786, 0xADFF2F, "green yellow","GreenYellow",_,_)\
        o( 2799, 0xEE3B3B, "brown2",_,_,_) \
        o( 2801, 0xFF4040, "brown1",_,_,_) \
        o( 2803, 0x8B2323, "brown4",_,_,_) \
        o( 2805, 0xCD3333, "brown3",_,_,_) \
        o( 2808, 0xFFFFF0, "ivory",_,_,_) \
        o( 2879, 0xEED5D2, "MistyRose2",_,_,_) \
        o( 2881, 0xFFE4E1, "MistyRose1",_,_,_) \
        o( 2883, 0x8B7D7B, "MistyRose4",_,_,_) \
        o( 2885, 0xCDB7B5, "MistyRose3",_,_,_) \
        o( 2886, 0x40E0D0, "turquoise",_,_,_) \
        o( 2948, 0xA9A9A9, "dark grey","DarkGrey","dark gray","DarkGray")\
        o( 2950, 0xFFFFFF, "white",_,_,_) \
        o( 2951, 0xEEDFCC, "AntiqueWhite2",_,_,_) \
        o( 2953, 0xFFEFDB, "AntiqueWhite1",_,_,_) \
        o( 2955, 0x8B8378, "AntiqueWhite4",_,_,_) \
        o( 2957, 0xCDC0B0, "AntiqueWhite3",_,_,_) \
        o( 2964, 0xFF6347, "tomato",_,_,_) \
        o( 2967, 0x7EC0EE, "SkyBlue2",_,_,_) \
        o( 2969, 0x87CEFF, "SkyBlue1",_,_,_) \
        o( 2971, 0x4A708B, "SkyBlue4",_,_,_) \
        o( 2973, 0x6CA6CD, "SkyBlue3",_,_,_) \
        o( 2980, 0xC71585, "medium violet red","MediumVioletRed",_,_)\
        o( 3014, 0xFFB6C1, "light pink","LightPink",_,_)\
        o( 3018, 0xFAF0E6, "linen",_,_,_) \
        o( 3039, 0x7A67EE, "SlateBlue2",_,_,_) \
        o( 3041, 0x836FFF, "SlateBlue1",_,_,_) \
        o( 3043, 0x473C8B, "SlateBlue4",_,_,_) \
        o( 3045, 0x6959CD, "SlateBlue3",_,_,_) \
        o( 3055, 0xEED8AE, "wheat2",_,_,_) \
        o( 3057, 0xFFE7BA, "wheat1",_,_,_) \
        o( 3059, 0x8B7E66, "wheat4",_,_,_) \
        o( 3061, 0xCDBA96, "wheat3",_,_,_) \
        o( 3086, 0x4169E1, "royal blue","RoyalBlue",_,_)\
        o( 3115, 0xEEE685, "khaki2",_,_,_) \
        o( 3117, 0xFFF68F, "khaki1",_,_,_) \
        o( 3119, 0x8B864E, "khaki4",_,_,_) \
        o( 3121, 0xCDC673, "khaki3",_,_,_) \
        o( 3126, 0x0000CD, "medium blue","MediumBlue",_,_)\
        o( 3148, 0xCD5C5C, "indian red","IndianRed",_,_)\
        o( 3183, 0xEE2C2C, "firebrick2",_,_,_) \
        o( 3185, 0xFF3030, "firebrick1",_,_,_) \
        o( 3187, 0x8B1A1A, "firebrick4",_,_,_) \
        o( 3189, 0xCD2626, "firebrick3",_,_,_) \
        o( 3211, 0xEEDC82, "LightGoldenrod2",_,_,_) \
        o( 3212, 0xD02090, "violet red","VioletRed",_,_)\
        o( 3213, 0xFFEC8B, "LightGoldenrod1",_,_,_) \
        o( 3215, 0x8B814C, "LightGoldenrod4",_,_,_) \
        o( 3217, 0xCDBE70, "LightGoldenrod3",_,_,_) \
        o( 3223, 0xBCEE68, "DarkOliveGreen2",_,_,_) \
        o( 3225, 0xCAFF70, "DarkOliveGreen1",_,_,_) \
        o( 3227, 0x6E8B3D, "DarkOliveGreen4",_,_,_) \
        o( 3229, 0xA2CD5A, "DarkOliveGreen3",_,_,_) \
        o( 3238, 0xFFFACD, "lemon chiffon","LemonChiffon",_,_)\
        o( 3258, 0xD2B48C, "tan",_,_,_) \
        o( 3262, 0x6495ED, "cornflower blue","CornflowerBlue",_,_)\
        o( 3283, 0xEEC591, "burlywood2",_,_,_) \
        o( 3285, 0xFFD39B, "burlywood1",_,_,_) \
        o( 3287, 0x8B7355, "burlywood4",_,_,_) \
        o( 3289, 0xCDAA7D, "burlywood3",_,_,_) \
        o( 3295, 0x00E5EE, "turquoise2",_,_,_) \
        o( 3297, 0x00F5FF, "turquoise1",_,_,_) \
        o( 3299, 0x00868B, "turquoise4",_,_,_) \
        o( 3301, 0x00C5CD, "turquoise3",_,_,_) \
        o( 3322, 0x32CD32, "lime green","LimeGreen",_,_)\
        o( 3351, 0xB3EE3A, "OliveDrab2",_,_,_) \
        o( 3353, 0xC0FF3E, "OliveDrab1",_,_,_) \
        o( 3355, 0x698B22, "OliveDrab4",_,_,_) \
        o( 3357, 0x9ACD32, "OliveDrab3",_,_,_) \
        o( 3376, 0xDA70D6, "orchid",_,_,_) \
        o( 3387, 0xEE799F, "PaleVioletRed2",_,_,_) \
        o( 3389, 0xFF82AB, "PaleVioletRed1",_,_,_) \
        o( 3391, 0x8B475D, "PaleVioletRed4",_,_,_) \
        o( 3393, 0xCD6889, "PaleVioletRed3",_,_,_) \
        o( 3423, 0x1C86EE, "DodgerBlue2",_,_,_) \
        o( 3425, 0x1E90FF, "DodgerBlue1",_,_,_) \
        o( 3427, 0x104E8B, "DodgerBlue4",_,_,_) \
        o( 3429, 0x1874CD, "DodgerBlue3",_,_,_) \
        o( 3446, 0x7B68EE, "medium slate blue","MediumSlateBlue",_,_)\
        o( 3470, 0x000080, "navy blue","NavyBlue",_,_)\
        o( 3495, 0xEE9A00, "orange2",_,_,_) \
        o( 3497, 0xFFA500, "orange1",_,_,_) \
        o( 3499, 0x8B5A00, "orange4",_,_,_) \
        o( 3501, 0xCD8500, "orange3",_,_,_) \
        o( 3503, 0xEE1289, "DeepPink2",_,_,_) \
        o( 3505, 0xFF1493, "DeepPink1",_,_,_) \
        o( 3507, 0x8B0A50, "DeepPink4",_,_,_) \
        o( 3509, 0xCD1076, "DeepPink3",_,_,_) \
        o( 3530, 0xFDF5E6, "old lace","OldLace",_,_)\
        o( 3532, 0xFF4500, "orange red","OrangeRed",_,_)\
        o( 3546, 0xFFFF00, "yellow",_,_,_) \
        o( 3547, 0xEEEEE0, "ivory2",_,_,_) \
        o( 3549, 0xFFFFF0, "ivory1",_,_,_) \
        o( 3551, 0x8B8B83, "ivory4",_,_,_) \
        o( 3553, 0xCDCDC1, "ivory3",_,_,_) \
        o( 3568, 0xEEE8AA, "pale goldenrod","PaleGoldenrod",_,_)\
        o( 3604, 0xBDB76B, "dark khaki","DarkKhaki",_,_)\
        o( 3622, 0x0000FF, "blue",_,_,_) \
        o( 3626, 0xDDA0DD, "plum",_,_,_) \
        o( 3646, 0xFFDEAD, "navajo white","NavajoWhite",_,_)\
        o( 3655, 0xE0EEEE, "azure2",_,_,_) \
        o( 3657, 0xF0FFFF, "azure1",_,_,_) \
        o( 3659, 0x838B8B, "azure4",_,_,_) \
        o( 3661, 0xC1CDCD, "azure3",_,_,_) \
        o( 3662, 0xFFE4E1, "misty rose","MistyRose",_,_)\
        o( 3667, 0xEEE0E5, "LavenderBlush2",_,_,_) \
        o( 3669, 0xFFF0F5, "LavenderBlush1",_,_,_) \
        o( 3671, 0x8B8386, "LavenderBlush4",_,_,_) \
        o( 3673, 0xCDC1C5, "LavenderBlush3",_,_,_) \
        o( 3678, 0x6A5ACD, "slate blue","SlateBlue",_,_)\
        o( 3686, 0xB03060, "maroon",_,_,_) \
        o( 3740, 0xFFF0F5, "lavender blush","LavenderBlush",_,_)\
        o( 3751, 0xEECFA1, "NavajoWhite2",_,_,_) \
        o( 3753, 0xFFDEAD, "NavajoWhite1",_,_,_) \
        o( 3755, 0x8B795E, "NavajoWhite4",_,_,_) \
        o( 3757, 0xCDB38B, "NavajoWhite3",_,_,_) \
        o( 3775, 0xD1EEEE, "LightCyan2",_,_,_) \
        o( 3777, 0xE0FFFF, "LightCyan1",_,_,_) \
        o( 3779, 0x7A8B8B, "LightCyan4",_,_,_) \
        o( 3781, 0xB4CDCD, "LightCyan3",_,_,_) \
        o( 3811, 0xD15FEE, "MediumOrchid2",_,_,_) \
        o( 3813, 0xE066FF, "MediumOrchid1",_,_,_) \
        o( 3815, 0x7A378B, "MediumOrchid4",_,_,_) \
        o( 3817, 0xB452CD, "MediumOrchid3",_,_,_) \
        o( 3822, 0x87CEFA, "light sky blue","LightSkyBlue",_,_)\
        o( 3852, 0xFF0000, "red",_,_,_) \
        o( 3866, 0x006400, "dark green","DarkGreen",_,_)\
        o( 3868, 0x000080, "navy",_,_,_) \
        o( 3870, 0xF8F8FF, "ghost white","GhostWhite",_,_)\
        o( 3882, 0xCD853F, "peru",_,_,_) \
        o( 3886, 0x00BFFF, "deep sky blue","DeepSkyBlue",_,_)\
        o( 3895, 0xE0EEE0, "honeydew2",_,_,_) \
        o( 3897, 0xF0FFF0, "honeydew1",_,_,_) \
        o( 3899, 0x838B83, "honeydew4",_,_,_) \
        o( 3901, 0xC1CDC1, "honeydew3",_,_,_) \
        o( 3951, 0xEE9A49, "tan2",_,_,_) \
        o( 3953, 0xFFA54F, "tan1",_,_,_) \
        o( 3955, 0x8B5A2B, "tan4",_,_,_) \
        o( 3957, 0xCD853F, "tan3",_,_,_) \
        o( 3996, 0x8B0000, "dark red","DarkRed",_,_)\
        o( 4000, 0xEEDD82, "light goldenrod","LightGoldenrod",_,_)\
        o( 4011, 0xEE6A50, "coral2",_,_,_) \
        o( 4013, 0xFF7256, "coral1",_,_,_) \
        o( 4015, 0x8B3E2F, "coral4",_,_,_) \
        o( 4017, 0xCD5B45, "coral3",_,_,_) \
        o( 4018, 0xBC8F8F, "rosy brown","RosyBrown",_,_)\
        o( 4023, 0xEEE8CD, "cornsilk2",_,_,_) \
        o( 4025, 0xFFF8DC, "cornsilk1",_,_,_) \
        o( 4027, 0x8B8878, "cornsilk4",_,_,_) \
        o( 4029, 0xCDC8B1, "cornsilk3",_,_,_) \
        o( 4050, 0xF5F5DC, "beige",_,_,_) \
        o( 4054, 0x00008B, "dark blue","DarkBlue",_,_)\
        o( 4063, 0xEEE9E9, "snow2",_,_,_) \
        o( 4065, 0xFFFAFA, "snow1",_,_,_) \
        o( 4067, 0x8B8989, "snow4",_,_,_) \
        o( 4069, 0xCDC9C9, "snow3",_,_,_) \
        o( 4078, 0x87CEEB, "sky blue","SkyBlue",_,_)\

static std::pair<unsigned long long, unsigned> ParseHex(const char*& s)
{
    char* endptr = nullptr;
    unsigned long long result = std::strtoll(s, &endptr, 16);
    unsigned length = endptr-s;
    s = endptr;
    return {result, length};
}
static unsigned ParseDbl(const char*& s)
{
    char* endptr = nullptr;
    double result = std::strtod(s, &endptr);
    s = endptr;
    return unsigned(255.9999 * std::max(std::min(result, 1.), 0.));
}
enum { use_separator=1, lsb_first=2, use_dbl=4 };
static unsigned ParseColorNumber(const char* source, unsigned specs)
{
    unsigned result        = 0;
    unsigned long long val = 0;
    unsigned length        = 0;
    unsigned bits          = 0;

    for(unsigned n=0; n<3; ++n)
    {
        while(*source == '/') ++source;

        if(specs & use_dbl)
        {
            val  = ParseDbl(source);
            bits = 8;
        }
        else if((specs & use_separator) || n==0)
        {
            std::tie(val, length) = ParseHex(source);
            bits = length*4;
            if(!(specs & use_separator)) bits /= 3;
        }

        unsigned mask = ((0x1u << bits)-1);
        unsigned temp = val & mask; val >>= bits;
        // Scale to 0xFF.
        temp = temp * 0xFF / mask;
        if(specs & lsb_first)
        {
            result += temp << (8*n);
            // Place n=0 to LSB and n=2 to MSB.
        }
        else
        {
            result <<= 8;
            result += temp;
            // Place n=0 to MSB and n=0 to MSB. Reverse of above.
        }
    }
    return result;
}
static std::pair<unsigned,bool> TryParseColorNumber(std::string_view lc)
{
    const char* source = nullptr;
    unsigned    specs  = 0;

    if(lc.size() >= 4)
    {
        static constexpr const char hextext[]  = {'#'};
        static constexpr const char rgbtext[]  = {'r','g','b',':'};
        static constexpr const char rgbitext[] = {'r','g','b','i',':'};

        if(std::memcmp(lc.data(), hextext, sizeof hextext) == 0) // shortest: "#FFF" = 4 characters
        {
            source = lc.data()+1;
            specs  = lsb_first;
        }
        else if(lc.size() >= 9)
        {
            if(std::memcmp(lc.data(), rgbtext, sizeof rgbtext) == 0) // shortest: "rgb:F/F/F" = 9
            {
                source = lc.data()+4;
                specs  = use_separator;
            }
            else if(std::memcmp(lc.data(), rgbitext, sizeof rgbitext) == 0) // shortest: "rgbi:1/1/1" = 10
            {
                source = lc.data()+5;
                specs  = use_separator | use_dbl;
            }
        }
    }
    return source
        ? std::pair{ ParseColorNumber(source, specs), true }
        : std::pair{ 0u, false };
}

static std::string PreFilterColor(std::string_view s)
{
    std::string lc(s);
    /* Create a copy of the color name, with spaces erased and characters lowercased */
    auto first = lc.begin(), end = lc.end();
    for(auto begin = first; begin != end; ++begin)
        if(*begin != ' ')
        {
            *first = tolower(*begin);
            ++first;
        }
    lc.erase(first, end);
    return lc;
}

static constexpr unsigned FindColor(unsigned h)
{
    #define o(a,b,s1,s2,s3,s4) case a: return b; break;
    switch(h) { docolors(o) }
    #undef o
    return 0;
}
template <std::size_t... N>
static constexpr const std::array<unsigned, sizeof...(N)> GetColors(std::index_sequence<N...>)
{
    return std::array<unsigned, sizeof...(N)>
    {
        // Using a separate function for this rather than
        // inlining a long ?: chain makes compilation on GCC fast.
        // While Clang has no trouble with a long ?: chain, GCC would take ages to compile it.
        FindColor(N) ...
    };
}

unsigned ParseColorName(std::string_view s)
{
    std::string lc = PreFilterColor(s);

    /* First check if it's #ABC, #AABBCC, rgb:aa/bb/cc or rgbi:0.2/0.6/0.8 */
    auto [result,success] = TryParseColorNumber(lc);
    if(success) return result;

    /* Nope? Then look it up in the hardcoded version of Xorg colors database. */
    /* Begin by reducing the provided color into a small hash that is unique
     * for all colors in the database.
     */
    unsigned short h = hash(lc.data(), lc.size());

    /* Verify that the hashes in docolors() match the implementation in hash() */
    #define v(a,b,s1,s2,s3,s4) static_assert(!s1[0] || hash(s1, sizeof(s1)-1) == a); \
                               static_assert(!s2[0] || hash(s2, sizeof(s2)-1) == a); \
                               static_assert(!s3[0] || hash(s3, sizeof(s3)-1) == a); \
                               static_assert(!s4[0] || hash(s4, sizeof(s4)-1) == a);
    docolors(v);
    #undef v

#if defined(__GNUC__)
    /* GCC generates pretty optimal code for this. */
    /* This produces an extra range check compared to what's below, but needs less data. */
    return FindColor(h);
#elif defined(__clang__) || defined(__GNUC__)
    /* Clang and GCC compile this well. */
    static constexpr const auto colorvalues = GetColors(std::make_index_sequence<mod>());
    return colorvalues[h];
#else
    #define o(a,b,s1,s2,s3,s4) a,
    #define q(a,b,s1,s2,s3,s4) b,
    static const unsigned short colorkeys[] { docolors(o) };
    static const unsigned int colorvalues[] { docolors(q) };
    #undef q
    #undef o

    auto i = std::lower_bound(std::begin(colorkeys), std::end(colorkeys), h);
    if(i != std::end(colorkeys) && *i == h)
        return colorvalues[ i-std::begin(colorkeys) ];
    return 0;
#endif
}
