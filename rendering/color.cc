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

static constexpr unsigned mod = 3832u, mul = 380u;
// Note: Not using std::string_view, because clang falsely reports it's not a built-in-constant when it is.
static constexpr unsigned hash(const char* s, std::size_t length)
{
    constexpr std::uint_least64_t lett = 0x927B029ACC77F; // maps a..z into 0..3 
    constexpr std::uint_least64_t map = 0x928CFB571EE207; // maps everything into hex digits
    std::uint_least64_t result = 0x7923C1BDEEE579CE;
    for(std::size_t p=0; p<length; ++p)
    {
        unsigned z = s[p], t;

        std::uint_least64_t space = -std::uint_least64_t(z != ' ');
        /* code_lett gets same result for both uppercase and lowercase. */
        unsigned ch = (2*z - 2*'a');
        unsigned code_lett = (4*lett) >> (ch&63) /* 0..3 */;
        unsigned code_dig  = (4*z + 4*4 - 4*'0') /* 4..13 */;

        // z = (z&64) ? code_lett : code_dig; // Branchful version commented out
        // Branchless version of the same:
        // Bit 6 is set for lowercase and uppercase, but not digits
        // Bit 5 is set for lowercase and digits, but not uppercase
        // Bit 4 is set for digits and half of letters
    #ifdef __clang__
        code_lett &= 3*4;
        // Clang optimizes this into a cmov
        t = (z & 64) ? code_lett : code_dig;
    #else
        // For GCC, this produces better code, still not very good though.
        t = (z >> 6) & 1;          // Test bit 6 to detect alphabet
        t = (code_lett & t*3*4)    // True: want 3*4, false: want 0
          + (code_dig  & ~-t);     // True: want 0,   false: want 15*4
    #endif

        // If the symbol was space, don't update result (skip spaces)
        // Clang generates branchless cmov from this.
        // GCC is not quite that smart...
        result = ((result*mul + ((map >> (t&63)) & 15)) & space) + (result & ~space);
    }
    return result % mod;
}

