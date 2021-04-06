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

static constexpr unsigned mod = 3892u, mul = 388u;
// Note: Not using std::string_view, because clang falsely reports it's not a built-in-constant when it is.
static constexpr unsigned hash(const char* s, std::size_t length)
{
    constexpr std::uint_least64_t lett = 0x9389D9BA57C60; // maps a..z into 0..3 
    constexpr std::uint_least64_t map = 0x23EB769A5103C4; // maps everything into hex digits
    std::uint_least64_t result = 0xC901A36092EC4B7;
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
        o(   20, 0x6A5ACD, "slate blue", "SlateBlue")\
        o(   41, 0x4D4D4D, "gray30", "grey30")\
        o(   42, 0x636363, "gray39", "grey39")\
        o(   43, 0x616161, "gray38", "grey38")\
        o(   45, 0x4F4F4F, "gray31", "grey31")\
        o(   46, 0x575757, "gray34", "grey34")\
        o(   47, 0x595959, "gray35", "grey35")\
        o(   49, 0x545454, "gray33", "grey33")\
        o(   50, 0x525252, "gray32", "grey32")\
        o(   51, 0x5C5C5C, "gray36", "grey36")\
        o(   54, 0x5E5E5E, "gray37", "grey37")\
        o(  112, 0x0000FF, "blue")\
        o(  113, 0xE066FF, "MediumOrchid1")\
        o(  114, 0x7A378B, "MediumOrchid4")\
        o(  117, 0xB452CD, "MediumOrchid3")\
        o(  118, 0xD15FEE, "MediumOrchid2")\
        o(  120, 0x00CED1, "dark turquoise", "DarkTurquoise")\
        o(  124, 0x708090, "slate gray", "SlateGray", "slate grey", "SlateGrey")\
        o(  127, 0xE9967A, "dark salmon", "DarkSalmon")\
        o(  129, 0xF0FFFF, "azure1")\
        o(  130, 0x838B8B, "azure4")\
        o(  133, 0xC1CDCD, "azure3")\
        o(  134, 0xE0EEEE, "azure2")\
        o(  156, 0xFF0000, "red")\
        o(  157, 0xFFC125, "goldenrod1")\
        o(  158, 0x8B6914, "goldenrod4")\
        o(  161, 0xCD9B1D, "goldenrod3")\
        o(  162, 0xEEB422, "goldenrod2")\
        o(  172, 0xDDA0DD, "plum")\
        o(  197, 0xFF7F00, "DarkOrange1")\
        o(  198, 0x8B4500, "DarkOrange4")\
        o(  201, 0xCD6600, "DarkOrange3")\
        o(  202, 0xEE7600, "DarkOrange2")\
        o(  216, 0xBEBEBE, "gray", "grey")\
        o(  221, 0x836FFF, "SlateBlue1")\
        o(  222, 0x473C8B, "SlateBlue4")\
        o(  225, 0x6959CD, "SlateBlue3")\
        o(  226, 0x7A67EE, "SlateBlue2")\
        o(  233, 0xFFC1C1, "RosyBrown1")\
        o(  234, 0x8B6969, "RosyBrown4")\
        o(  237, 0xCD9B9B, "RosyBrown3")\
        o(  238, 0xEEB4B4, "RosyBrown2")\
        o(  247, 0xEE82EE, "violet")\
        o(  267, 0xFFC0CB, "pink")\
        o(  293, 0x54FF9F, "SeaGreen1")\
        o(  294, 0x2E8B57, "SeaGreen4")\
        o(  297, 0x43CD80, "SeaGreen3")\
        o(  298, 0x4EEE94, "SeaGreen2")\
        o(  307, 0xD2B48C, "tan")\
        o(  308, 0x4169E1, "royal blue", "RoyalBlue")\
        o(  325, 0xBFEFFF, "LightBlue1")\
        o(  326, 0x68838B, "LightBlue4")\
        o(  329, 0x9AC0CD, "LightBlue3")\
        o(  330, 0xB2DFEE, "LightBlue2")\
        o(  341, 0xFF8C69, "salmon1")\
        o(  342, 0x8B4C39, "salmon4")\
        o(  345, 0xCD7054, "salmon3")\
        o(  346, 0xEE8262, "salmon2")\
        o(  361, 0xFFFFFF, "gray100", "grey100")\
        o(  371, 0xFFF5EE, "seashell")\
        o(  376, 0x9370DB, "medium purple", "MediumPurple")\
        o(  412, 0x00BFFF, "deep sky blue", "DeepSkyBlue")\
        o(  423, 0xFF1493, "deep pink", "DeepPink")\
        o(  429, 0x333333, "gray20", "grey20")\
        o(  430, 0x4A4A4A, "gray29", "grey29")\
        o(  431, 0x474747, "gray28", "grey28")\
        o(  433, 0x363636, "gray21", "grey21")\
        o(  434, 0x3D3D3D, "gray24", "grey24")\
        o(  435, 0x404040, "gray25", "grey25")\
        o(  437, 0x3B3B3B, "gray23", "grey23")\
        o(  438, 0x383838, "gray22", "grey22")\
        o(  439, 0x424242, "gray26", "grey26")\
        o(  442, 0x454545, "gray27", "grey27")\
        o(  448, 0x7B68EE, "medium slate blue", "MediumSlateBlue")\
        o(  449, 0xFF6347, "tomato1")\
        o(  450, 0x8B3626, "tomato4")\
        o(  452, 0x8B0000, "dark red", "DarkRed")\
        o(  453, 0xCD4F39, "tomato3")\
        o(  454, 0xEE5C42, "tomato2")\
        o(  455, 0xFA8072, "salmon")\
        o(  456, 0x483D8B, "dark slate blue", "DarkSlateBlue")\
        o(  464, 0x87CEEB, "sky blue", "SkyBlue")\
        o(  468, 0xFFD700, "gold")\
        o(  541, 0xFF82AB, "PaleVioletRed1")\
        o(  542, 0x8B475D, "PaleVioletRed4")\
        o(  543, 0xBC8F8F, "rosy brown", "RosyBrown")\
        o(  545, 0xCD6889, "PaleVioletRed3")\
        o(  546, 0xEE799F, "PaleVioletRed2")\
        o(  549, 0x7FFF00, "chartreuse1")\
        o(  550, 0x458B00, "chartreuse4")\
        o(  553, 0x66CD00, "chartreuse3")\
        o(  554, 0x76EE00, "chartreuse2")\
        o(  560, 0x2F4F4F, "dark slate gray", "DarkSlateGray", "dark slate grey", "DarkSlateGrey")\
        o(  608, 0x000080, "navy blue", "NavyBlue")\
        o(  612, 0xFFFF00, "yellow")\
        o(  619, 0xB22222, "firebrick")\
        o(  627, 0x8A2BE2, "blue violet", "BlueViolet")\
        o(  633, 0xFFDEAD, "NavajoWhite1")\
        o(  634, 0x8B795E, "NavajoWhite4")\
        o(  637, 0xCDB38B, "NavajoWhite3")\
        o(  638, 0xEECFA1, "NavajoWhite2")\
        o(  644, 0x7FFFD4, "aquamarine")\
        o(  655, 0x00FFFF, "cyan")\
        o(  693, 0x1E90FF, "DodgerBlue1")\
        o(  694, 0x104E8B, "DodgerBlue4")\
        o(  697, 0x1874CD, "DodgerBlue3")\
        o(  698, 0x1C86EE, "DodgerBlue2")\
        o(  723, 0x7CFC00, "lawn green", "LawnGreen")\
        o(  728, 0xFFE4E1, "misty rose", "MistyRose")\
        o(  729, 0xC1FFC1, "DarkSeaGreen1")\
        o(  730, 0x698B69, "DarkSeaGreen4")\
        o(  733, 0x9BCD9B, "DarkSeaGreen3")\
        o(  734, 0xB4EEB4, "DarkSeaGreen2")\
        o(  753, 0x000000, "gray0", "grey0")\
        o(  754, 0x171717, "gray9", "grey9")\
        o(  755, 0x141414, "gray8", "grey8")\
        o(  757, 0x030303, "gray1", "grey1")\
        o(  758, 0x0A0A0A, "gray4", "grey4")\
        o(  759, 0x0D0D0D, "gray5", "grey5")\
        o(  761, 0x080808, "gray3", "grey3")\
        o(  762, 0x050505, "gray2", "grey2")\
        o(  763, 0x0F0F0F, "gray6", "grey6")\
        o(  766, 0x121212, "gray7", "grey7")\
        o(  775, 0xF5DEB3, "wheat")\
        o(  788, 0xA0522D, "sienna")\
        o(  792, 0xA020F0, "purple")\
        o(  804, 0xADD8E6, "light blue", "LightBlue")\
        o(  812, 0x7FFF00, "chartreuse")\
        o(  815, 0xFFFACD, "lemon chiffon", "LemonChiffon")\
        o(  817, 0x999999, "gray60", "grey60")\
        o(  818, 0xB0B0B0, "gray69", "grey69")\
        o(  819, 0xADADAD, "gray68", "grey68")\
        o(  821, 0x9C9C9C, "gray61", "grey61")\
        o(  822, 0xA3A3A3, "gray64", "grey64")\
        o(  823, 0xA6A6A6, "gray65", "grey65")\
        o(  825, 0xA1A1A1, "gray63", "grey63")\
        o(  826, 0x9E9E9E, "gray62", "grey62")\
        o(  827, 0xA8A8A8, "gray66", "grey66")\
        o(  830, 0xABABAB, "gray67", "grey67")\
        o(  841, 0xE0FFFF, "LightCyan1")\
        o(  842, 0x7A8B8B, "LightCyan4")\
        o(  845, 0xB4CDCD, "LightCyan3")\
        o(  846, 0xD1EEEE, "LightCyan2")\
        o(  896, 0xDCDCDC, "gainsboro")\
        o(  908, 0xD3D3D3, "light grey", "LightGrey", "light gray", "LightGray")\
        o(  920, 0xFFF0F5, "lavender blush", "LavenderBlush")\
        o(  944, 0xFAEBD7, "antique white", "AntiqueWhite")\
        o(  948, 0x00008B, "dark blue", "DarkBlue")\
        o(  952, 0xFFFAF0, "floral white", "FloralWhite")\
        o(  953, 0xBF3EFF, "DarkOrchid1")\
        o(  954, 0x68228B, "DarkOrchid4")\
        o(  957, 0x9A32CD, "DarkOrchid3")\
        o(  958, 0xB23AEE, "DarkOrchid2")\
        o(  959, 0xFFB6C1, "light pink", "LightPink")\
        o(  980, 0x6B8E23, "olive drab", "OliveDrab")\
        o( 1025, 0xFFB90F, "DarkGoldenrod1")\
        o( 1026, 0x8B6508, "DarkGoldenrod4")\
        o( 1029, 0xCD950C, "DarkGoldenrod3")\
        o( 1030, 0xEEAD0E, "DarkGoldenrod2")\
        o( 1041, 0xFFA07A, "LightSalmon1")\
        o( 1042, 0x8B5742, "LightSalmon4")\
        o( 1045, 0xCD8162, "LightSalmon3")\
        o( 1046, 0xEE9572, "LightSalmon2")\
        o( 1052, 0xA9A9A9, "dark grey", "DarkGrey", "dark gray", "DarkGray")\
        o( 1085, 0xFFB5C5, "pink1")\
        o( 1086, 0x8B636C, "pink4")\
        o( 1089, 0xCD919E, "pink3")\
        o( 1090, 0xEEA9B8, "pink2")\
        o( 1109, 0xFF8247, "sienna1")\
        o( 1110, 0x8B4726, "sienna4")\
        o( 1113, 0xCD6839, "sienna3")\
        o( 1114, 0xEE7942, "sienna2")\
        o( 1164, 0x87CEFA, "light sky blue", "LightSkyBlue")\
        o( 1173, 0x98F5FF, "CadetBlue1")\
        o( 1174, 0x53868B, "CadetBlue4")\
        o( 1177, 0x7AC5CD, "CadetBlue3")\
        o( 1178, 0x8EE5EE, "CadetBlue2")\
        o( 1181, 0x9AFF9A, "PaleGreen1")\
        o( 1182, 0x548B54, "PaleGreen4")\
        o( 1185, 0x7CCD7C, "PaleGreen3")\
        o( 1186, 0x90EE90, "PaleGreen2")\
        o( 1197, 0xFFF5EE, "seashell1")\
        o( 1198, 0x8B8682, "seashell4")\
        o( 1201, 0xCDC5BF, "seashell3")\
        o( 1202, 0xEEE5DE, "seashell2")\
        o( 1217, 0xE5E5E5, "gray90", "grey90")\
        o( 1218, 0xFCFCFC, "gray99", "grey99")\
        o( 1219, 0xFAFAFA, "gray98", "grey98")\
        o( 1221, 0xE8E8E8, "gray91", "grey91")\
        o( 1222, 0xF0F0F0, "gray94", "grey94")\
        o( 1223, 0xF2F2F2, "gray95", "grey95")\
        o( 1225, 0xEDEDED, "gray93", "grey93")\
        o( 1226, 0xEBEBEB, "gray92", "grey92")\
        o( 1227, 0xF5F5F5, "gray96", "grey96")\
        o( 1230, 0xF7F7F7, "gray97", "grey97")\
        o( 1233, 0xFFD700, "gold1")\
        o( 1234, 0x8B7500, "gold4")\
        o( 1237, 0xCDAD00, "gold3")\
        o( 1238, 0xEEC900, "gold2")\
        o( 1268, 0xD02090, "violet red", "VioletRed")\
        o( 1272, 0xCD853F, "peru")\
        o( 1311, 0xFF7F50, "coral")\
        o( 1312, 0xE6E6FA, "lavender")\
        o( 1339, 0x9ACD32, "yellow green", "YellowGreen")\
        o( 1347, 0xE0FFFF, "light cyan", "LightCyan")\
        o( 1361, 0xFFFFF0, "ivory1")\
        o( 1362, 0x8B8B83, "ivory4")\
        o( 1365, 0xCDCDC1, "ivory3")\
        o( 1366, 0xEEEEE0, "ivory2")\
        o( 1369, 0x00FF7F, "SpringGreen1")\
        o( 1370, 0x008B45, "SpringGreen4")\
        o( 1373, 0x00CD66, "SpringGreen3")\
        o( 1374, 0x00EE76, "SpringGreen2")\
        o( 1380, 0xADFF2F, "green yellow", "GreenYellow")\
        o( 1385, 0xFFE4C4, "bisque1")\
        o( 1386, 0x8B7D6B, "bisque4")\
        o( 1389, 0xCDB79E, "bisque3")\
        o( 1390, 0xEED5B7, "bisque2")\
        o( 1412, 0x6495ED, "cornflower blue", "CornflowerBlue")\
        o( 1416, 0xB0E0E6, "powder blue", "PowderBlue")\
        o( 1457, 0x00BFFF, "DeepSkyBlue1")\
        o( 1458, 0x00688B, "DeepSkyBlue4")\
        o( 1461, 0x009ACD, "DeepSkyBlue3")\
        o( 1462, 0x00B2EE, "DeepSkyBlue2")\
        o( 1471, 0xFAF0E6, "linen")\
        o( 1488, 0xFF8C00, "dark orange", "DarkOrange")\
        o( 1491, 0x008B8B, "dark cyan", "DarkCyan")\
        o( 1492, 0xFFDAB9, "peach puff", "PeachPuff")\
        o( 1496, 0x5F9EA0, "cadet blue", "CadetBlue")\
        o( 1497, 0x00F5FF, "turquoise1")\
        o( 1498, 0x00868B, "turquoise4")\
        o( 1501, 0x00C5CD, "turquoise3")\
        o( 1502, 0x00E5EE, "turquoise2")\
        o( 1504, 0xF5F5F5, "white smoke", "WhiteSmoke")\
        o( 1543, 0x00FF00, "green")\
        o( 1545, 0xCAFF70, "DarkOliveGreen1")\
        o( 1546, 0x6E8B3D, "DarkOliveGreen4")\
        o( 1549, 0xA2CD5A, "DarkOliveGreen3")\
        o( 1550, 0xBCEE68, "DarkOliveGreen2")\
        o( 1555, 0x006400, "dark green", "DarkGreen")\
        o( 1559, 0x228B22, "forest green", "ForestGreen")\
        o( 1565, 0xFF34B3, "maroon1")\
        o( 1566, 0x8B1C62, "maroon4")\
        o( 1569, 0xCD2990, "maroon3")\
        o( 1570, 0xEE30A7, "maroon2")\
        o( 1572, 0xFFDEAD, "navajo white", "NavajoWhite")\
        o( 1579, 0x3CB371, "medium sea green", "MediumSeaGreen")\
        o( 1585, 0xFFE7BA, "wheat1")\
        o( 1586, 0x8B7E66, "wheat4")\
        o( 1589, 0xCDBA96, "wheat3")\
        o( 1590, 0xEED8AE, "wheat2")\
        o( 1596, 0xFF00FF, "magenta")\
        o( 1604, 0xFFEBCD, "blanched almond", "BlanchedAlmond")\
        o( 1605, 0xCCCCCC, "gray80", "grey80")\
        o( 1606, 0xE3E3E3, "gray89", "grey89")\
        o( 1607, 0xE0E0E0, "gray88", "grey88")\
        o( 1609, 0xCFCFCF, "gray81", "grey81")\
        o( 1610, 0xD6D6D6, "gray84", "grey84")\
        o( 1611, 0xD9D9D9, "gray85", "grey85")\
        o( 1613, 0xD4D4D4, "gray83", "grey83")\
        o( 1614, 0xD1D1D1, "gray82", "grey82")\
        o( 1615, 0xDBDBDB, "gray86", "grey86")\
        o( 1618, 0xDEDEDE, "gray87", "grey87")\
        o( 1653, 0xC6E2FF, "SlateGray1")\
        o( 1654, 0x6C7B8B, "SlateGray4")\
        o( 1657, 0x9FB6CD, "SlateGray3")\
        o( 1658, 0xB9D3EE, "SlateGray2")\
        o( 1660, 0xD70751, "DebianRed")\
        o( 1664, 0xF0FFFF, "azure")\
        o( 1696, 0xC71585, "medium violet red", "MediumVioletRed")\
        o( 1723, 0x32CD32, "lime green", "LimeGreen")\
        o( 1729, 0xFF3E96, "VioletRed1")\
        o( 1730, 0x8B2252, "VioletRed4")\
        o( 1733, 0xCD3278, "VioletRed3")\
        o( 1734, 0xEE3A8C, "VioletRed2")\
        o( 1737, 0xFFF8DC, "cornsilk1")\
        o( 1738, 0x8B8878, "cornsilk4")\
        o( 1741, 0xCDC8B1, "cornsilk3")\
        o( 1742, 0xEEE8CD, "cornsilk2")\
        o( 1761, 0xFF6A6A, "IndianRed1")\
        o( 1762, 0x8B3A3A, "IndianRed4")\
        o( 1765, 0xCD5555, "IndianRed3")\
        o( 1766, 0xEE6363, "IndianRed2")\
        o( 1805, 0xFFFACD, "LemonChiffon1")\
        o( 1806, 0x8B8970, "LemonChiffon4")\
        o( 1809, 0xCDC9A5, "LemonChiffon3")\
        o( 1810, 0xEEE9BF, "LemonChiffon2")\
        o( 1816, 0xFFA500, "orange")\
        o( 1852, 0xFF4500, "orange red", "OrangeRed")\
        o( 1875, 0x00FA9A, "medium spring green", "MediumSpringGreen")\
        o( 1901, 0xCAE1FF, "LightSteelBlue1")\
        o( 1902, 0x6E7B8B, "LightSteelBlue4")\
        o( 1905, 0xA2B5CD, "LightSteelBlue3")\
        o( 1906, 0xBCD2EE, "LightSteelBlue2")\
        o( 1913, 0xFF1493, "DeepPink1")\
        o( 1914, 0x8B0A50, "DeepPink4")\
        o( 1917, 0xCD1076, "DeepPink3")\
        o( 1918, 0xEE1289, "DeepPink2")\
        o( 1956, 0xEEDD82, "light goldenrod", "LightGoldenrod")\
        o( 1957, 0xFFE4E1, "MistyRose1")\
        o( 1958, 0x8B7D7B, "MistyRose4")\
        o( 1961, 0xCDB7B5, "MistyRose3")\
        o( 1962, 0xEED5D2, "MistyRose2")\
        o( 1968, 0xDB7093, "pale violet red", "PaleVioletRed")\
        o( 1980, 0xCD5C5C, "indian red", "IndianRed")\
        o( 1981, 0xB3B3B3, "gray70", "grey70")\
        o( 1982, 0xC9C9C9, "gray79", "grey79")\
        o( 1983, 0xC7C7C7, "gray78", "grey78")\
        o( 1985, 0xB5B5B5, "gray71", "grey71")\
        o( 1986, 0xBDBDBD, "gray74", "grey74")\
        o( 1987, 0xBFBFBF, "gray75", "grey75")\
        o( 1989, 0xBABABA, "gray73", "grey73")\
        o( 1990, 0xB8B8B8, "gray72", "grey72")\
        o( 1991, 0xC2C2C2, "gray76", "grey76")\
        o( 1994, 0xC4C4C4, "gray77", "grey77")\
        o( 2027, 0xFFF8DC, "cornsilk")\
        o( 2036, 0xD8BFD8, "thistle")\
        o( 2073, 0xFF3030, "firebrick1")\
        o( 2074, 0x8B1A1A, "firebrick4")\
        o( 2077, 0xCD2626, "firebrick3")\
        o( 2078, 0xEE2C2C, "firebrick2")\
        o( 2085, 0xFFAEB9, "LightPink1")\
        o( 2086, 0x8B5F65, "LightPink4")\
        o( 2089, 0xCD8C95, "LightPink3")\
        o( 2090, 0xEEA2AD, "LightPink2")\
        o( 2112, 0xFFFFFF, "white")\
        o( 2145, 0xFF7F24, "chocolate1")\
        o( 2146, 0x8B4513, "chocolate4")\
        o( 2149, 0xCD661D, "chocolate3")\
        o( 2150, 0xEE7621, "chocolate2")\
        o( 2173, 0xFFF0F5, "LavenderBlush1")\
        o( 2174, 0x8B8386, "LavenderBlush4")\
        o( 2177, 0xCDC1C5, "LavenderBlush3")\
        o( 2178, 0xEEE0E5, "LavenderBlush2")\
        o( 2208, 0xDEB887, "burlywood")\
        o( 2227, 0x8B4513, "saddle brown", "SaddleBrown")\
        o( 2229, 0xFF0000, "red1")\
        o( 2230, 0x8B0000, "red4")\
        o( 2233, 0xCD0000, "red3")\
        o( 2234, 0xEE0000, "red2")\
        o( 2311, 0xF08080, "light coral", "LightCoral")\
        o( 2363, 0xFFA07A, "light salmon", "LightSalmon")\
        o( 2381, 0x1A1A1A, "gray10", "grey10")\
        o( 2382, 0x303030, "gray19", "grey19")\
        o( 2383, 0x2E2E2E, "gray18", "grey18")\
        o( 2385, 0x1C1C1C, "gray11", "grey11")\
        o( 2386, 0x242424, "gray14", "grey14")\
        o( 2387, 0x262626, "gray15", "grey15")\
        o( 2389, 0x212121, "gray13", "grey13")\
        o( 2390, 0x1F1F1F, "gray12", "grey12")\
        o( 2391, 0x292929, "gray16", "grey16")\
        o( 2394, 0x2B2B2B, "gray17", "grey17")\
        o( 2437, 0xFFA54F, "tan1")\
        o( 2438, 0x8B5A2B, "tan4")\
        o( 2441, 0xCD853F, "tan3")\
        o( 2442, 0xEE9A49, "tan2")\
        o( 2444, 0x000080, "navy")\
        o( 2461, 0x4876FF, "RoyalBlue1")\
        o( 2462, 0x27408B, "RoyalBlue4")\
        o( 2465, 0x3A5FCD, "RoyalBlue3")\
        o( 2466, 0x436EEE, "RoyalBlue2")\
        o( 2468, 0x1E90FF, "dodger blue", "DodgerBlue")\
        o( 2479, 0xF4A460, "sandy brown", "SandyBrown")\
        o( 2500, 0xF0FFF0, "honeydew")\
        o( 2504, 0xFFE4C4, "bisque")\
        o( 2520, 0xFFFFE0, "light yellow", "LightYellow")\
        o( 2521, 0x97FFFF, "DarkSlateGray1")\
        o( 2522, 0x528B8B, "DarkSlateGray4")\
        o( 2525, 0x79CDCD, "DarkSlateGray3")\
        o( 2526, 0x8DEEEE, "DarkSlateGray2")\
        o( 2531, 0x556B2F, "dark olive green", "DarkOliveGreen")\
        o( 2540, 0xDAA520, "goldenrod")\
        o( 2541, 0x63B8FF, "SteelBlue1")\
        o( 2542, 0x36648B, "SteelBlue4")\
        o( 2543, 0x90EE90, "light green", "LightGreen")\
        o( 2545, 0x4F94CD, "SteelBlue3")\
        o( 2546, 0x5CACEE, "SteelBlue2")\
        o( 2549, 0xF0FFF0, "honeydew1")\
        o( 2550, 0x838B83, "honeydew4")\
        o( 2553, 0xC1CDC1, "honeydew3")\
        o( 2554, 0xE0EEE0, "honeydew2")\
        o( 2583, 0xFFEFD5, "papaya whip", "PapayaWhip")\
        o( 2661, 0x9B30FF, "purple1")\
        o( 2662, 0x551A8B, "purple4")\
        o( 2665, 0x7D26CD, "purple3")\
        o( 2666, 0x912CEE, "purple2")\
        o( 2679, 0xFF69B4, "hot pink", "HotPink")\
        o( 2692, 0xB0C4DE, "light steel blue", "LightSteelBlue")\
        o( 2696, 0x8B008B, "dark magenta", "DarkMagenta")\
        o( 2700, 0xFFFAFA, "snow")\
        o( 2716, 0xBA55D3, "medium orchid", "MediumOrchid")\
        o( 2721, 0xFFE1FF, "thistle1")\
        o( 2722, 0x8B7B8B, "thistle4")\
        o( 2725, 0xCDB5CD, "thistle3")\
        o( 2726, 0xEED2EE, "thistle2")\
        o( 2769, 0x666666, "gray40", "grey40")\
        o( 2770, 0x7D7D7D, "gray49", "grey49")\
        o( 2771, 0x7A7A7A, "gray48", "grey48")\
        o( 2773, 0x696969, "gray41", "grey41")\
        o( 2774, 0x707070, "gray44", "grey44")\
        o( 2775, 0x737373, "gray45", "grey45")\
        o( 2777, 0x6E6E6E, "gray43", "grey43")\
        o( 2778, 0x6B6B6B, "gray42", "grey42")\
        o( 2779, 0x757575, "gray46", "grey46")\
        o( 2782, 0x787878, "gray47", "grey47")\
        o( 2788, 0xFDF5E6, "old lace", "OldLace")\
        o( 2804, 0x9932CC, "dark orchid", "DarkOrchid")\
        o( 2827, 0x000000, "black")\
        o( 2845, 0xFF00FF, "magenta1")\
        o( 2846, 0x8B008B, "magenta4")\
        o( 2849, 0xCD00CD, "magenta3")\
        o( 2850, 0xEE00EE, "magenta2")\
        o( 2877, 0xFFFF00, "yellow1")\
        o( 2878, 0x8B8B00, "yellow4")\
        o( 2881, 0xCDCD00, "yellow3")\
        o( 2882, 0xEEEE00, "yellow2")\
        o( 2921, 0xAB82FF, "MediumPurple1")\
        o( 2922, 0x5D478B, "MediumPurple4")\
        o( 2924, 0x191970, "midnight blue", "MidnightBlue")\
        o( 2925, 0x8968CD, "MediumPurple3")\
        o( 2926, 0x9F79EE, "MediumPurple2")\
        o( 2961, 0xC0FF3E, "OliveDrab1")\
        o( 2962, 0x698B22, "OliveDrab4")\
        o( 2965, 0x9ACD32, "OliveDrab3")\
        o( 2966, 0xB3EE3A, "OliveDrab2")\
        o( 2988, 0xF8F8FF, "ghost white", "GhostWhite")\
        o( 2989, 0xFFA500, "orange1")\
        o( 2990, 0x8B5A00, "orange4")\
        o( 2993, 0xCD8500, "orange3")\
        o( 2994, 0xEE9A00, "orange2")\
        o( 3111, 0x98FB98, "pale green", "PaleGreen")\
        o( 3117, 0xFF6EB4, "HotPink1")\
        o( 3118, 0x8B3A62, "HotPink4")\
        o( 3121, 0xCD6090, "HotPink3")\
        o( 3122, 0xEE6AA7, "HotPink2")\
        o( 3127, 0xFFE4B5, "moccasin")\
        o( 3132, 0xDA70D6, "orchid")\
        o( 3145, 0xFFBBFF, "plum1")\
        o( 3146, 0x8B668B, "plum4")\
        o( 3149, 0xCD96CD, "plum3")\
        o( 3150, 0xEEAEEE, "plum2")\
        o( 3157, 0x7F7F7F, "gray50", "grey50")\
        o( 3158, 0x969696, "gray59", "grey59")\
        o( 3159, 0x949494, "gray58", "grey58")\
        o( 3161, 0x828282, "gray51", "grey51")\
        o( 3162, 0x8A8A8A, "gray54", "grey54")\
        o( 3163, 0x8C8C8C, "gray55", "grey55")\
        o( 3165, 0x878787, "gray53", "grey53")\
        o( 3166, 0x858585, "gray52", "grey52")\
        o( 3167, 0x8F8F8F, "gray56", "grey56")\
        o( 3170, 0x919191, "gray57", "grey57")\
        o( 3172, 0x40E0D0, "turquoise")\
        o( 3176, 0x696969, "dim gray", "DimGray", "dim grey", "DimGrey")\
        o( 3192, 0x0000CD, "medium blue", "MediumBlue")\
        o( 3217, 0x0000FF, "blue1")\
        o( 3218, 0x00008B, "blue4")\
        o( 3221, 0x0000CD, "blue3")\
        o( 3222, 0x0000EE, "blue2")\
        o( 3225, 0xFFFAFA, "snow1")\
        o( 3226, 0x8B8989, "snow4")\
        o( 3229, 0xCDC9C9, "snow3")\
        o( 3230, 0xEEE9E9, "snow2")\
        o( 3232, 0xFFFFF0, "ivory")\
        o( 3240, 0xEEE8AA, "pale goldenrod", "PaleGoldenrod")\
        o( 3276, 0x4682B4, "steel blue", "SteelBlue")\
        o( 3277, 0xFF7256, "coral1")\
        o( 3278, 0x8B3E2F, "coral4")\
        o( 3281, 0xCD5B45, "coral3")\
        o( 3282, 0xEE6A50, "coral2")\
        o( 3284, 0xFF6347, "tomato")\
        o( 3324, 0xF5F5DC, "beige")\
        o( 3327, 0xB03060, "maroon")\
        o( 3328, 0x8470FF, "light slate blue", "LightSlateBlue")\
        o( 3380, 0xB8860B, "dark goldenrod", "DarkGoldenrod")\
        o( 3409, 0xFFEC8B, "LightGoldenrod1")\
        o( 3410, 0x8B814C, "LightGoldenrod4")\
        o( 3413, 0xCDBE70, "LightGoldenrod3")\
        o( 3414, 0xEEDC82, "LightGoldenrod2")\
        o( 3419, 0x00FF7F, "spring green", "SpringGreen")\
        o( 3425, 0x7FFFD4, "aquamarine1")\
        o( 3426, 0x458B74, "aquamarine4")\
        o( 3429, 0x66CDAA, "aquamarine3")\
        o( 3430, 0x76EEC6, "aquamarine2")\
        o( 3432, 0x778899, "light slate gray", "LightSlateGray", "light slate grey", "LightSlateGrey")\
        o( 3464, 0xFAFAD2, "light goldenrod yellow", "LightGoldenrodYellow")\
        o( 3488, 0xF5FFFA, "mint cream", "MintCream")\
        o( 3513, 0xFFDAB9, "PeachPuff1")\
        o( 3514, 0x8B7765, "PeachPuff4")\
        o( 3516, 0xD2691E, "chocolate")\
        o( 3517, 0xCDAF95, "PeachPuff3")\
        o( 3518, 0xEECBAD, "PeachPuff2")\
        o( 3552, 0xF0F8FF, "alice blue", "AliceBlue")\
        o( 3577, 0xFFFFE0, "LightYellow1")\
        o( 3578, 0x8B8B7A, "LightYellow4")\
        o( 3579, 0xA52A2A, "brown")\
        o( 3581, 0xCDCDB4, "LightYellow3")\
        o( 3582, 0xEEEED1, "LightYellow2")\
        o( 3600, 0x48D1CC, "medium turquoise", "MediumTurquoise")\
        o( 3627, 0x8FBC8F, "dark sea green", "DarkSeaGreen")\
        o( 3669, 0xFF4040, "brown1")\
        o( 3670, 0x8B2323, "brown4")\
        o( 3673, 0xCD3333, "brown3")\
        o( 3674, 0xEE3B3B, "brown2")\
        o( 3697, 0xFFD39B, "burlywood1")\
        o( 3698, 0x8B7355, "burlywood4")\
        o( 3701, 0xCDAA7D, "burlywood3")\
        o( 3702, 0xEEC591, "burlywood2")\
        o( 3704, 0x66CDAA, "medium aquamarine", "MediumAquamarine")\
        o( 3719, 0x2E8B57, "sea green", "SeaGreen")\
        o( 3725, 0xFFEFDB, "AntiqueWhite1")\
        o( 3726, 0x8B8378, "AntiqueWhite4")\
        o( 3729, 0xCDC0B0, "AntiqueWhite3")\
        o( 3730, 0xEEDFCC, "AntiqueWhite2")\
        o( 3733, 0x00FFFF, "cyan1")\
        o( 3734, 0x008B8B, "cyan4")\
        o( 3737, 0x00CDCD, "cyan3")\
        o( 3738, 0x00EEEE, "cyan2")\
        o( 3740, 0xF0E68C, "khaki")\
        o( 3743, 0x20B2AA, "light sea green", "LightSeaGreen")\
        o( 3745, 0xFF83FA, "orchid1")\
        o( 3746, 0x8B4789, "orchid4")\
        o( 3749, 0xCD69C9, "orchid3")\
        o( 3750, 0xEE7AE9, "orchid2")\
        o( 3752, 0xBDB76B, "dark khaki", "DarkKhaki")\
        o( 3777, 0x00FF00, "green1")\
        o( 3778, 0x008B00, "green4")\
        o( 3781, 0x00CD00, "green3")\
        o( 3782, 0x00EE00, "green2")\
        o( 3785, 0xFF4500, "OrangeRed1")\
        o( 3786, 0x8B2500, "OrangeRed4")\
        o( 3789, 0xCD3700, "OrangeRed3")\
        o( 3790, 0xEE4000, "OrangeRed2")\
        o( 3797, 0xBBFFFF, "PaleTurquoise1")\
        o( 3798, 0x668B8B, "PaleTurquoise4")\
        o( 3801, 0x96CDCD, "PaleTurquoise3")\
        o( 3802, 0xAEEEEE, "PaleTurquoise2")\
        o( 3811, 0x9400D3, "dark violet", "DarkViolet")\
        o( 3829, 0x87CEFF, "SkyBlue1")\
        o( 3830, 0x4A708B, "SkyBlue4")\
        o( 3833, 0x6CA6CD, "SkyBlue3")\
        o( 3834, 0x7EC0EE, "SkyBlue2")\
        o( 3853, 0xB0E2FF, "LightSkyBlue1")\
        o( 3854, 0x607B8B, "LightSkyBlue4")\
        o( 3857, 0x8DB6CD, "LightSkyBlue3")\
        o( 3858, 0xA4D3EE, "LightSkyBlue2")\
        o( 3865, 0xFFF68F, "khaki1")\
        o( 3866, 0x8B864E, "khaki4")\
        o( 3869, 0xCDC673, "khaki3")\
        o( 3870, 0xEEE685, "khaki2")\
        o( 3872, 0xAFEEEE, "pale turquoise", "PaleTurquoise")\

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
