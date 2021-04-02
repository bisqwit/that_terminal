#include <array>
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

static constexpr unsigned mod = 3931u;
// Note: Not using std::string_view, because clang falsely reports it's not a built-in-constant when it is.
static constexpr unsigned hash(const char* s, std::size_t length)
{
    constexpr std::uint_least64_t lett = 0xB3D70D84F0AC6; // maps a..z into 0..3 
    constexpr std::uint_least64_t map = 0x32784160594783; // maps everything into 0..9
    std::uint_least64_t result = 0x1BDA14BA4D9538F9;
    for(std::size_t p=0; p<length; ++p)
    {
        unsigned z = s[p], t;

        /* code_lett gets same result for both uppercase and lowercase. */
        unsigned ch = (2*z - 2*'a');
        if(std::is_constant_evaluated()) ch &= 63;//2*31;
        unsigned code_lett = (4*lett) >> ch      /* 0..3 */;
        unsigned code_dig  = (4*z + 4*4 - 4*'0') /* 4..13 */;

        // z = (z&64) ? code_lett : code_dig; // Branchful version commented out
        // Branchless version of the same:
        // Bit 6 is set for lowercase and uppercase, but not digits
        // Bit 5 is set for lowercase and digits, but not uppercase
        // Bit 4 is set for digits and half of letters
    #ifdef __clang__
        code_lett &= 3*4;
        if(std::is_constant_evaluated()) code_dig &= 63;//15*4;
        // Clang optimizes this into a cmov
        t = (z & 64) ? code_lett : code_dig;
    #else
        // For GCC, this produces better code, still not very good though.
        t = (z >> 6) & 1;          // Test bit 6 to detect alphabet
        unsigned m = ~0u;
        if(std::is_constant_evaluated()) m = 63; //15*4;
        t = (code_lett & t*3*4)    // True: want 3*4, false: want 0
          + (code_dig  & ~-t & m); // True: want 0,   false: want 15*4
    #endif

        // If the symbol was space, don't update result (skip spaces)
        // Clang generates branchless cmov from this.
        // GCC is not quite that smart...
        std::uint_least64_t space = -std::uint_least64_t(z != ' ');
        result = ((result*10 + ((map >> t) & 15)) & space) + (result & ~space);
    }
    return result % mod;
}