#define docolors(o) \
        o(    1, 0xFFB5C5, "pink1")\
        o(    5, 0xCD919E, "pink3")\
        o(    7, 0xEEA9B8, "pink2")\
        o(   11, 0x8B636C, "pink4")\
        o(   30, 0x6495ED, "cornflower blue", "CornflowerBlue")\
        o(   42, 0xFFFAFA, "snow")\
        o(   65, 0xFFFAFA, "snow1")\
        o(   66, 0xB03060, "maroon")\
        o(   69, 0xCDC9C9, "snow3")\
        o(   70, 0xFFE4B5, "moccasin")\
        o(   71, 0xEEE9E9, "snow2")\
        o(   72, 0x000080, "navy")\
        o(   75, 0x8B8989, "snow4")\
        o(   78, 0x7B68EE, "medium slate blue", "MediumSlateBlue")\
        o(   80, 0x2F4F4F, "dark slate gray", "DarkSlateGray", "dark slate grey", "DarkSlateGrey")\
        o(   94, 0xFDF5E6, "old lace", "OldLace")\
        o(  104, 0xEEE8AA, "pale goldenrod", "PaleGoldenrod")\
        o(  120, 0xF5FFFA, "mint cream", "MintCream")\
        o(  134, 0x0000CD, "medium blue", "MediumBlue")\
        o(  138, 0xEE82EE, "violet")\
        o(  176, 0xC71585, "medium violet red", "MediumVioletRed")\
        o(  189, 0xFFF68F, "khaki1")\
        o(  193, 0xCDC673, "khaki3")\
        o(  195, 0xEEE685, "khaki2")\
        o(  199, 0x8B864E, "khaki4")\
        o(  209, 0xFF8C69, "salmon1")\
        o(  213, 0xCD7054, "salmon3")\
        o(  215, 0xEE8262, "salmon2")\
        o(  219, 0x8B4C39, "salmon4")\
        o(  221, 0x696969, "gray41", "grey41")\
        o(  222, 0x7A7A7A, "gray48", "grey48")\
        o(  225, 0x6E6E6E, "gray43", "grey43")\
        o(  227, 0x6B6B6B, "gray42", "grey42")\
        o(  228, 0x787878, "gray47", "grey47")\
        o(  229, 0x7D7D7D, "gray49", "grey49")\
        o(  231, 0x707070, "gray44", "grey44")\
        o(  232, 0x757575, "gray46", "grey46")\
        o(  234, 0x666666, "gray40", "grey40")\
        o(  235, 0x737373, "gray45", "grey45")\
        o(  240, 0xD70751, "DebianRed")\
        o(  253, 0x1C1C1C, "gray11", "grey11")\
        o(  254, 0x2E2E2E, "gray18", "grey18")\
        o(  257, 0x212121, "gray13", "grey13")\
        o(  259, 0x1F1F1F, "gray12", "grey12")\
        o(  260, 0x2B2B2B, "gray17", "grey17")\
        o(  261, 0x303030, "gray19", "grey19")\
        o(  263, 0x242424, "gray14", "grey14")\
        o(  264, 0x292929, "gray16", "grey16")\
        o(  266, 0x1A1A1A, "gray10", "grey10")\
        o(  267, 0x262626, "gray15", "grey15")\
        o(  274, 0x8B4513, "saddle brown", "SaddleBrown")\
        o(  329, 0xFFEFDB, "AntiqueWhite1")\
        o(  333, 0xCDC0B0, "AntiqueWhite3")\
        o(  335, 0xEEDFCC, "AntiqueWhite2")\
        o(  338, 0x2E8B57, "sea green", "SeaGreen")\
        o(  339, 0x8B8378, "AntiqueWhite4")\
        o(  353, 0xFFE4C4, "bisque1")\
        o(  357, 0xCDB79E, "bisque3")\
        o(  359, 0xEED5B7, "bisque2")\
        o(  361, 0x7FFF00, "chartreuse1")\
        o(  362, 0xA52A2A, "brown")\
        o(  363, 0x8B7D6B, "bisque4")\
        o(  365, 0x66CD00, "chartreuse3")\
        o(  367, 0x76EE00, "chartreuse2")\
        o(  369, 0xFF0000, "red1")\
        o(  371, 0x458B00, "chartreuse4")\
        o(  373, 0xCD0000, "red3")\
        o(  375, 0xEE0000, "red2")\
        o(  377, 0xFF83FA, "orchid1")\
        o(  379, 0x8B0000, "red4")\
        o(  381, 0xCD69C9, "orchid3")\
        o(  383, 0xEE7AE9, "orchid2")\
        o(  387, 0x8B4789, "orchid4")\
        o(  392, 0xFFD700, "gold")\
        o(  393, 0x63B8FF, "SteelBlue1")\
        o(  397, 0x4F94CD, "SteelBlue3")\
        o(  399, 0x5CACEE, "SteelBlue2")\
        o(  403, 0x36648B, "SteelBlue4")\
        o(  426, 0xF08080, "light coral", "LightCoral")\
        o(  430, 0x00008B, "dark blue", "DarkBlue")\
        o(  433, 0xFFFACD, "LemonChiffon1")\
        o(  437, 0xCDC9A5, "LemonChiffon3")\
        o(  439, 0xEEE9BF, "LemonChiffon2")\
        o(  443, 0x8B8970, "LemonChiffon4")\
        o(  490, 0xD2B48C, "tan")\
        o(  505, 0x00BFFF, "DeepSkyBlue1")\
        o(  509, 0x009ACD, "DeepSkyBlue3")\
        o(  510, 0xA020F0, "purple")\
        o(  511, 0x00B2EE, "DeepSkyBlue2")\
        o(  515, 0x00688B, "DeepSkyBlue4")\
        o(  526, 0x40E0D0, "turquoise")\
        o(  529, 0x836FFF, "SlateBlue1")\
        o(  533, 0x6959CD, "SlateBlue3")\
        o(  535, 0x7A67EE, "SlateBlue2")\
        o(  539, 0x473C8B, "SlateBlue4")\
        o(  564, 0xDA70D6, "orchid")\
        o(  569, 0xFFD39B, "burlywood1")\
        o(  573, 0xCDAA7D, "burlywood3")\
        o(  574, 0x6A5ACD, "slate blue", "SlateBlue")\
        o(  575, 0xEEC591, "burlywood2")\
        o(  579, 0x8B7355, "burlywood4")\
        o(  601, 0x9C9C9C, "gray61", "grey61")\
        o(  602, 0xADADAD, "gray68", "grey68")\
        o(  605, 0xA1A1A1, "gray63", "grey63")\
        o(  606, 0xF0F8FF, "alice blue", "AliceBlue")\
        o(  607, 0x9E9E9E, "gray62", "grey62")\
        o(  608, 0xABABAB, "gray67", "grey67")\
        o(  609, 0xB0B0B0, "gray69", "grey69")\
        o(  611, 0xA3A3A3, "gray64", "grey64")\
        o(  612, 0xA8A8A8, "gray66", "grey66")\
        o(  614, 0x999999, "gray60", "grey60")\
        o(  615, 0xA6A6A6, "gray65", "grey65")\
        o(  617, 0xFF7256, "coral1")\
        o(  621, 0xCD5B45, "coral3")\
        o(  623, 0xEE6A50, "coral2")\
        o(  627, 0x8B3E2F, "coral4")\
        o(  633, 0xCFCFCF, "gray81", "grey81")\
        o(  634, 0xE0E0E0, "gray88", "grey88")\
        o(  637, 0xD4D4D4, "gray83", "grey83")\
        o(  639, 0xD1D1D1, "gray82", "grey82")\
        o(  640, 0xDEDEDE, "gray87", "grey87")\
        o(  641, 0xE3E3E3, "gray89", "grey89")\
        o(  643, 0xD6D6D6, "gray84", "grey84")\
        o(  644, 0xDBDBDB, "gray86", "grey86")\
        o(  646, 0xCCCCCC, "gray80", "grey80")\
        o(  647, 0xD9D9D9, "gray85", "grey85")\
        o(  672, 0xD02090, "violet red", "VioletRed")\
        o(  689, 0xF0FFF0, "honeydew1")\
        o(  693, 0xC1CDC1, "honeydew3")\
        o(  695, 0xE0EEE0, "honeydew2")\
        o(  697, 0xFFB90F, "DarkGoldenrod1")\
        o(  699, 0x838B83, "honeydew4")\
        o(  701, 0xCD950C, "DarkGoldenrod3")\
        o(  703, 0xEEAD0E, "DarkGoldenrod2")\
        o(  707, 0x8B6508, "DarkGoldenrod4")\
        o(  722, 0x8A2BE2, "blue violet", "BlueViolet")\
        o(  737, 0x1E90FF, "DodgerBlue1")\
        o(  738, 0xFFB6C1, "light pink", "LightPink")\
        o(  741, 0x1874CD, "DodgerBlue3")\
        o(  743, 0x1C86EE, "DodgerBlue2")\
        o(  747, 0x104E8B, "DodgerBlue4")\
        o(  761, 0xFF7F24, "chocolate1")\
        o(  765, 0xCD661D, "chocolate3")\
        o(  767, 0xEE7621, "chocolate2")\
        o(  769, 0xFF6EB4, "HotPink1")\
        o(  771, 0x8B4513, "chocolate4")\
        o(  773, 0xCD6090, "HotPink3")\
        o(  775, 0xEE6AA7, "HotPink2")\
        o(  779, 0x8B3A62, "HotPink4")\
        o(  782, 0x0000FF, "blue")\
        o(  810, 0xF0FFF0, "honeydew")\
        o(  824, 0xB8860B, "dark goldenrod", "DarkGoldenrod")\
        o(  830, 0x191970, "midnight blue", "MidnightBlue")\
        o(  833, 0xFFA54F, "tan1")\
        o(  837, 0xCD853F, "tan3")\
        o(  839, 0xEE9A49, "tan2")\
        o(  843, 0x8B5A2B, "tan4")\
        o(  894, 0xA0522D, "sienna")\
        o(  905, 0xFFFFE0, "LightYellow1")\
        o(  909, 0xCDCDB4, "LightYellow3")\
        o(  911, 0xEEEED1, "LightYellow2")\
        o(  913, 0xBF3EFF, "DarkOrchid1")\
        o(  915, 0x8B8B7A, "LightYellow4")\
        o(  917, 0x9A32CD, "DarkOrchid3")\
        o(  919, 0xB23AEE, "DarkOrchid2")\
        o(  922, 0xFFF8DC, "cornsilk")\
        o(  923, 0x68228B, "DarkOrchid4")\
        o(  934, 0xB0E0E6, "powder blue", "PowderBlue")\
        o(  937, 0xFF4040, "brown1")\
        o(  938, 0xBC8F8F, "rosy brown", "RosyBrown")\
        o(  941, 0xCD3333, "brown3")\
        o(  943, 0xEE3B3B, "brown2")\
        o(  946, 0x00FA9A, "medium spring green", "MediumSpringGreen")\
        o(  947, 0x8B2323, "brown4")\
        o(  970, 0xFF8C00, "dark orange", "DarkOrange")\
        o(  990, 0x8470FF, "light slate blue", "LightSlateBlue")\
        o( 1009, 0xC1FFC1, "DarkSeaGreen1")\
        o( 1013, 0x9BCD9B, "DarkSeaGreen3")\
        o( 1015, 0xB4EEB4, "DarkSeaGreen2")\
        o( 1019, 0x698B69, "DarkSeaGreen4")\
        o( 1025, 0xFFEC8B, "LightGoldenrod1")\
        o( 1026, 0xE9967A, "dark salmon", "DarkSalmon")\
        o( 1029, 0xCDBE70, "LightGoldenrod3")\
        o( 1031, 0xEEDC82, "LightGoldenrod2")\
        o( 1035, 0x8B814C, "LightGoldenrod4")\
        o( 1086, 0xDCDCDC, "gainsboro")\
        o( 1090, 0xFAFAD2, "light goldenrod yellow", "LightGoldenrodYellow")\
        o( 1097, 0xFFF8DC, "cornsilk1")\
        o( 1101, 0xCDC8B1, "cornsilk3")\
        o( 1103, 0xEEE8CD, "cornsilk2")\
        o( 1107, 0x8B8878, "cornsilk4")\
        o( 1120, 0xDDA0DD, "plum")\
        o( 1137, 0xFFDAB9, "PeachPuff1")\
        o( 1141, 0xCDAF95, "PeachPuff3")\
        o( 1143, 0xEECBAD, "PeachPuff2")\
        o( 1147, 0x8B7765, "PeachPuff4")\
        o( 1161, 0xE066FF, "MediumOrchid1")\
        o( 1165, 0xB452CD, "MediumOrchid3")\
        o( 1167, 0xD15FEE, "MediumOrchid2")\
        o( 1171, 0x7A378B, "MediumOrchid4")\
        o( 1216, 0x8B0000, "dark red", "DarkRed")\
        o( 1222, 0xAFEEEE, "pale turquoise", "PaleTurquoise")\
        o( 1258, 0xFFFFE0, "light yellow", "LightYellow")\
        o( 1266, 0x9400D3, "dark violet", "DarkViolet")\
        o( 1345, 0xFFDEAD, "NavajoWhite1")\
        o( 1349, 0xCDB38B, "NavajoWhite3")\
        o( 1351, 0xEECFA1, "NavajoWhite2")\
        o( 1355, 0x8B795E, "NavajoWhite4")\
        o( 1368, 0xDB7093, "pale violet red", "PaleVioletRed")\
        o( 1375, 0xBDB76B, "dark khaki", "DarkKhaki")\
        o( 1385, 0xFFFF00, "yellow1")\
        o( 1386, 0xFFA07A, "light salmon", "LightSalmon")\
        o( 1389, 0xCDCD00, "yellow3")\
        o( 1391, 0xEEEE00, "yellow2")\
        o( 1395, 0x8B8B00, "yellow4")\
        o( 1409, 0xFF3030, "firebrick1")\
        o( 1413, 0xCD2626, "firebrick3")\
        o( 1415, 0xEE2C2C, "firebrick2")\
        o( 1419, 0x8B1A1A, "firebrick4")\
        o( 1449, 0x00FFFF, "cyan1")\
        o( 1453, 0x00CDCD, "cyan3")\
        o( 1455, 0x00EEEE, "cyan2")\
        o( 1459, 0x008B8B, "cyan4")\
        o( 1463, 0xE6E6FA, "lavender")\
        o( 1464, 0xD3D3D3, "light grey", "LightGrey", "light gray", "LightGray")\
        o( 1466, 0x00FF00, "green")\
        o( 1478, 0xF8F8FF, "ghost white", "GhostWhite")\
        o( 1482, 0x8FBC8F, "dark sea green", "DarkSeaGreen")\
        o( 1494, 0xFFE4E1, "misty rose", "MistyRose")\
        o( 1506, 0x3CB371, "medium sea green", "MediumSeaGreen")\
        o( 1513, 0xFF3E96, "VioletRed1")\
        o( 1514, 0xF4A460, "sandy brown", "SandyBrown")\
        o( 1517, 0xCD3278, "VioletRed3")\
        o( 1519, 0xEE3A8C, "VioletRed2")\
        o( 1523, 0x8B2252, "VioletRed4")\
        o( 1529, 0x0000FF, "blue1")\
        o( 1530, 0x000000, "black")\
        o( 1533, 0x0000CD, "blue3")\
        o( 1535, 0x0000EE, "blue2")\
        o( 1539, 0x00008B, "blue4")\
        o( 1552, 0xFFDAB9, "peach puff", "PeachPuff")\
        o( 1609, 0x4876FF, "RoyalBlue1")\
        o( 1613, 0x3A5FCD, "RoyalBlue3")\
        o( 1615, 0x436EEE, "RoyalBlue2")\
        o( 1619, 0x27408B, "RoyalBlue4")\
        o( 1630, 0xFAEBD7, "antique white", "AntiqueWhite")\
        o( 1638, 0x87CEFA, "light sky blue", "LightSkyBlue")\
        o( 1692, 0x9932CC, "dark orchid", "DarkOrchid")\
        o( 1697, 0xFF1493, "DeepPink1")\
        o( 1701, 0xCD1076, "DeepPink3")\
        o( 1703, 0xEE1289, "DeepPink2")\
        o( 1705, 0xFFE1FF, "thistle1")\
        o( 1707, 0x8B0A50, "DeepPink4")\
        o( 1709, 0xCDB5CD, "thistle3")\
        o( 1711, 0xEED2EE, "thistle2")\
        o( 1715, 0x8B7B8B, "thistle4")\
        o( 1741, 0x828282, "gray51", "grey51")\
        o( 1742, 0x949494, "gray58", "grey58")\
        o( 1745, 0x878787, "gray53", "grey53")\
        o( 1747, 0x858585, "gray52", "grey52")\
        o( 1748, 0x919191, "gray57", "grey57")\
        o( 1749, 0x969696, "gray59", "grey59")\
        o( 1751, 0x8A8A8A, "gray54", "grey54")\
        o( 1752, 0x8F8F8F, "gray56", "grey56")\
        o( 1754, 0x7F7F7F, "gray50", "grey50")\
        o( 1755, 0x8C8C8C, "gray55", "grey55")\
        o( 1773, 0x4F4F4F, "gray31", "grey31")\
        o( 1774, 0x616161, "gray38", "grey38")\
        o( 1777, 0x545454, "gray33", "grey33")\
        o( 1779, 0x525252, "gray32", "grey32")\
        o( 1780, 0x5E5E5E, "gray37", "grey37")\
        o( 1781, 0x636363, "gray39", "grey39")\
        o( 1783, 0x575757, "gray34", "grey34")\
        o( 1784, 0x5C5C5C, "gray36", "grey36")\
        o( 1786, 0x4D4D4D, "gray30", "grey30")\
        o( 1787, 0x595959, "gray35", "grey35")\
        o( 1814, 0x4682B4, "steel blue", "SteelBlue")\
        o( 1825, 0xFF4500, "OrangeRed1")\
        o( 1826, 0xFF1493, "deep pink", "DeepPink")\
        o( 1829, 0xCD3700, "OrangeRed3")\
        o( 1831, 0xEE4000, "OrangeRed2")\
        o( 1835, 0x8B2500, "OrangeRed4")\
        o( 1841, 0xF0FFFF, "azure1")\
        o( 1845, 0xC1CDCD, "azure3")\
        o( 1847, 0xE0EEEE, "azure2")\
        o( 1850, 0xFF6347, "tomato")\
        o( 1851, 0x838B8B, "azure4")\
        o( 1870, 0x48D1CC, "medium turquoise", "MediumTurquoise")\
        o( 1881, 0xFFAEB9, "LightPink1")\
        o( 1885, 0xCD8C95, "LightPink3")\
        o( 1886, 0xD8BFD8, "thistle")\
        o( 1887, 0xEEA2AD, "LightPink2")\
        o( 1891, 0x8B5F65, "LightPink4")\
        o( 1897, 0xFFF5EE, "seashell1")\
        o( 1901, 0xCDC5BF, "seashell3")\
        o( 1903, 0xEEE5DE, "seashell2")\
        o( 1907, 0x8B8682, "seashell4")\
        o( 1921, 0xFFA500, "orange1")\
        o( 1925, 0xCD8500, "orange3")\
        o( 1927, 0xEE9A00, "orange2")\
        o( 1931, 0x8B5A00, "orange4")\
        o( 1942, 0x00CED1, "dark turquoise", "DarkTurquoise")\
        o( 1958, 0xFFDEAD, "navajo white", "NavajoWhite")\
        o( 1962, 0xE0FFFF, "light cyan", "LightCyan")\
        o( 1978, 0xFFC0CB, "pink")\
        o( 1990, 0x483D8B, "dark slate blue", "DarkSlateBlue")\
        o( 1998, 0x66CDAA, "medium aquamarine", "MediumAquamarine")\
        o( 2006, 0xD2691E, "chocolate")\
        o( 2022, 0xFFFAF0, "floral white", "FloralWhite")\
        o( 2042, 0xFFFACD, "lemon chiffon", "LemonChiffon")\
        o( 2072, 0xFF0000, "red")\
        o( 2081, 0xFFF0F5, "LavenderBlush1")\
        o( 2085, 0xCDC1C5, "LavenderBlush3")\
        o( 2087, 0xEEE0E5, "LavenderBlush2")\
        o( 2091, 0x8B8386, "LavenderBlush4")\
        o( 2094, 0x000080, "navy blue", "NavyBlue")\
        o( 2110, 0xFFFFFF, "gray100", "grey100")\
        o( 2118, 0x87CEEB, "sky blue", "SkyBlue")\
        o( 2126, 0x00BFFF, "deep sky blue", "DeepSkyBlue")\
        o( 2185, 0xFF00FF, "magenta1")\
        o( 2189, 0xCD00CD, "magenta3")\
        o( 2191, 0xEE00EE, "magenta2")\
        o( 2195, 0x8B008B, "magenta4")\
        o( 2230, 0xB0C4DE, "light steel blue", "LightSteelBlue")\
        o( 2241, 0x9AFF9A, "PaleGreen1")\
        o( 2245, 0x7CCD7C, "PaleGreen3")\
        o( 2247, 0x90EE90, "PaleGreen2")\
        o( 2251, 0x548B54, "PaleGreen4")\
        o( 2252, 0xFFFFF0, "ivory")\
        o( 2265, 0xFFC125, "goldenrod1")\
        o( 2269, 0xCD9B1D, "goldenrod3")\
        o( 2271, 0xEEB422, "goldenrod2")\
        o( 2275, 0x8B6914, "goldenrod4")\
        o( 2278, 0x9370DB, "medium purple", "MediumPurple")\
        o( 2304, 0x696969, "dim gray", "DimGray", "dim grey", "DimGrey")\
        o( 2322, 0xFF69B4, "hot pink", "HotPink")\
        o( 2332, 0xBA55D3, "medium orchid", "MediumOrchid")\
        o( 2342, 0x1E90FF, "dodger blue", "DodgerBlue")\
        o( 2352, 0xA9A9A9, "dark grey", "DarkGrey", "dark gray", "DarkGray")\
        o( 2378, 0xFF7F50, "coral")\
        o( 2393, 0xFF6347, "tomato1")\
        o( 2397, 0xCD4F39, "tomato3")\
        o( 2399, 0xEE5C42, "tomato2")\
        o( 2403, 0x8B3626, "tomato4")\
        o( 2409, 0xFF82AB, "PaleVioletRed1")\
        o( 2413, 0xCD6889, "PaleVioletRed3")\
        o( 2415, 0xEE799F, "PaleVioletRed2")\
        o( 2419, 0x8B475D, "PaleVioletRed4")\
        o( 2425, 0xB0E2FF, "LightSkyBlue1")\
        o( 2429, 0x8DB6CD, "LightSkyBlue3")\
        o( 2431, 0xA4D3EE, "LightSkyBlue2")\
        o( 2432, 0xFF4500, "orange red", "OrangeRed")\
        o( 2435, 0x607B8B, "LightSkyBlue4")\
        o( 2457, 0xFF7F00, "DarkOrange1")\
        o( 2461, 0xCD6600, "DarkOrange3")\
        o( 2463, 0xEE7600, "DarkOrange2")\
        o( 2467, 0x8B4500, "DarkOrange4")\
        o( 2496, 0x708090, "slate gray", "SlateGray", "slate grey", "SlateGrey")\
        o( 2533, 0x363636, "gray21", "grey21")\
        o( 2534, 0x474747, "gray28", "grey28")\
        o( 2537, 0x3B3B3B, "gray23", "grey23")\
        o( 2539, 0x383838, "gray22", "grey22")\
        o( 2540, 0x454545, "gray27", "grey27")\
        o( 2541, 0x4A4A4A, "gray29", "grey29")\
        o( 2543, 0x3D3D3D, "gray24", "grey24")\
        o( 2544, 0x424242, "gray26", "grey26")\
        o( 2546, 0x333333, "gray20", "grey20")\
        o( 2547, 0x404040, "gray25", "grey25")\
        o( 2553, 0xFFFFF0, "ivory1")\
        o( 2557, 0xCDCDC1, "ivory3")\
        o( 2559, 0xEEEEE0, "ivory2")\
        o( 2563, 0x8B8B83, "ivory4")\
        o( 2569, 0xFF6A6A, "IndianRed1")\
        o( 2573, 0xCD5555, "IndianRed3")\
        o( 2575, 0xEE6363, "IndianRed2")\
        o( 2577, 0xCAFF70, "DarkOliveGreen1")\
        o( 2579, 0x8B3A3A, "IndianRed4")\
        o( 2581, 0xA2CD5A, "DarkOliveGreen3")\
        o( 2583, 0xBCEE68, "DarkOliveGreen2")\
        o( 2587, 0x6E8B3D, "DarkOliveGreen4")\
        o( 2617, 0xC0FF3E, "OliveDrab1")\
        o( 2621, 0x9ACD32, "OliveDrab3")\
        o( 2623, 0xB3EE3A, "OliveDrab2")\
        o( 2627, 0x698B22, "OliveDrab4")\
        o( 2675, 0xFFEFD5, "papaya whip", "PapayaWhip")\
        o( 2694, 0xFF00FF, "magenta")\
        o( 2697, 0x00F5FF, "turquoise1")\
        o( 2701, 0x00C5CD, "turquoise3")\
        o( 2703, 0x00E5EE, "turquoise2")\
        o( 2704, 0xBEBEBE, "gray", "grey")\
        o( 2707, 0x00868B, "turquoise4")\
        o( 2714, 0xF0FFFF, "azure")\
        o( 2737, 0xFF34B3, "maroon1")\
        o( 2741, 0xCD2990, "maroon3")\
        o( 2743, 0xEE30A7, "maroon2")\
        o( 2747, 0x8B1C62, "maroon4")\
        o( 2750, 0x5F9EA0, "cadet blue", "CadetBlue")\
        o( 2769, 0x00FF00, "green1")\
        o( 2773, 0x00CD00, "green3")\
        o( 2775, 0x00EE00, "green2")\
        o( 2777, 0xFFD700, "gold1")\
        o( 2779, 0x008B00, "green4")\
        o( 2781, 0xCDAD00, "gold3")\
        o( 2783, 0xEEC900, "gold2")\
        o( 2787, 0x8B7500, "gold4")\
        o( 2809, 0xC6E2FF, "SlateGray1")\
        o( 2813, 0x9FB6CD, "SlateGray3")\
        o( 2815, 0xB9D3EE, "SlateGray2")\
        o( 2819, 0x6C7B8B, "SlateGray4")\
        o( 2849, 0x9B30FF, "purple1")\
        o( 2850, 0x008B8B, "dark cyan", "DarkCyan")\
        o( 2853, 0x7D26CD, "purple3")\
        o( 2855, 0x912CEE, "purple2")\
        o( 2857, 0xFFE7BA, "wheat1")\
        o( 2859, 0x551A8B, "purple4")\
        o( 2861, 0xCDBA96, "wheat3")\
        o( 2863, 0xEED8AE, "wheat2")\
        o( 2867, 0x8B7E66, "wheat4")\
        o( 2871, 0xF0E68C, "khaki")\
        o( 2874, 0x20B2AA, "light sea green", "LightSeaGreen")\
        o( 2881, 0x00FF7F, "SpringGreen1")\
        o( 2882, 0x32CD32, "lime green", "LimeGreen")\
        o( 2885, 0x00CD66, "SpringGreen3")\
        o( 2886, 0x7FFF00, "chartreuse")\
        o( 2887, 0x00EE76, "SpringGreen2")\
        o( 2891, 0x008B45, "SpringGreen4")\
        o( 2912, 0x778899, "light slate gray", "LightSlateGray", "light slate grey", "LightSlateGrey")\
        o( 2913, 0xB5B5B5, "gray71", "grey71")\
        o( 2914, 0xC7C7C7, "gray78", "grey78")\
        o( 2917, 0xBABABA, "gray73", "grey73")\
        o( 2919, 0xB8B8B8, "gray72", "grey72")\
        o( 2920, 0xC4C4C4, "gray77", "grey77")\
        o( 2921, 0xC9C9C9, "gray79", "grey79")\
        o( 2923, 0xBDBDBD, "gray74", "grey74")\
        o( 2924, 0xC2C2C2, "gray76", "grey76")\
        o( 2926, 0xB3B3B3, "gray70", "grey70")\
        o( 2927, 0xBFBFBF, "gray75", "grey75")\
        o( 2966, 0x4169E1, "royal blue", "RoyalBlue")\
        o( 2985, 0xCAE1FF, "LightSteelBlue1")\
        o( 2989, 0xA2B5CD, "LightSteelBlue3")\
        o( 2991, 0xBCD2EE, "LightSteelBlue2")\
        o( 2995, 0x6E7B8B, "LightSteelBlue4")\
        o( 3024, 0xDEB887, "burlywood")\
        o( 3042, 0xCD853F, "peru")\
        o( 3048, 0xFFEBCD, "blanched almond", "BlanchedAlmond")\
        o( 3081, 0x97FFFF, "DarkSlateGray1")\
        o( 3085, 0x79CDCD, "DarkSlateGray3")\
        o( 3087, 0x8DEEEE, "DarkSlateGray2")\
        o( 3091, 0x528B8B, "DarkSlateGray4")\
        o( 3153, 0xFF8247, "sienna1")\
        o( 3157, 0xCD6839, "sienna3")\
        o( 3159, 0xEE7942, "sienna2")\
        o( 3163, 0x8B4726, "sienna4")\
        o( 3193, 0xFFC1C1, "RosyBrown1")\
        o( 3197, 0xCD9B9B, "RosyBrown3")\
        o( 3199, 0xEEB4B4, "RosyBrown2")\
        o( 3202, 0x00FFFF, "cyan")\
        o( 3203, 0x8B6969, "RosyBrown4")\
        o( 3206, 0xFFE4C4, "bisque")\
        o( 3210, 0xFAF0E6, "linen")\
        o( 3214, 0x6B8E23, "olive drab", "OliveDrab")\
        o( 3226, 0x228B22, "forest green", "ForestGreen")\
        o( 3230, 0x8B008B, "dark magenta", "DarkMagenta")\
        o( 3240, 0xDAA520, "goldenrod")\
        o( 3250, 0x556B2F, "dark olive green", "DarkOliveGreen")\
        o( 3265, 0x7FFFD4, "aquamarine1")\
        o( 3266, 0x7CFC00, "lawn green", "LawnGreen")\
        o( 3269, 0x66CDAA, "aquamarine3")\
        o( 3271, 0x76EEC6, "aquamarine2")\
        o( 3275, 0x458B74, "aquamarine4")\
        o( 3293, 0xE8E8E8, "gray91", "grey91")\
        o( 3294, 0xFAFAFA, "gray98", "grey98")\
        o( 3297, 0xEDEDED, "gray93", "grey93")\
        o( 3299, 0xEBEBEB, "gray92", "grey92")\
        o( 3300, 0xF7F7F7, "gray97", "grey97")\
        o( 3301, 0xFCFCFC, "gray99", "grey99")\
        o( 3303, 0xF0F0F0, "gray94", "grey94")\
        o( 3304, 0xF5F5F5, "gray96", "grey96")\
        o( 3306, 0xE5E5E5, "gray90", "grey90")\
        o( 3307, 0xF2F2F2, "gray95", "grey95")\
        o( 3326, 0xFFF0F5, "lavender blush", "LavenderBlush")\
        o( 3329, 0xE0FFFF, "LightCyan1")\
        o( 3333, 0xB4CDCD, "LightCyan3")\
        o( 3335, 0xD1EEEE, "LightCyan2")\
        o( 3339, 0x7A8B8B, "LightCyan4")\
        o( 3346, 0x90EE90, "light green", "LightGreen")\
        o( 3362, 0x00FF7F, "spring green", "SpringGreen")\
        o( 3374, 0xADD8E6, "light blue", "LightBlue")\
        o( 3409, 0xBFEFFF, "LightBlue1")\
        o( 3413, 0x9AC0CD, "LightBlue3")\
        o( 3414, 0xF5F5F5, "white smoke", "WhiteSmoke")\
        o( 3415, 0xB2DFEE, "LightBlue2")\
        o( 3419, 0x68838B, "LightBlue4")\
        o( 3425, 0x54FF9F, "SeaGreen1")\
        o( 3429, 0x43CD80, "SeaGreen3")\
        o( 3431, 0x4EEE94, "SeaGreen2")\
        o( 3434, 0xADFF2F, "green yellow", "GreenYellow")\
        o( 3435, 0x2E8B57, "SeaGreen4")\
        o( 3450, 0xFFF5EE, "seashell")\
        o( 3481, 0xFFE4E1, "MistyRose1")\
        o( 3485, 0xCDB7B5, "MistyRose3")\
        o( 3487, 0xEED5D2, "MistyRose2")\
        o( 3491, 0x8B7D7B, "MistyRose4")\
        o( 3506, 0xF5F5DC, "beige")\
        o( 3513, 0xFFBBFF, "plum1")\
        o( 3517, 0xCD96CD, "plum3")\
        o( 3519, 0xEEAEEE, "plum2")\
        o( 3523, 0x8B668B, "plum4")\
        o( 3558, 0x7FFFD4, "aquamarine")\
        o( 3561, 0xFFA07A, "LightSalmon1")\
        o( 3565, 0xCD8162, "LightSalmon3")\
        o( 3567, 0xEE9572, "LightSalmon2")\
        o( 3571, 0x8B5742, "LightSalmon4")\
        o( 3593, 0xBBFFFF, "PaleTurquoise1")\
        o( 3594, 0xF5DEB3, "wheat")\
        o( 3597, 0x96CDCD, "PaleTurquoise3")\
        o( 3599, 0xAEEEEE, "PaleTurquoise2")\
        o( 3602, 0xFFFF00, "yellow")\
        o( 3603, 0x668B8B, "PaleTurquoise4")\
        o( 3624, 0xCD5C5C, "indian red", "IndianRed")\
        o( 3633, 0xAB82FF, "MediumPurple1")\
        o( 3637, 0x8968CD, "MediumPurple3")\
        o( 3639, 0x9F79EE, "MediumPurple2")\
        o( 3643, 0x5D478B, "MediumPurple4")\
        o( 3656, 0xEEDD82, "light goldenrod", "LightGoldenrod")\
        o( 3674, 0xFFA500, "orange")\
        o( 3698, 0x98FB98, "pale green", "PaleGreen")\
        o( 3721, 0x87CEFF, "SkyBlue1")\
        o( 3725, 0x6CA6CD, "SkyBlue3")\
        o( 3726, 0xFFFFFF, "white")\
        o( 3727, 0x7EC0EE, "SkyBlue2")\
        o( 3730, 0xFA8072, "salmon")\
        o( 3731, 0x4A708B, "SkyBlue4")\
        o( 3745, 0x98F5FF, "CadetBlue1")\
        o( 3749, 0x7AC5CD, "CadetBlue3")\
        o( 3751, 0x8EE5EE, "CadetBlue2")\
        o( 3755, 0x53868B, "CadetBlue4")\
        o( 3794, 0x9ACD32, "yellow green", "YellowGreen")\
        o( 3802, 0x006400, "dark green", "DarkGreen")\
        o( 3809, 0x030303, "gray1", "grey1")\
        o( 3810, 0x141414, "gray8", "grey8")\
        o( 3813, 0x080808, "gray3", "grey3")\
        o( 3815, 0x050505, "gray2", "grey2")\
        o( 3816, 0x121212, "gray7", "grey7")\
        o( 3817, 0x171717, "gray9", "grey9")\
        o( 3818, 0xB22222, "firebrick")\
        o( 3819, 0x0A0A0A, "gray4", "grey4")\
        o( 3820, 0x0F0F0F, "gray6", "grey6")\
        o( 3822, 0x000000, "gray0", "grey0")\
        o( 3823, 0x0D0D0D, "gray5", "grey5")\

static std::pair<unsigned long long, unsigned> ParseHex(const char*& s)
{
    unsigned long long result = 0;
    unsigned length           = 0;
    while(*s != '\0' && *s != '/')
    {
        result <<= 4;
        if(*s >= 'A' && *s <= 'F')      result += *s - 'A' + 10;
        else if(*s >= 'a' && *s <= 'f') result += *s - 'a' + 10;
        else if(*s >= '0' && *s <= '9') result += *s - '0' +  0;
        /* Non-hex digits are treated as '0'.             */
        /* This matches the HTML color parsing algorithm. */
        ++s; ++length;
    }
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
        /* Note that these strings are case-sensitive. */
        if(std::memcmp(lc.data(), hextext, sizeof hextext) == 0) // shortest: "#FFF" = 4 characters
        {
            source = lc.data()+1;
            specs  = lsb_first;
        }
        else if(std::memcmp(lc.data(), rgbtext, sizeof rgbtext) == 0) // shortest: "rgb:F/F/F" = 9
        {
            source = lc.data()+4;
            specs  = use_separator;
        }
        else if(std::memcmp(lc.data(), rgbitext, sizeof rgbitext) == 0) // shortest: "rgbi:1/1/1" = 10
        {
            // Note: compares 5 letters while we only checked size >= 4.
            // If size == 4, there will be a nul, so this comparison is still safe,
            // assuming that the string view is nul terminated.
            source = lc.data()+5;
            specs  = use_separator | use_dbl;
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
static consteval auto GetColors(std::index_sequence<N...>)
{
    return std::array<unsigned, sizeof...(N)>
    {
        // Using a separate function for this rather than
        // inlining a long ?: chain makes compilation on GCC fast.
        // While Clang has no trouble with a long ?: chain, GCC would take ages to compile it.
        FindColor(N) ...
    };
}


/* See explanation in VerifyHash() */
template<std::size_t N>
struct fixed_string
{
    constexpr fixed_string(const char (&s)[N]) { std::copy_n(s, N, str); }
    consteval const char* data() const { return str; }
    consteval std::size_t size() const { return N-1; }
    char str[N];
};

/* VerifyHash()
 * Purpose:
 *
 * Uses static_assert(hash()) to verify that all of the given strings
 * match to the given hash value.
 *
 * Note: fixed_string as a template parameter is
 * a c++20 feature (class-type in non-type template parameter)
 * Minimum supported compiler version: GCC 9, Clang 12, MSVC 19.28
 * Not supported by Apple Clang yet.
 *
 * For concepts, you need GCC 10 or Clang 10.
 *
 * A fixed_string wrapper must be used, because a character array
 * is not a valid template parameter. Attempts to use "const char*"
 * or "char[]" will result in compilation errors.
 *
 * Similarly, attempts to pass the strings as parameters will fail,
 * such as this:
 *    template<unsigned expected_hash, std::size_t... N>
 *    static consteval void VerifyHash(const char (&...s)[N])
 * because the compiler says they are not constant expressions.
 *
 * The expected hash number must also be a template parameter.
 * Otherwise static_assert will fail saying that the number
 * is not a constant expression.
 * Even though the function is consteval, passing any of the
 * function parameters to static_assert() will fail the compilation.
 *
 * Then, static_assert is not an expression. It is a statement.
 * This would not compile:
 *   (static_assert(hash(s.data(), s.size()) == expected_hash), ...);
 * We could wrap the static_assert in a lambda, turning it into
 * an expression, but due to *two* bugs in GCC (PR99893 and PR99895),
 * it does not compile. Both would work on Clang though.
 */
template<unsigned expected_hash, fixed_string... s>
static constexpr inline void VerifyHash()
{
    // Make sure all the hashes are equal and match expected_hash
    static_assert( ((hash(s.data(), s.size()) == expected_hash) and ...), "Incorrect hash" );
}

unsigned ParseColorName(std::string_view s)
{
    /* First check if it's #ABC, #AABBCC, rgb:aa/bb/cc or rgbi:0.2/0.6/0.8 */
    auto [result,success] = TryParseColorNumber(s);
    if(success) return result;

    /* Nope? Then look it up in the hardcoded version of Xorg colors database. */

    /* Verify that all of the hashes in docolors() match the implementation in hash() */
    #define v(a,b,...) VerifyHash<a, __VA_ARGS__>();
    docolors(v);
    #undef v

    /* Begin by reducing the provided color into a small hash that is unique
     * for all colors in the database.
     */
    unsigned short h = hash(s.data(), s.size());

#if defined(__GNUC__) && !defined(__clang__)
    /* GCC generates pretty optimal code for this. */
    /* This produces an extra range check compared to what's below, but needs less data. */
    /* Amount of data: same as below, minus ~100 bytes by average */
    return FindColor(h);
#elif defined(__clang__) || defined(__GNUC__)
    /* Clang and GCC compile this well. No comparisons, just a singular data access. */
    /* Amount of data: mod*4 = ~15 kB */
    static constinit const auto colorvalues = GetColors(std::make_index_sequence<mod>());
    return colorvalues[h];
#else
    #define o(a,b,...) a,
    #define q(a,b,...) b,
    static constinit const unsigned short colorkeys[] { docolors(o) };
    static constinit const unsigned int colorvalues[] { docolors(q) };
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