#define docolors(o) \
        o(   25, 0xBDB76B, "dark khaki", "DarkKhaki") \
        o(   32, 0xEE7621, "chocolate2") \
        o(   33, 0x8B4513, "chocolate4") \
        o(   37, 0xFF7F24, "chocolate1") \
        o(   38, 0xCD661D, "chocolate3") \
        o(   66, 0xEEE8AA, "pale goldenrod", "PaleGoldenrod") \
        o(   68, 0x006400, "dark green", "DarkGreen") \
        o(  109, 0xBCEE68, "DarkOliveGreen2") \
        o(  110, 0x6E8B3D, "DarkOliveGreen4") \
        o(  114, 0xCAFF70, "DarkOliveGreen1") \
        o(  115, 0xA2CD5A, "DarkOliveGreen3") \
        o(  162, 0xEEAD0E, "DarkGoldenrod2") \
        o(  163, 0x8B6508, "DarkGoldenrod4") \
        o(  167, 0xFFB90F, "DarkGoldenrod1") \
        o(  168, 0xCD950C, "DarkGoldenrod3") \
        o(  176, 0xFFA500, "orange") \
        o(  184, 0xB03060, "maroon") \
        o(  194, 0x8FBC8F, "dark sea green", "DarkSeaGreen") \
        o(  207, 0xEEE5DE, "seashell2") \
        o(  208, 0x8B8682, "seashell4") \
        o(  212, 0xFFF5EE, "seashell1") \
        o(  213, 0xCDC5BF, "seashell3") \
        o(  236, 0xEE0000, "red2") \
        o(  237, 0x8B0000, "red4") \
        o(  241, 0xFF0000, "red1") \
        o(  242, 0xCD0000, "red3") \
        o(  243, 0xFFFFFF, "white") \
        o(  248, 0xEEDC82, "LightGoldenrod2") \
        o(  249, 0x8B814C, "LightGoldenrod4") \
        o(  253, 0xFFEC8B, "LightGoldenrod1") \
        o(  254, 0xCDBE70, "LightGoldenrod3") \
        o(  258, 0xFFF8DC, "cornsilk") \
        o(  309, 0x000080, "navy") \
        o(  313, 0xEE799F, "PaleVioletRed2") \
        o(  314, 0x8B475D, "PaleVioletRed4") \
        o(  318, 0xFF82AB, "PaleVioletRed1") \
        o(  319, 0xCD6889, "PaleVioletRed3") \
        o(  320, 0xB0C4DE, "light steel blue", "LightSteelBlue") \
        o(  328, 0xEEEED1, "LightYellow2") \
        o(  329, 0x8B8B7A, "LightYellow4") \
        o(  333, 0xFFFFE0, "LightYellow1") \
        o(  334, 0xCDCDB4, "LightYellow3") \
        o(  335, 0x40E0D0, "turquoise") \
        o(  341, 0xFFA07A, "light salmon", "LightSalmon") \
        o(  377, 0x3CB371, "medium sea green", "MediumSeaGreen") \
        o(  380, 0xFFD700, "gold") \
        o(  381, 0x90EE90, "PaleGreen2") \
        o(  382, 0x548B54, "PaleGreen4") \
        o(  386, 0x9AFF9A, "PaleGreen1") \
        o(  387, 0x7CCD7C, "PaleGreen3") \
        o(  400, 0xA9A9A9, "dark grey", "DarkGrey", "dark gray", "DarkGray") \
        o(  405, 0x696969, "dim gray", "DimGray", "dim grey", "DimGrey") \
        o(  414, 0x7EC0EE, "SkyBlue2") \
        o(  415, 0x4A708B, "SkyBlue4") \
        o(  416, 0xF8F8FF, "ghost white", "GhostWhite") \
        o(  419, 0x87CEFF, "SkyBlue1") \
        o(  420, 0x6CA6CD, "SkyBlue3") \
        o(  478, 0x6A5ACD, "slate blue", "SlateBlue") \
        o(  483, 0xEE7600, "DarkOrange2") \
        o(  484, 0x8B4500, "DarkOrange4") \
        o(  486, 0xEE82EE, "violet") \
        o(  488, 0xFF7F00, "DarkOrange1") \
        o(  489, 0xCD6600, "DarkOrange3") \
        o(  496, 0x66CDAA, "medium aquamarine", "MediumAquamarine") \
        o(  503, 0x008B8B, "dark cyan", "DarkCyan") \
        o(  522, 0x000080, "navy blue", "NavyBlue") \
        o(  534, 0xF5DEB3, "wheat") \
        o(  536, 0xEE2C2C, "firebrick2") \
        o(  537, 0x8B1A1A, "firebrick4") \
        o(  541, 0xFF3030, "firebrick1") \
        o(  542, 0xCD2626, "firebrick3") \
        o(  573, 0xD8BFD8, "thistle") \
        o(  601, 0xB9D3EE, "SlateGray2") \
        o(  602, 0x6C7B8B, "SlateGray4") \
        o(  605, 0xFF00FF, "magenta") \
        o(  606, 0xC6E2FF, "SlateGray1") \
        o(  607, 0x9FB6CD, "SlateGray3") \
        o(  647, 0x87CEFA, "light sky blue", "LightSkyBlue") \
        o(  665, 0xEEE9E9, "snow2") \
        o(  666, 0x8B8989, "snow4") \
        o(  670, 0xFFFAFA, "snow1") \
        o(  671, 0xCDC9C9, "snow3") \
        o(  678, 0xDCDCDC, "gainsboro") \
        o(  694, 0xE0EEE0, "honeydew2") \
        o(  695, 0x838B83, "honeydew4") \
        o(  697, 0x7A67EE, "SlateBlue2") \
        o(  698, 0x473C8B, "SlateBlue4") \
        o(  699, 0xF0FFF0, "honeydew1") \
        o(  700, 0xC1CDC1, "honeydew3") \
        o(  702, 0x836FFF, "SlateBlue1") \
        o(  703, 0x6959CD, "SlateBlue3") \
        o(  776, 0xDEB887, "burlywood") \
        o(  796, 0x9932CC, "dark orchid", "DarkOrchid") \
        o(  828, 0xF4A460, "sandy brown", "SandyBrown") \
        o(  838, 0xFFFFE0, "light yellow", "LightYellow") \
        o(  845, 0xC71585, "medium violet red", "MediumVioletRed") \
        o(  855, 0xB22222, "firebrick") \
        o(  872, 0xF0F8FF, "alice blue", "AliceBlue") \
        o(  891, 0x4682B4, "steel blue", "SteelBlue") \
        o(  894, 0x8B4513, "saddle brown", "SaddleBrown") \
        o(  896, 0x5CACEE, "SteelBlue2") \
        o(  897, 0x36648B, "SteelBlue4") \
        o(  901, 0x63B8FF, "SteelBlue1") \
        o(  902, 0x4F94CD, "SteelBlue3") \
        o(  909, 0xEE3B3B, "brown2") \
        o(  910, 0x8B2323, "brown4") \
        o(  914, 0xFF4040, "brown1") \
        o(  915, 0xCD3333, "brown3") \
        o(  919, 0xEE7AE9, "orchid2") \
        o(  920, 0x8B4789, "orchid4") \
        o(  924, 0xFF83FA, "orchid1") \
        o(  925, 0xCD69C9, "orchid3") \
        o(  939, 0x912CEE, "purple2") \
        o(  940, 0x551A8B, "purple4") \
        o(  944, 0x9B30FF, "purple1") \
        o(  945, 0x7D26CD, "purple3") \
        o(  952, 0xF5F5DC, "beige") \
        o(  953, 0x4169E1, "royal blue", "RoyalBlue") \
        o(  969, 0xFFC0CB, "pink") \
        o( 1003, 0xD15FEE, "MediumOrchid2") \
        o( 1004, 0x7A378B, "MediumOrchid4") \
        o( 1008, 0xE066FF, "MediumOrchid1") \
        o( 1009, 0xB452CD, "MediumOrchid3") \
        o( 1023, 0x9F79EE, "MediumPurple2") \
        o( 1024, 0x5D478B, "MediumPurple4") \
        o( 1028, 0xAB82FF, "MediumPurple1") \
        o( 1029, 0x8968CD, "MediumPurple3") \
        o( 1036, 0xB0E0E6, "powder blue", "PowderBlue") \
        o( 1042, 0xEE8262, "salmon2") \
        o( 1043, 0x8B4C39, "salmon4") \
        o( 1047, 0xFF8C69, "salmon1") \
        o( 1048, 0xCD7054, "salmon3") \
        o( 1049, 0xD3D3D3, "light grey", "LightGrey", "light gray", "LightGray") \
        o( 1127, 0xE6E6FA, "lavender") \
        o( 1142, 0x228B22, "forest green", "ForestGreen") \
        o( 1152, 0xE0FFFF, "light cyan", "LightCyan") \
        o( 1155, 0xB3EE3A, "OliveDrab2") \
        o( 1156, 0x698B22, "OliveDrab4") \
        o( 1160, 0xC0FF3E, "OliveDrab1") \
        o( 1161, 0x9ACD32, "OliveDrab3") \
        o( 1170, 0xF5FFFA, "mint cream", "MintCream") \
        o( 1181, 0xEED8AE, "wheat2") \
        o( 1182, 0x8B7E66, "wheat4") \
        o( 1185, 0x6495ED, "cornflower blue", "CornflowerBlue") \
        o( 1186, 0xFFE7BA, "wheat1") \
        o( 1187, 0xCDBA96, "wheat3") \
        o( 1194, 0x556B2F, "dark olive green", "DarkOliveGreen") \
        o( 1216, 0x00FA9A, "medium spring green", "MediumSpringGreen") \
        o( 1219, 0xEE6363, "IndianRed2") \
        o( 1220, 0x8B3A3A, "IndianRed4") \
        o( 1224, 0xFF6A6A, "IndianRed1") \
        o( 1225, 0xCD5555, "IndianRed3") \
        o( 1226, 0xFFEFD5, "papaya whip", "PapayaWhip") \
        o( 1234, 0xCD853F, "peru") \
        o( 1241, 0xDB7093, "pale violet red", "PaleVioletRed") \
        o( 1255, 0xFDF5E6, "old lace", "OldLace") \
        o( 1258, 0xFF8C00, "dark orange", "DarkOrange") \
        o( 1280, 0xFFFAFA, "snow") \
        o( 1281, 0xEED5B7, "bisque2") \
        o( 1282, 0x8B7D6B, "bisque4") \
        o( 1286, 0xFFE4C4, "bisque1") \
        o( 1287, 0xCDB79E, "bisque3") \
        o( 1293, 0xA52A2A, "brown") \
        o( 1301, 0x00EE76, "SpringGreen2") \
        o( 1302, 0x008B45, "SpringGreen4") \
        o( 1306, 0x00FF7F, "SpringGreen1") \
        o( 1307, 0x00CD66, "SpringGreen3") \
        o( 1310, 0x6B8E23, "olive drab", "OliveDrab") \
        o( 1335, 0xEE9A49, "tan2") \
        o( 1336, 0x8B5A2B, "tan4") \
        o( 1340, 0xFFA54F, "tan1") \
        o( 1341, 0xCD853F, "tan3") \
        o( 1379, 0xDDA0DD, "plum") \
        o( 1417, 0x5F9EA0, "cadet blue", "CadetBlue") \
        o( 1452, 0x0000FF, "blue") \
        o( 1486, 0xEEA9B8, "pink2") \
        o( 1487, 0x8B636C, "pink4") \
        o( 1491, 0xFFB5C5, "pink1") \
        o( 1492, 0xCD919E, "pink3") \
        o( 1509, 0xFFDEAD, "navajo white", "NavajoWhite") \
        o( 1516, 0x436EEE, "RoyalBlue2") \
        o( 1517, 0x27408B, "RoyalBlue4") \
        o( 1521, 0x4876FF, "RoyalBlue1") \
        o( 1522, 0x3A5FCD, "RoyalBlue3") \
        o( 1523, 0xEEDFCC, "AntiqueWhite2") \
        o( 1524, 0x8B8378, "AntiqueWhite4") \
        o( 1528, 0xFFEFDB, "AntiqueWhite1") \
        o( 1529, 0xCDC0B0, "AntiqueWhite3") \
        o( 1560, 0xEE7942, "sienna2") \
        o( 1561, 0x8B4726, "sienna4") \
        o( 1565, 0xFF8247, "sienna1") \
        o( 1566, 0xCD6839, "sienna3") \
        o( 1568, 0x9400D3, "dark violet", "DarkViolet") \
        o( 1572, 0xEEB4B4, "RosyBrown2") \
        o( 1573, 0x8B6969, "RosyBrown4") \
        o( 1576, 0x7FFF00, "chartreuse") \
        o( 1577, 0xFFC1C1, "RosyBrown1") \
        o( 1578, 0xCD9B9B, "RosyBrown3") \
        o( 1590, 0xFFFFF0, "ivory") \
        o( 1593, 0x32CD32, "lime green", "LimeGreen") \
        o( 1601, 0xEEDD82, "light goldenrod", "LightGoldenrod") \
        o( 1608, 0xEE9A00, "orange2") \
        o( 1609, 0x8B5A00, "orange4") \
        o( 1613, 0xFFA500, "orange1") \
        o( 1614, 0xCD8500, "orange3") \
        o( 1619, 0xB8860B, "dark goldenrod", "DarkGoldenrod") \
        o( 1641, 0xAEEEEE, "PaleTurquoise2") \
        o( 1642, 0x668B8B, "PaleTurquoise4") \
        o( 1646, 0xBBFFFF, "PaleTurquoise1") \
        o( 1647, 0x96CDCD, "PaleTurquoise3") \
        o( 1649, 0x8B0000, "dark red", "DarkRed") \
        o( 1655, 0xEEAEEE, "plum2") \
        o( 1656, 0x8B668B, "plum4") \
        o( 1660, 0xFFBBFF, "plum1") \
        o( 1661, 0xCD96CD, "plum3") \
        o( 1673, 0xEEA2AD, "LightPink2") \
        o( 1674, 0x8B5F65, "LightPink4") \
        o( 1678, 0xFFAEB9, "LightPink1") \
        o( 1679, 0xCD8C95, "LightPink3") \
        o( 1688, 0xEE30A7, "maroon2") \
        o( 1689, 0x8B1C62, "maroon4") \
        o( 1691, 0xEEE0E5, "LavenderBlush2") \
        o( 1692, 0x8B8386, "LavenderBlush4") \
        o( 1693, 0xFF34B3, "maroon1") \
        o( 1694, 0xCD2990, "maroon3") \
        o( 1696, 0xFFF0F5, "LavenderBlush1") \
        o( 1697, 0xCDC1C5, "LavenderBlush3") \
        o( 1727, 0x7CFC00, "lawn green", "LawnGreen") \
        o( 1750, 0xB4EEB4, "DarkSeaGreen2") \
        o( 1751, 0x698B69, "DarkSeaGreen4") \
        o( 1755, 0xC1FFC1, "DarkSeaGreen1") \
        o( 1756, 0x9BCD9B, "DarkSeaGreen3") \
        o( 1764, 0x48D1CC, "medium turquoise", "MediumTurquoise") \
        o( 1776, 0x20B2AA, "light sea green", "LightSeaGreen") \
        o( 1799, 0xEED2EE, "thistle2") \
        o( 1800, 0x8B7B8B, "thistle4") \
        o( 1804, 0xFFE1FF, "thistle1") \
        o( 1805, 0xCDB5CD, "thistle3") \
        o( 1810, 0xEECBAD, "PeachPuff2") \
        o( 1811, 0x8B7765, "PeachPuff4") \
        o( 1815, 0xFFDAB9, "PeachPuff1") \
        o( 1816, 0xCDAF95, "PeachPuff3") \
        o( 1844, 0xFF69B4, "hot pink", "HotPink") \
        o( 1870, 0xEE3A8C, "VioletRed2") \
        o( 1871, 0x8B2252, "VioletRed4") \
        o( 1875, 0xFF3E96, "VioletRed1") \
        o( 1876, 0xCD3278, "VioletRed3") \
        o( 1906, 0x00BFFF, "deep sky blue", "DeepSkyBlue") \
        o( 1907, 0x7B68EE, "medium slate blue", "MediumSlateBlue") \
        o( 1913, 0xEEB422, "goldenrod2") \
        o( 1914, 0x8B6914, "goldenrod4") \
        o( 1917, 0xD70751, "DebianRed") \
        o( 1918, 0xFFC125, "goldenrod1") \
        o( 1919, 0xCD9B1D, "goldenrod3") \
        o( 1930, 0x7FFFD4, "aquamarine") \
        o( 1948, 0xFFFAF0, "floral white", "FloralWhite") \
        o( 1982, 0x00008B, "dark blue", "DarkBlue") \
        o( 1990, 0xFFF5EE, "seashell") \
        o( 2081, 0xEEEE00, "yellow2") \
        o( 2082, 0x8B8B00, "yellow4") \
        o( 2086, 0xFFFF00, "yellow1") \
        o( 2087, 0xCDCD00, "yellow3") \
        o( 2119, 0xEE00EE, "magenta2") \
        o( 2120, 0x8B008B, "magenta4") \
        o( 2124, 0xFF00FF, "magenta1") \
        o( 2125, 0xCD00CD, "magenta3") \
        o( 2148, 0xFFB6C1, "light pink", "LightPink") \
        o( 2160, 0xAFEEEE, "pale turquoise", "PaleTurquoise") \
        o( 2165, 0xFFF0F5, "lavender blush", "LavenderBlush") \
        o( 2172, 0xDAA520, "goldenrod") \
        o( 2191, 0xEE5C42, "tomato2") \
        o( 2192, 0x8B3626, "tomato4") \
        o( 2196, 0xFF6347, "tomato1") \
        o( 2197, 0xCD4F39, "tomato3") \
        o( 2215, 0xEE6A50, "coral2") \
        o( 2216, 0x8B3E2F, "coral4") \
        o( 2220, 0xFF7256, "coral1") \
        o( 2221, 0xCD5B45, "coral3") \
        o( 2223, 0xFAF0E6, "linen") \
        o( 2225, 0x8EE5EE, "CadetBlue2") \
        o( 2226, 0x53868B, "CadetBlue4") \
        o( 2230, 0x98F5FF, "CadetBlue1") \
        o( 2231, 0x7AC5CD, "CadetBlue3") \
        o( 2248, 0x000000, "black") \
        o( 2256, 0x778899, "light slate gray", "LightSlateGray", "light slate grey", "LightSlateGrey") \
        o( 2274, 0x2F4F4F, "dark slate gray", "DarkSlateGray", "dark slate grey", "DarkSlateGrey") \
        o( 2283, 0xF0E68C, "khaki") \
        o( 2289, 0x050505, "gray2", "grey2") \
        o( 2290, 0x0A0A0A, "gray4", "grey4") \
        o( 2291, 0x141414, "gray8", "grey8") \
        o( 2292, 0x171717, "gray9", "grey9") \
        o( 2293, 0x0D0D0D, "gray5", "grey5") \
        o( 2294, 0x030303, "gray1", "grey1") \
        o( 2295, 0x080808, "gray3", "grey3") \
        o( 2296, 0x121212, "gray7", "grey7") \
        o( 2297, 0x0F0F0F, "gray6", "grey6") \
        o( 2298, 0x000000, "gray0", "grey0") \
        o( 2303, 0x4EEE94, "SeaGreen2") \
        o( 2304, 0x2E8B57, "SeaGreen4") \
        o( 2308, 0x54FF9F, "SeaGreen1") \
        o( 2309, 0x43CD80, "SeaGreen3") \
        o( 2326, 0x00FF00, "green") \
        o( 2349, 0xA4D3EE, "LightSkyBlue2") \
        o( 2350, 0x607B8B, "LightSkyBlue4") \
        o( 2354, 0xB0E2FF, "LightSkyBlue1") \
        o( 2355, 0x8DB6CD, "LightSkyBlue3") \
        o( 2376, 0xFFFACD, "lemon chiffon", "LemonChiffon") \
        o( 2377, 0xD2691E, "chocolate") \
        o( 2385, 0x0000EE, "blue2") \
        o( 2386, 0x00008B, "blue4") \
        o( 2390, 0x0000FF, "blue1") \
        o( 2391, 0x0000CD, "blue3") \
        o( 2400, 0x87CEEB, "sky blue", "SkyBlue") \
        o( 2420, 0xADFF2F, "green yellow", "GreenYellow") \
        o( 2473, 0xFAFAD2, "light goldenrod yellow", "LightGoldenrodYellow") \
        o( 2478, 0xFA8072, "salmon") \
        o( 2513, 0x90EE90, "light green", "LightGreen") \
        o( 2531, 0xBC8F8F, "rosy brown", "RosyBrown") \
        o( 2532, 0xFFE4B5, "moccasin") \
        o( 2542, 0xEEE8CD, "cornsilk2") \
        o( 2543, 0x8B8878, "cornsilk4") \
        o( 2547, 0xFFF8DC, "cornsilk1") \
        o( 2548, 0xCDC8B1, "cornsilk3") \
        o( 2572, 0xB2DFEE, "LightBlue2") \
        o( 2573, 0x68838B, "LightBlue4") \
        o( 2577, 0xBFEFFF, "LightBlue1") \
        o( 2578, 0x9AC0CD, "LightBlue3") \
        o( 2631, 0xADD8E6, "light blue", "LightBlue") \
        o( 2658, 0x1C86EE, "DodgerBlue2") \
        o( 2659, 0x104E8B, "DodgerBlue4") \
        o( 2663, 0x1E90FF, "DodgerBlue1") \
        o( 2664, 0x1874CD, "DodgerBlue3") \
        o( 2675, 0xFF1493, "deep pink", "DeepPink") \
        o( 2698, 0x0000CD, "medium blue", "MediumBlue") \
        o( 2716, 0xEE6AA7, "HotPink2") \
        o( 2717, 0x8B3A62, "HotPink4") \
        o( 2721, 0xFF6EB4, "HotPink1") \
        o( 2722, 0xCD6090, "HotPink3") \
        o( 2735, 0xFFE4E1, "misty rose", "MistyRose") \
        o( 2772, 0x191970, "midnight blue", "MidnightBlue") \
        o( 2781, 0x8DEEEE, "DarkSlateGray2") \
        o( 2782, 0x528B8B, "DarkSlateGray4") \
        o( 2786, 0x97FFFF, "DarkSlateGray1") \
        o( 2787, 0x79CDCD, "DarkSlateGray3") \
        o( 2805, 0x98FB98, "pale green", "PaleGreen") \
        o( 2827, 0x708090, "slate gray", "SlateGray", "slate grey", "SlateGrey") \
        o( 2871, 0xBA55D3, "medium orchid", "MediumOrchid") \
        o( 2873, 0x9370DB, "medium purple", "MediumPurple") \
        o( 2895, 0xFFE4C4, "bisque") \
        o( 2923, 0xFAEBD7, "antique white", "AntiqueWhite") \
        o( 2947, 0xEEE685, "khaki2") \
        o( 2948, 0x8B864E, "khaki4") \
        o( 2952, 0xFFF68F, "khaki1") \
        o( 2953, 0xCDC673, "khaki3") \
        o( 2975, 0xFFFF00, "yellow") \
        o( 2986, 0xFF6347, "tomato") \
        o( 2996, 0xFF7F50, "coral") \
        o( 3000, 0xFFFFFF, "gray100", "grey100") \
        o( 3007, 0x383838, "gray22", "grey22") \
        o( 3008, 0x3D3D3D, "gray24", "grey24") \
        o( 3009, 0x474747, "gray28", "grey28") \
        o( 3010, 0x4A4A4A, "gray29", "grey29") \
        o( 3011, 0x404040, "gray25", "grey25") \
        o( 3012, 0x363636, "gray21", "grey21") \
        o( 3013, 0x3B3B3B, "gray23", "grey23") \
        o( 3014, 0x454545, "gray27", "grey27") \
        o( 3015, 0x424242, "gray26", "grey26") \
        o( 3016, 0x333333, "gray20", "grey20") \
        o( 3017, 0x6B6B6B, "gray42", "grey42") \
        o( 3018, 0x707070, "gray44", "grey44") \
        o( 3019, 0x7A7A7A, "gray48", "grey48") \
        o( 3020, 0x7D7D7D, "gray49", "grey49") \
        o( 3021, 0x737373, "gray45", "grey45") \
        o( 3022, 0x696969, "gray41", "grey41") \
        o( 3023, 0x6E6E6E, "gray43", "grey43") \
        o( 3024, 0x787878, "gray47", "grey47") \
        o( 3025, 0x757575, "gray46", "grey46") \
        o( 3026, 0x666666, "gray40", "grey40") \
        o( 3027, 0xD1D1D1, "gray82", "grey82") \
        o( 3028, 0xD6D6D6, "gray84", "grey84") \
        o( 3029, 0xE0E0E0, "gray88", "grey88") \
        o( 3030, 0xE3E3E3, "gray89", "grey89") \
        o( 3031, 0xD9D9D9, "gray85", "grey85") \
        o( 3032, 0xCFCFCF, "gray81", "grey81") \
        o( 3033, 0xD4D4D4, "gray83", "grey83") \
        o( 3034, 0xDEDEDE, "gray87", "grey87") \
        o( 3035, 0xDBDBDB, "gray86", "grey86") \
        o( 3036, 0xCCCCCC, "gray80", "grey80") \
        o( 3037, 0xEBEBEB, "gray92", "grey92") \
        o( 3038, 0xF0F0F0, "gray94", "grey94") \
        o( 3039, 0xFAFAFA, "gray98", "grey98") \
        o( 3040, 0xFCFCFC, "gray99", "grey99") \
        o( 3041, 0xF2F2F2, "gray95", "grey95") \
        o( 3042, 0xE8E8E8, "gray91", "grey91") \
        o( 3043, 0xEDEDED, "gray93", "grey93") \
        o( 3044, 0xF7F7F7, "gray97", "grey97") \
        o( 3045, 0xF5F5F5, "gray96", "grey96") \
        o( 3046, 0xE5E5E5, "gray90", "grey90") \
        o( 3047, 0x858585, "gray52", "grey52") \
        o( 3048, 0x8A8A8A, "gray54", "grey54") \
        o( 3049, 0x949494, "gray58", "grey58") \
        o( 3050, 0x969696, "gray59", "grey59") \
        o( 3051, 0x8C8C8C, "gray55", "grey55") \
        o( 3052, 0x828282, "gray51", "grey51") \
        o( 3053, 0x878787, "gray53", "grey53") \
        o( 3054, 0x919191, "gray57", "grey57") \
        o( 3055, 0x8F8F8F, "gray56", "grey56") \
        o( 3056, 0x7F7F7F, "gray50", "grey50") \
        o( 3057, 0x1F1F1F, "gray12", "grey12") \
        o( 3058, 0x242424, "gray14", "grey14") \
        o( 3059, 0x2E2E2E, "gray18", "grey18") \
        o( 3060, 0x303030, "gray19", "grey19") \
        o( 3061, 0x262626, "gray15", "grey15") \
        o( 3062, 0x1C1C1C, "gray11", "grey11") \
        o( 3063, 0x212121, "gray13", "grey13") \
        o( 3064, 0x2B2B2B, "gray17", "grey17") \
        o( 3065, 0x292929, "gray16", "grey16") \
        o( 3066, 0x1A1A1A, "gray10", "grey10") \
        o( 3067, 0x525252, "gray32", "grey32") \
        o( 3068, 0x575757, "gray34", "grey34") \
        o( 3069, 0x616161, "gray38", "grey38") \
        o( 3070, 0x636363, "gray39", "grey39") \
        o( 3071, 0x595959, "gray35", "grey35") \
        o( 3072, 0x4F4F4F, "gray31", "grey31") \
        o( 3073, 0x545454, "gray33", "grey33") \
        o( 3074, 0x5E5E5E, "gray37", "grey37") \
        o( 3075, 0x5C5C5C, "gray36", "grey36") \
        o( 3076, 0x4D4D4D, "gray30", "grey30") \
        o( 3077, 0xB8B8B8, "gray72", "grey72") \
        o( 3078, 0xBDBDBD, "gray74", "grey74") \
        o( 3079, 0xC7C7C7, "gray78", "grey78") \
        o( 3080, 0xC9C9C9, "gray79", "grey79") \
        o( 3081, 0xBFBFBF, "gray75", "grey75") \
        o( 3082, 0xB5B5B5, "gray71", "grey71") \
        o( 3083, 0xBABABA, "gray73", "grey73") \
        o( 3084, 0xC4C4C4, "gray77", "grey77") \
        o( 3085, 0xC2C2C2, "gray76", "grey76") \
        o( 3086, 0xB3B3B3, "gray70", "grey70") \
        o( 3087, 0x9E9E9E, "gray62", "grey62") \
        o( 3088, 0xA3A3A3, "gray64", "grey64") \
        o( 3089, 0xADADAD, "gray68", "grey68") \
        o( 3090, 0xB0B0B0, "gray69", "grey69") \
        o( 3091, 0xA6A6A6, "gray65", "grey65") \
        o( 3092, 0x9C9C9C, "gray61", "grey61") \
        o( 3093, 0xA1A1A1, "gray63", "grey63") \
        o( 3094, 0xABABAB, "gray67", "grey67") \
        o( 3095, 0xA8A8A8, "gray66", "grey66") \
        o( 3096, 0x999999, "gray60", "grey60") \
        o( 3107, 0xEECFA1, "NavajoWhite2") \
        o( 3108, 0x8B795E, "NavajoWhite4") \
        o( 3112, 0xFFDEAD, "NavajoWhite1") \
        o( 3113, 0xCDB38B, "NavajoWhite3") \
        o( 3126, 0xEE1289, "DeepPink2") \
        o( 3127, 0x8B0A50, "DeepPink4") \
        o( 3131, 0xFF1493, "DeepPink1") \
        o( 3132, 0xCD1076, "DeepPink3") \
        o( 3145, 0xF0FFFF, "azure") \
        o( 3146, 0x00B2EE, "DeepSkyBlue2") \
        o( 3147, 0x00688B, "DeepSkyBlue4") \
        o( 3151, 0x00BFFF, "DeepSkyBlue1") \
        o( 3152, 0x009ACD, "DeepSkyBlue3") \
        o( 3162, 0xBCD2EE, "LightSteelBlue2") \
        o( 3163, 0x6E7B8B, "LightSteelBlue4") \
        o( 3167, 0xCAE1FF, "LightSteelBlue1") \
        o( 3168, 0xA2B5CD, "LightSteelBlue3") \
        o( 3183, 0xF08080, "light coral", "LightCoral") \
        o( 3195, 0xFF0000, "red") \
        o( 3198, 0x00E5EE, "turquoise2") \
        o( 3199, 0x00868B, "turquoise4") \
        o( 3203, 0x00F5FF, "turquoise1") \
        o( 3204, 0x00C5CD, "turquoise3") \
        o( 3218, 0xF0FFF0, "honeydew") \
        o( 3220, 0xEE9572, "LightSalmon2") \
        o( 3221, 0x8B5742, "LightSalmon4") \
        o( 3225, 0xFFA07A, "LightSalmon1") \
        o( 3226, 0xCD8162, "LightSalmon3") \
        o( 3272, 0x76EEC6, "aquamarine2") \
        o( 3273, 0x458B74, "aquamarine4") \
        o( 3277, 0x7FFFD4, "aquamarine1") \
        o( 3278, 0x66CDAA, "aquamarine3") \
        o( 3316, 0xA0522D, "sienna") \
        o( 3319, 0x00EEEE, "cyan2") \
        o( 3320, 0x008B8B, "cyan4") \
        o( 3324, 0x00FFFF, "cyan1") \
        o( 3325, 0x00CDCD, "cyan3") \
        o( 3341, 0xFFDAB9, "peach puff", "PeachPuff") \
        o( 3347, 0xD02090, "violet red", "VioletRed") \
        o( 3377, 0x00EE00, "green2") \
        o( 3378, 0x008B00, "green4") \
        o( 3382, 0x00FF00, "green1") \
        o( 3383, 0x00CD00, "green3") \
        o( 3411, 0x8B008B, "dark magenta", "DarkMagenta") \
        o( 3429, 0xEE4000, "OrangeRed2") \
        o( 3430, 0x8B2500, "OrangeRed4") \
        o( 3434, 0xFF4500, "OrangeRed1") \
        o( 3435, 0xCD3700, "OrangeRed3") \
        o( 3441, 0x1E90FF, "dodger blue", "DodgerBlue") \
        o( 3458, 0xEEC900, "gold2") \
        o( 3459, 0x8B7500, "gold4") \
        o( 3463, 0xFFD700, "gold1") \
        o( 3464, 0xCDAD00, "gold3") \
        o( 3506, 0xD1EEEE, "LightCyan2") \
        o( 3507, 0x7A8B8B, "LightCyan4") \
        o( 3511, 0xE0FFFF, "LightCyan1") \
        o( 3512, 0xB4CDCD, "LightCyan3") \
        o( 3560, 0xE9967A, "dark salmon", "DarkSalmon") \
        o( 3612, 0xEED5D2, "MistyRose2") \
        o( 3613, 0x8B7D7B, "MistyRose4") \
        o( 3614, 0xFFEBCD, "blanched almond", "BlanchedAlmond") \
        o( 3617, 0xFFE4E1, "MistyRose1") \
        o( 3618, 0xCDB7B5, "MistyRose3") \
        o( 3645, 0xDA70D6, "orchid") \
        o( 3647, 0xA020F0, "purple") \
        o( 3663, 0x76EE00, "chartreuse2") \
        o( 3664, 0x458B00, "chartreuse4") \
        o( 3668, 0x7FFF00, "chartreuse1") \
        o( 3669, 0x66CD00, "chartreuse3") \
        o( 3675, 0xCD5C5C, "indian red", "IndianRed") \
        o( 3677, 0xEEC591, "burlywood2") \
        o( 3678, 0x8B7355, "burlywood4") \
        o( 3682, 0xFFD39B, "burlywood1") \
        o( 3683, 0xCDAA7D, "burlywood3") \
        o( 3687, 0x00FF7F, "spring green", "SpringGreen") \
        o( 3698, 0xD2B48C, "tan") \
        o( 3705, 0xE0EEEE, "azure2") \
        o( 3706, 0x838B8B, "azure4") \
        o( 3710, 0xF0FFFF, "azure1") \
        o( 3711, 0xC1CDCD, "azure3") \
        o( 3713, 0x00CED1, "dark turquoise", "DarkTurquoise") \
        o( 3725, 0xB23AEE, "DarkOrchid2") \
        o( 3726, 0x68228B, "DarkOrchid4") \
        o( 3730, 0xBF3EFF, "DarkOrchid1") \
        o( 3731, 0x9A32CD, "DarkOrchid3") \
        o( 3772, 0x2E8B57, "sea green", "SeaGreen") \
        o( 3800, 0x8A2BE2, "blue violet", "BlueViolet") \
        o( 3801, 0xBEBEBE, "gray", "grey") \
        o( 3838, 0x8470FF, "light slate blue", "LightSlateBlue") \
        o( 3842, 0xF5F5F5, "white smoke", "WhiteSmoke") \
        o( 3853, 0x9ACD32, "yellow green", "YellowGreen") \
        o( 3856, 0x483D8B, "dark slate blue", "DarkSlateBlue") \
        o( 3879, 0xEEEEE0, "ivory2") \
        o( 3880, 0x8B8B83, "ivory4") \
        o( 3884, 0xFFFFF0, "ivory1") \
        o( 3885, 0xCDCDC1, "ivory3") \
        o( 3896, 0xFF4500, "orange red", "OrangeRed") \
        o( 3904, 0x00FFFF, "cyan") \
        o( 3915, 0xEEE9BF, "LemonChiffon2") \
        o( 3916, 0x8B8970, "LemonChiffon4") \
        o( 3920, 0xFFFACD, "LemonChiffon1") \
        o( 3921, 0xCDC9A5, "LemonChiffon3") \

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

static constexpr unsigned FindColor(unsigned h)
{
    #define o(a,b,...) case a: return b; break;
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


template<std::size_t N>
struct fixed_string
{
    constexpr fixed_string(const char (&s)[N]) { std::copy_n(s, N, str); }
    constexpr const char* data() const { return str; }
    constexpr std::size_t size() const { return N-1; }
    char str[N];
};
template<unsigned h, fixed_string s>    static constexpr void VerifyHash1() { static_assert(hash(s.data(), s.size()) == h); }
template<unsigned h, fixed_string... s> static constexpr void VerifyHash()  { (VerifyHash1<h, s>(), ...); }

unsigned ParseColorName(std::string_view s)
{
    /* First check if it's #ABC, #AABBCC, rgb:aa/bb/cc or rgbi:0.2/0.6/0.8 */
    auto [result,success] = TryParseColorNumber(s);
    if(success) return result;

    /* Nope? Then look it up in the hardcoded version of Xorg colors database. */
    /* Begin by reducing the provided color into a small hash that is unique
     * for all colors in the database.
     */
    unsigned short h = hash(s.data(), s.size());

    /* Verify that the hashes in docolors() match the implementation in hash() */
    #define v(a,b,...) VerifyHash<a, __VA_ARGS__>();
    docolors(v);
    #undef v

#if defined(__GNUC__) && !defined(__clang__)
    /* GCC generates pretty optimal code for this. */
    /* This produces an extra range check compared to what's below, but needs less data. */
    /* Amount of data: same as below, minus ~100 bytes by average */
    return FindColor(h);
#elif defined(__clang__) || defined(__GNUC__)
    /* Clang and GCC compile this well. No comparisons, just a singular data access. */
    /* Amount of data: mod*4 = ~15 kB */
    static constexpr const auto colorvalues = GetColors(std::make_index_sequence<mod>());
    return colorvalues[h];
#else
    #define o(a,b,...) a,
    #define q(a,b,...) b,
    static const unsigned short colorkeys[] { docolors(o) };
    static const unsigned int colorvalues[] { docolors(q) };
    #undef q
    #undef o
    /* This uses least data, but performs about log2(753) = 10 comparisons. */
    /* Amount of data: 753*2 + 753*4 = ~4.5 kB */
    auto i = std::lower_bound(std::begin(colorkeys), std::end(colorkeys), h);
    if(i != std::end(colorkeys) && *i == h)
        return colorvalues[ i-std::begin(colorkeys) ];
    return 0;
#endif
}
