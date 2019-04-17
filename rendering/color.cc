#include <string>
#include <cstdlib>
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

static constexpr std::uint_least64_t a = 0x28017C849DA5FA1C;
static constexpr std::uint_least64_t b = 0x22AFE2F3FB9AEB4F;
static constexpr std::uint_least64_t c = 0xD037EA92DF77F67C;
static constexpr std::uint_least64_t d = 0x9B774A882348672;
static constexpr unsigned mod          = 8190;

static constexpr unsigned short hash(const char* s, std::size_t length)
{
    std::uint_least64_t result = a;
    while(length--)
    {
        result = (result ^ (result >> 33)) * b;
        result = (result ^ (result >> 33)) * c;
        result = (result ^ (result >> 33)) * d;
        result += (unsigned char)*s++;
    }
    return result % mod;
}

#define docolors(o) \
        o(  31, 0x483D8B, "darkslateblue") \
        o(  38, 0x90EE90, "light green") \
        o(  83, 0x97FFFF, "darkslategray1") \
        o(  84, 0x8DEEEE, "darkslategray2") \
        o(  85, 0x79CDCD, "darkslategray3") \
        o(  86, 0x528B8B, "darkslategray4") \
        o( 131, 0xFFFFF0, "ivory") \
        o( 133, 0x87CEEB, "skyblue") \
        o( 151, 0xF0FFF0, "honeydew1") \
        o( 152, 0xE0EEE0, "honeydew2") \
        o( 153, 0xC1CDC1, "honeydew3") \
        o( 154, 0x838B83, "honeydew4") \
        o( 161, 0xFF6EB4, "hotpink1") \
        o( 162, 0xEE6AA7, "hotpink2") \
        o( 163, 0xCD6090, "hotpink3") \
        o( 164, 0x8B3A62, "hotpink4") \
        o( 186, 0xDAA520, "goldenrod") \
        o( 209, 0xFFE4E1, "mistyrose") \
        o( 229, 0x98F5FF, "cadetblue1") \
        o( 230, 0x8EE5EE, "cadetblue2") \
        o( 231, 0x7AC5CD, "cadetblue3") \
        o( 232, 0x53868B, "cadetblue4") \
        o( 233, 0x1E90FF, "dodger blue") \
        o( 239, 0x0000CD, "mediumblue") \
        o( 261, 0xFFE4C4, "bisque1") \
        o( 262, 0xEED5B7, "bisque2") \
        o( 263, 0xCDB79E, "bisque3") \
        o( 264, 0x8B7D6B, "bisque4") \
        o( 279, 0x00BFFF, "deepskyblue1") \
        o( 280, 0x00B2EE, "deepskyblue2") \
        o( 281, 0x009ACD, "deepskyblue3") \
        o( 282, 0x00688B, "deepskyblue4") \
        o( 283, 0x4876FF, "royalblue1") \
        o( 284, 0x436EEE, "royalblue2") \
        o( 285, 0x3A5FCD, "royalblue3") \
        o( 286, 0x27408B, "royalblue4") \
        o( 288, 0xEE82EE, "violet") \
        o( 308, 0x3CB371, "mediumseagreen") \
        o( 327, 0xFFFFE0, "lightyellow") \
        o( 329, 0xFF6A6A, "indianred1") \
        o( 330, 0xEE6363, "indianred2") \
        o( 331, 0xCD5555, "indianred3") \
        o( 332, 0x8B3A3A, "indianred4") \
        o( 348, 0x20B2AA, "lightseagreen") \
        o( 369, 0xFDF5E6, "old lace") \
        o( 393, 0x00CED1, "darkturquoise") \
        o( 405, 0xCD853F, "peru") \
        o( 415, 0xFF69B4, "hot pink") \
        o( 435, 0xFF1493, "deeppink1") \
        o( 436, 0xEE1289, "deeppink2") \
        o( 437, 0xCD1076, "deeppink3") \
        o( 438, 0x8B0A50, "deeppink4") \
        o( 443, 0x7FFFD4, "aquamarine1") \
        o( 444, 0x76EEC6, "aquamarine2") \
        o( 445, 0x66CDAA, "aquamarine3") \
        o( 446, 0x458B74, "aquamarine4") \
        o( 533, 0xFFC0CB, "pink") \
        o( 553, 0xFF8247, "sienna1") \
        o( 554, 0xEE7942, "sienna2") \
        o( 555, 0xCD6839, "sienna3") \
        o( 556, 0x8B4726, "sienna4") \
        o( 586, 0xFFE4B5, "moccasin") \
        o( 635, 0xFF00FF, "magenta1") \
        o( 636, 0xEE00EE, "magenta2") \
        o( 637, 0xCD00CD, "magenta3") \
        o( 638, 0x8B008B, "magenta4") \
        o( 643, 0xADD8E6, "light blue") \
        o( 738, 0xFFFFFF, "gray100") \
        o( 793, 0xFFB90F, "darkgoldenrod1") \
        o( 794, 0xEEAD0E, "darkgoldenrod2") \
        o( 795, 0xCD950C, "darkgoldenrod3") \
        o( 796, 0x8B6508, "darkgoldenrod4") \
        o( 805, 0xB0C4DE, "light steel blue") \
        o( 869, 0xFF7F00, "darkorange1") \
        o( 870, 0xEE7600, "darkorange2") \
        o( 871, 0xCD6600, "darkorange3") \
        o( 872, 0x8B4500, "darkorange4") \
        o( 877, 0xFFF8DC, "cornsilk1") \
        o( 878, 0xEEE8CD, "cornsilk2") \
        o( 879, 0xCDC8B1, "cornsilk3") \
        o( 880, 0x8B8878, "cornsilk4") \
        o( 913, 0xBDB76B, "darkkhaki") \
        o( 926, 0xDB7093, "palevioletred") \
        o( 927, 0xFFD700, "gold1") \
        o( 928, 0xEEC900, "gold2") \
        o( 929, 0xCDAD00, "gold3") \
        o( 930, 0x8B7500, "gold4") \
        o( 937, 0x8B008B, "dark magenta") \
        o(1025, 0x63B8FF, "steelblue1") \
        o(1026, 0x5CACEE, "steelblue2") \
        o(1027, 0x4F94CD, "steelblue3") \
        o(1028, 0x36648B, "steelblue4") \
        o(1062, 0xFFF0F5, "lavenderblush") \
        o(1108, 0x333333, "gray20") \
        o(1109, 0x363636, "gray21") \
        o(1110, 0x383838, "gray22") \
        o(1111, 0x3B3B3B, "gray23") \
        o(1112, 0x3D3D3D, "gray24") \
        o(1113, 0x404040, "gray25") \
        o(1114, 0x424242, "gray26") \
        o(1115, 0x454545, "gray27") \
        o(1116, 0x474747, "gray28") \
        o(1117, 0x4A4A4A, "gray29") \
        o(1157, 0xBF3EFF, "darkorchid1") \
        o(1158, 0xB23AEE, "darkorchid2") \
        o(1159, 0x9A32CD, "darkorchid3") \
        o(1160, 0x68228B, "darkorchid4") \
        o(1169, 0xD3D3D3, "lightgrey") \
        o(1190, 0x8B0000, "dark red") \
        o(1301, 0xFF1493, "deep pink") \
        o(1341, 0xFFAEB9, "lightpink1") \
        o(1342, 0xEEA2AD, "lightpink2") \
        o(1343, 0xCD8C95, "lightpink3") \
        o(1344, 0x8B5F65, "lightpink4") \
        o(1358, 0x9932CC, "dark orchid") \
        o(1366, 0x1A1A1A, "gray10") \
        o(1367, 0x1C1C1C, "gray11") \
        o(1368, 0x1F1F1F, "gray12") \
        o(1369, 0x212121, "gray13") \
        o(1370, 0x242424, "gray14") \
        o(1371, 0x262626, "gray15") \
        o(1372, 0x292929, "gray16") \
        o(1373, 0x2B2B2B, "gray17") \
        o(1374, 0x2E2E2E, "gray18") \
        o(1375, 0x303030, "gray19") \
        o(1377, 0xFFF5EE, "seashell1") \
        o(1378, 0xEEE5DE, "seashell2") \
        o(1379, 0xCDC5BF, "seashell3") \
        o(1380, 0x8B8682, "seashell4") \
        o(1402, 0xFFF5EE, "seashell") \
        o(1407, 0x696969, "dimgray") \
        o(1420, 0xBA55D3, "mediumorchid") \
        o(1446, 0x8A2BE2, "blueviolet") \
        o(1469, 0x6A5ACD, "slate blue") \
        o(1473, 0xFFFFE0, "light yellow") \
        o(1524, 0xCCCCCC, "grey80") \
        o(1525, 0xCFCFCF, "grey81") \
        o(1526, 0xD1D1D1, "grey82") \
        o(1527, 0xD4D4D4, "grey83") \
        o(1528, 0xD6D6D6, "grey84") \
        o(1529, 0xD9D9D9, "grey85") \
        o(1530, 0xDBDBDB, "grey86") \
        o(1531, 0xDEDEDE, "grey87") \
        o(1532, 0xE0E0E0, "grey88") \
        o(1533, 0xE3E3E3, "grey89") \
        o(1535, 0xFF4040, "brown1") \
        o(1536, 0xEE3B3B, "brown2") \
        o(1537, 0xCD3333, "brown3") \
        o(1538, 0x8B2323, "brown4") \
        o(1579, 0xFF3E96, "violetred1") \
        o(1580, 0xEE3A8C, "violetred2") \
        o(1581, 0xCD3278, "violetred3") \
        o(1582, 0x8B2252, "violetred4") \
        o(1586, 0x7CFC00, "lawn green") \
        o(1713, 0xBBFFFF, "paleturquoise1") \
        o(1714, 0xAEEEEE, "paleturquoise2") \
        o(1715, 0x96CDCD, "paleturquoise3") \
        o(1716, 0x668B8B, "paleturquoise4") \
        o(1731, 0x00FFFF, "cyan1") \
        o(1732, 0x00EEEE, "cyan2") \
        o(1733, 0x00CDCD, "cyan3") \
        o(1734, 0x008B8B, "cyan4") \
        o(1821, 0xB0E0E6, "powderblue") \
        o(1859, 0x708090, "slate gray") \
        o(1931, 0xFFE1FF, "thistle1") \
        o(1932, 0xEED2EE, "thistle2") \
        o(1933, 0xCDB5CD, "thistle3") \
        o(1934, 0x8B7B8B, "thistle4") \
        o(1973, 0xFFE7BA, "wheat1") \
        o(1974, 0xEED8AE, "wheat2") \
        o(1975, 0xCDBA96, "wheat3") \
        o(1976, 0x8B7E66, "wheat4") \
        o(1982, 0x00FF7F, "spring green") \
        o(2025, 0xAFEEEE, "pale turquoise") \
        o(2047, 0xFFFFF0, "ivory1") \
        o(2048, 0xEEEEE0, "ivory2") \
        o(2049, 0xCDCDC1, "ivory3") \
        o(2050, 0x8B8B83, "ivory4") \
        o(2051, 0x00BFFF, "deep sky blue") \
        o(2067, 0x00FF00, "green1") \
        o(2068, 0x00EE00, "green2") \
        o(2069, 0x00CD00, "green3") \
        o(2070, 0x008B00, "green4") \
        o(2126, 0xE0FFFF, "lightcyan") \
        o(2159, 0xFFFAF0, "floral white") \
        o(2183, 0xFAEBD7, "antique white") \
        o(2207, 0x708090, "slategrey") \
        o(2222, 0xFFF0F5, "lavender blush") \
        o(2270, 0x8B0000, "darkred") \
        o(2280, 0x98FB98, "palegreen") \
        o(2290, 0x32CD32, "lime green") \
        o(2311, 0xF5F5F5, "white smoke") \
        o(2339, 0xFFD39B, "burlywood1") \
        o(2340, 0xEEC591, "burlywood2") \
        o(2341, 0xCDAA7D, "burlywood3") \
        o(2342, 0x8B7355, "burlywood4") \
        o(2349, 0xFFDEAD, "navajowhite") \
        o(2357, 0xD8BFD8, "thistle") \
        o(2373, 0xF0FFF0, "honeydew") \
        o(2388, 0xFFDAB9, "peach puff") \
        o(2392, 0x00FF7F, "springgreen") \
        o(2395, 0x1E90FF, "dodgerblue1") \
        o(2396, 0x1C86EE, "dodgerblue2") \
        o(2397, 0x1874CD, "dodgerblue3") \
        o(2398, 0x104E8B, "dodgerblue4") \
        o(2400, 0x9ACD32, "yellow green") \
        o(2433, 0x191970, "midnightblue") \
        o(2440, 0x8FBC8F, "dark sea green") \
        o(2466, 0x000000, "gray0") \
        o(2467, 0x030303, "gray1") \
        o(2468, 0x050505, "gray2") \
        o(2469, 0x080808, "gray3") \
        o(2470, 0x0A0A0A, "gray4") \
        o(2471, 0x0D0D0D, "gray5") \
        o(2472, 0x0F0F0F, "gray6") \
        o(2473, 0x121212, "gray7") \
        o(2474, 0x141414, "gray8") \
        o(2475, 0x171717, "gray9") \
        o(2478, 0x9400D3, "darkviolet") \
        o(2479, 0xD3D3D3, "light grey") \
        o(2527, 0xADFF2F, "greenyellow") \
        o(2575, 0x000080, "navy blue") \
        o(2585, 0x191970, "midnight blue") \
        o(2596, 0xCD5C5C, "indian red") \
        o(2675, 0xADFF2F, "green yellow") \
        o(2677, 0x00F5FF, "turquoise1") \
        o(2678, 0x00E5EE, "turquoise2") \
        o(2679, 0x00C5CD, "turquoise3") \
        o(2680, 0x00868B, "turquoise4") \
        o(2708, 0xB3B3B3, "gray70") \
        o(2709, 0xB5B5B5, "gray71") \
        o(2710, 0xB8B8B8, "gray72") \
        o(2711, 0xBABABA, "gray73") \
        o(2712, 0xBDBDBD, "gray74") \
        o(2713, 0xBFBFBF, "gray75") \
        o(2714, 0xC2C2C2, "gray76") \
        o(2715, 0xC4C4C4, "gray77") \
        o(2716, 0xC7C7C7, "gray78") \
        o(2717, 0xC9C9C9, "gray79") \
        o(2773, 0xF0FFFF, "azure1") \
        o(2774, 0xE0EEEE, "azure2") \
        o(2775, 0xC1CDCD, "azure3") \
        o(2776, 0x838B8B, "azure4") \
        o(2855, 0xFFB5C5, "pink1") \
        o(2856, 0xEEA9B8, "pink2") \
        o(2857, 0xCD919E, "pink3") \
        o(2858, 0x8B636C, "pink4") \
        o(2862, 0x00FFFF, "cyan") \
        o(2863, 0x7B68EE, "medium slate blue") \
        o(2865, 0x87CEFA, "lightskyblue") \
        o(2871, 0xF8F8FF, "ghostwhite") \
        o(2888, 0xEEDD82, "lightgoldenrod") \
        o(2945, 0xAFEEEE, "paleturquoise") \
        o(2968, 0xA52A2A, "brown") \
        o(2983, 0x9370DB, "medium purple") \
        o(3023, 0xFFB6C1, "lightpink") \
        o(3025, 0x00BFFF, "deepskyblue") \
        o(3039, 0x836FFF, "slateblue1") \
        o(3040, 0x7A67EE, "slateblue2") \
        o(3041, 0x6959CD, "slateblue3") \
        o(3042, 0x473C8B, "slateblue4") \
        o(3087, 0xFFC125, "goldenrod1") \
        o(3088, 0xEEB422, "goldenrod2") \
        o(3089, 0xCD9B1D, "goldenrod3") \
        o(3090, 0x8B6914, "goldenrod4") \
        o(3106, 0x8FBC8F, "darkseagreen") \
        o(3110, 0x00FA9A, "medium spring green") \
        o(3121, 0xFFE4E1, "misty rose") \
        o(3132, 0x228B22, "forestgreen") \
        o(3153, 0xFFE4C4, "bisque") \
        o(3159, 0x4682B4, "steelblue") \
        o(3201, 0xFF7256, "coral1") \
        o(3202, 0xEE6A50, "coral2") \
        o(3203, 0xCD5B45, "coral3") \
        o(3204, 0x8B3E2F, "coral4") \
        o(3205, 0x778899, "light slate gray") \
        o(3303, 0x0000FF, "blue") \
        o(3305, 0x2F4F4F, "darkslategrey") \
        o(3307, 0xD3D3D3, "lightgray") \
        o(3309, 0x2F4F4F, "dark slate gray") \
        o(3314, 0xE9967A, "dark salmon") \
        o(3332, 0xFF0000, "red") \
        o(3342, 0x7F7F7F, "grey50") \
        o(3343, 0x828282, "grey51") \
        o(3344, 0x858585, "grey52") \
        o(3345, 0x878787, "grey53") \
        o(3346, 0x8A8A8A, "grey54") \
        o(3347, 0x8C8C8C, "grey55") \
        o(3348, 0x8F8F8F, "grey56") \
        o(3349, 0x919191, "grey57") \
        o(3350, 0x949494, "grey58") \
        o(3351, 0x969696, "grey59") \
        o(3361, 0xB22222, "firebrick") \
        o(3399, 0x000080, "navyblue") \
        o(3412, 0xFFEFD5, "papayawhip") \
        o(3420, 0x000000, "grey0") \
        o(3421, 0x030303, "grey1") \
        o(3422, 0x050505, "grey2") \
        o(3423, 0x080808, "grey3") \
        o(3424, 0x0A0A0A, "grey4") \
        o(3425, 0x0D0D0D, "grey5") \
        o(3426, 0x0F0F0F, "grey6") \
        o(3427, 0x121212, "grey7") \
        o(3428, 0x141414, "grey8") \
        o(3429, 0x171717, "grey9") \
        o(3437, 0x8470FF, "light slate blue") \
        o(3441, 0xF5F5DC, "beige") \
        o(3442, 0xFFFACD, "lemon chiffon") \
        o(3460, 0x008B8B, "dark cyan") \
        o(3475, 0xFFA54F, "tan1") \
        o(3476, 0xEE9A49, "tan2") \
        o(3477, 0xCD853F, "tan3") \
        o(3478, 0x8B5A2B, "tan4") \
        o(3499, 0xA9A9A9, "dark gray") \
        o(3524, 0xFFEBCD, "blanched almond") \
        o(3569, 0xFFFF00, "yellow") \
        o(3616, 0x7F7F7F, "gray50") \
        o(3617, 0x828282, "gray51") \
        o(3618, 0x858585, "gray52") \
        o(3619, 0x878787, "gray53") \
        o(3620, 0x8A8A8A, "gray54") \
        o(3621, 0x8C8C8C, "gray55") \
        o(3622, 0x8F8F8F, "gray56") \
        o(3623, 0x919191, "gray57") \
        o(3624, 0x949494, "gray58") \
        o(3625, 0x969696, "gray59") \
        o(3675, 0xC1FFC1, "darkseagreen1") \
        o(3676, 0xB4EEB4, "darkseagreen2") \
        o(3677, 0x9BCD9B, "darkseagreen3") \
        o(3678, 0x698B69, "darkseagreen4") \
        o(3691, 0xF0F8FF, "alice blue") \
        o(3705, 0x5F9EA0, "cadetblue") \
        o(3709, 0x87CEEB, "sky blue") \
        o(3722, 0x7CFC00, "lawngreen") \
        o(3798, 0x2E8B57, "sea green") \
        o(3811, 0x87CEFA, "light sky blue") \
        o(3836, 0xFFFACD, "lemonchiffon") \
        o(3885, 0x1E90FF, "dodgerblue") \
        o(3939, 0xBDB76B, "dark khaki") \
        o(3969, 0xAB82FF, "mediumpurple1") \
        o(3970, 0x9F79EE, "mediumpurple2") \
        o(3971, 0x8968CD, "mediumpurple3") \
        o(3972, 0x5D478B, "mediumpurple4") \
        o(3998, 0xE9967A, "darksalmon") \
        o(4007, 0xA9A9A9, "darkgray") \
        o(4030, 0xD02090, "violetred") \
        o(4085, 0xFF1493, "deeppink") \
        o(4087, 0xF0E68C, "khaki") \
        o(4091, 0x7FFF00, "chartreuse") \
        o(4100, 0xC71585, "medium violet red") \
        o(4117, 0xF5F5F5, "whitesmoke") \
        o(4164, 0xD02090, "violet red") \
        o(4207, 0x0000CD, "medium blue") \
        o(4214, 0x8B4513, "saddlebrown") \
        o(4215, 0x000080, "navy") \
        o(4247, 0xBEBEBE, "grey") \
        o(4251, 0x9AFF9A, "palegreen1") \
        o(4252, 0x90EE90, "palegreen2") \
        o(4253, 0x7CCD7C, "palegreen3") \
        o(4254, 0x548B54, "palegreen4") \
        o(4295, 0x6A5ACD, "slateblue") \
        o(4309, 0xF8F8FF, "ghost white") \
        o(4349, 0xFFFF00, "yellow1") \
        o(4350, 0xEEEE00, "yellow2") \
        o(4351, 0xCDCD00, "yellow3") \
        o(4352, 0x8B8B00, "yellow4") \
        o(4355, 0xFF6347, "tomato") \
        o(4357, 0xF5FFFA, "mintcream") \
        o(4383, 0xA0522D, "sienna") \
        o(4387, 0xFDF5E6, "oldlace") \
        o(4397, 0x6495ED, "cornflower blue") \
        o(4403, 0xC6E2FF, "slategray1") \
        o(4404, 0xB9D3EE, "slategray2") \
        o(4405, 0x9FB6CD, "slategray3") \
        o(4406, 0x6C7B8B, "slategray4") \
        o(4424, 0xFAF0E6, "linen") \
        o(4425, 0xB0E0E6, "powder blue") \
        o(4428, 0xD70751, "debianred") \
        o(4469, 0xFFC1C1, "rosybrown1") \
        o(4470, 0xEEB4B4, "rosybrown2") \
        o(4471, 0xCD9B9B, "rosybrown3") \
        o(4472, 0x8B6969, "rosybrown4") \
        o(4528, 0xF4A460, "sandybrown") \
        o(4538, 0xEEE8AA, "palegoldenrod") \
        o(4543, 0xFFA07A, "lightsalmon1") \
        o(4544, 0xEE9572, "lightsalmon2") \
        o(4545, 0xCD8162, "lightsalmon3") \
        o(4546, 0x8B5742, "lightsalmon4") \
        o(4553, 0xBEBEBE, "gray") \
        o(4572, 0x006400, "darkgreen") \
        o(4584, 0x00FF00, "green") \
        o(4593, 0xFF8C00, "darkorange") \
        o(4603, 0xD2691E, "chocolate") \
        o(4610, 0xBC8F8F, "rosy brown") \
        o(4613, 0xFF00FF, "magenta") \
        o(4624, 0xEEE8AA, "pale goldenrod") \
        o(4657, 0xF0F8FF, "aliceblue") \
        o(4756, 0x3CB371, "medium sea green") \
        o(4778, 0xFFFFFF, "grey100") \
        o(4801, 0x66CDAA, "mediumaquamarine") \
        o(4842, 0xE6E6FA, "lavender") \
        o(4849, 0x4169E1, "royalblue") \
        o(4859, 0x778899, "lightslategray") \
        o(4909, 0xFFFAFA, "snow1") \
        o(4910, 0xEEE9E9, "snow2") \
        o(4911, 0xCDC9C9, "snow3") \
        o(4912, 0x8B8989, "snow4") \
        o(4936, 0xEEDD82, "light goldenrod") \
        o(4949, 0xFF34B3, "maroon1") \
        o(4950, 0xEE30A7, "maroon2") \
        o(4951, 0xCD2990, "maroon3") \
        o(4952, 0x8B1C62, "maroon4") \
        o(5019, 0x7FFFD4, "aquamarine") \
        o(5043, 0xF5FFFA, "mint cream") \
        o(5102, 0xFFDAB9, "peachpuff") \
        o(5105, 0xFF82AB, "palevioletred1") \
        o(5106, 0xEE799F, "palevioletred2") \
        o(5107, 0xCD6889, "palevioletred3") \
        o(5108, 0x8B475D, "palevioletred4") \
        o(5109, 0xE0FFFF, "lightcyan1") \
        o(5110, 0xD1EEEE, "lightcyan2") \
        o(5111, 0xB4CDCD, "lightcyan3") \
        o(5112, 0x7A8B8B, "lightcyan4") \
        o(5116, 0x00FA9A, "mediumspringgreen") \
        o(5126, 0x9ACD32, "yellowgreen") \
        o(5182, 0xBA55D3, "medium orchid") \
        o(5186, 0xB3B3B3, "grey70") \
        o(5187, 0xB5B5B5, "grey71") \
        o(5188, 0xB8B8B8, "grey72") \
        o(5189, 0xBABABA, "grey73") \
        o(5190, 0xBDBDBD, "grey74") \
        o(5191, 0xBFBFBF, "grey75") \
        o(5192, 0xC2C2C2, "grey76") \
        o(5193, 0xC4C4C4, "grey77") \
        o(5194, 0xC7C7C7, "grey78") \
        o(5195, 0xC9C9C9, "grey79") \
        o(5263, 0x00FF7F, "springgreen1") \
        o(5264, 0x00EE76, "springgreen2") \
        o(5265, 0x00CD66, "springgreen3") \
        o(5266, 0x008B45, "springgreen4") \
        o(5269, 0x9B30FF, "purple1") \
        o(5270, 0x912CEE, "purple2") \
        o(5271, 0x7D26CD, "purple3") \
        o(5272, 0x551A8B, "purple4") \
        o(5283, 0xD3D3D3, "light gray") \
        o(5291, 0xFFFFFF, "white") \
        o(5309, 0x778899, "lightslategrey") \
        o(5317, 0x48D1CC, "mediumturquoise") \
        o(5323, 0x696969, "dim grey") \
        o(5339, 0xFFDEAD, "navajo white") \
        o(5359, 0xFF8C69, "salmon1") \
        o(5360, 0xEE8262, "salmon2") \
        o(5361, 0xCD7054, "salmon3") \
        o(5362, 0x8B4C39, "salmon4") \
        o(5364, 0x556B2F, "darkolivegreen") \
        o(5383, 0x4682B4, "steel blue") \
        o(5404, 0x999999, "gray60") \
        o(5405, 0x9C9C9C, "gray61") \
        o(5406, 0x9E9E9E, "gray62") \
        o(5407, 0xA1A1A1, "gray63") \
        o(5408, 0xA3A3A3, "gray64") \
        o(5409, 0xA6A6A6, "gray65") \
        o(5410, 0xA8A8A8, "gray66") \
        o(5411, 0xABABAB, "gray67") \
        o(5412, 0xADADAD, "gray68") \
        o(5413, 0xB0B0B0, "gray69") \
        o(5424, 0x006400, "dark green") \
        o(5434, 0xC71585, "mediumvioletred") \
        o(5437, 0xFAEBD7, "antiquewhite") \
        o(5449, 0xADD8E6, "lightblue") \
        o(5456, 0xFFA07A, "lightsalmon") \
        o(5464, 0xF5DEB3, "wheat") \
        o(5575, 0xFFEC8B, "lightgoldenrod1") \
        o(5576, 0xEEDC82, "lightgoldenrod2") \
        o(5577, 0xCDBE70, "lightgoldenrod3") \
        o(5578, 0x8B814C, "lightgoldenrod4") \
        o(5600, 0xCD5C5C, "indianred") \
        o(5622, 0x556B2F, "dark olive green") \
        o(5623, 0x8B008B, "darkmagenta") \
        o(5624, 0x6B8E23, "olivedrab") \
        o(5637, 0x483D8B, "dark slate blue") \
        o(5652, 0xF4A460, "sandy brown") \
        o(5667, 0x8470FF, "lightslateblue") \
        o(5691, 0x54FF9F, "seagreen1") \
        o(5692, 0x4EEE94, "seagreen2") \
        o(5693, 0x43CD80, "seagreen3") \
        o(5694, 0x2E8B57, "seagreen4") \
        o(5716, 0x8B4513, "saddle brown") \
        o(5730, 0xE5E5E5, "grey90") \
        o(5731, 0xE8E8E8, "grey91") \
        o(5732, 0xEBEBEB, "grey92") \
        o(5733, 0xEDEDED, "grey93") \
        o(5734, 0xF0F0F0, "grey94") \
        o(5735, 0xF2F2F2, "grey95") \
        o(5736, 0xF5F5F5, "grey96") \
        o(5737, 0xF7F7F7, "grey97") \
        o(5738, 0xFAFAFA, "grey98") \
        o(5739, 0xFCFCFC, "grey99") \
        o(5762, 0xB8860B, "dark goldenrod") \
        o(5767, 0x7B68EE, "mediumslateblue") \
        o(5771, 0xFF6347, "tomato1") \
        o(5772, 0xEE5C42, "tomato2") \
        o(5773, 0xCD4F39, "tomato3") \
        o(5774, 0x8B3626, "tomato4") \
        o(5780, 0xFFEBCD, "blanchedalmond") \
        o(5813, 0xFFA500, "orange1") \
        o(5814, 0xEE9A00, "orange2") \
        o(5815, 0xCD8500, "orange3") \
        o(5816, 0x8B5A00, "orange4") \
        o(5831, 0xFFB6C1, "light pink") \
        o(5833, 0xFF7F24, "chocolate1") \
        o(5834, 0xEE7621, "chocolate2") \
        o(5835, 0xCD661D, "chocolate3") \
        o(5836, 0x8B4513, "chocolate4") \
        o(5837, 0x696969, "dim gray") \
        o(5862, 0xE0FFFF, "light cyan") \
        o(5907, 0x2F4F4F, "darkslategray") \
        o(5912, 0xF08080, "light coral") \
        o(5927, 0x5F9EA0, "cadet blue") \
        o(5957, 0xFFF0F5, "lavenderblush1") \
        o(5958, 0xEEE0E5, "lavenderblush2") \
        o(5959, 0xCDC1C5, "lavenderblush3") \
        o(5960, 0x8B8386, "lavenderblush4") \
        o(6046, 0x008B8B, "darkcyan") \
        o(6130, 0x333333, "grey20") \
        o(6131, 0x363636, "grey21") \
        o(6132, 0x383838, "grey22") \
        o(6133, 0x3B3B3B, "grey23") \
        o(6134, 0x3D3D3D, "grey24") \
        o(6135, 0x404040, "grey25") \
        o(6136, 0x424242, "grey26") \
        o(6137, 0x454545, "grey27") \
        o(6138, 0x474747, "grey28") \
        o(6139, 0x4A4A4A, "grey29") \
        o(6228, 0xF08080, "lightcoral") \
        o(6229, 0x0000FF, "blue1") \
        o(6230, 0x0000EE, "blue2") \
        o(6231, 0x0000CD, "blue3") \
        o(6232, 0x00008B, "blue4") \
        o(6233, 0x7FFF00, "chartreuse1") \
        o(6234, 0x76EE00, "chartreuse2") \
        o(6235, 0x66CD00, "chartreuse3") \
        o(6236, 0x458B00, "chartreuse4") \
        o(6243, 0x40E0D0, "turquoise") \
        o(6275, 0xFAFAD2, "lightgoldenrodyellow") \
        o(6282, 0xFFEFD5, "papaya whip") \
        o(6309, 0x778899, "light slate grey") \
        o(6335, 0xA9A9A9, "dark grey") \
        o(6363, 0xFFFACD, "lemonchiffon1") \
        o(6364, 0xEEE9BF, "lemonchiffon2") \
        o(6365, 0xCDC9A5, "lemonchiffon3") \
        o(6366, 0x8B8970, "lemonchiffon4") \
        o(6415, 0xFF0000, "red1") \
        o(6416, 0xEE0000, "red2") \
        o(6417, 0xCD0000, "red3") \
        o(6418, 0x8B0000, "red4") \
        o(6427, 0x696969, "dimgrey") \
        o(6481, 0xFFA500, "orange") \
        o(6499, 0xE066FF, "mediumorchid1") \
        o(6500, 0xD15FEE, "mediumorchid2") \
        o(6501, 0xB452CD, "mediumorchid3") \
        o(6502, 0x7A378B, "mediumorchid4") \
        o(6513, 0xFF83FA, "orchid1") \
        o(6514, 0xEE7AE9, "orchid2") \
        o(6515, 0xCD69C9, "orchid3") \
        o(6516, 0x8B4789, "orchid4") \
        o(6522, 0x2E8B57, "seagreen") \
        o(6535, 0xA9A9A9, "darkgrey") \
        o(6609, 0x00CED1, "dark turquoise") \
        o(6625, 0xFAFAD2, "light goldenrod yellow") \
        o(6627, 0xCAFF70, "darkolivegreen1") \
        o(6628, 0xBCEE68, "darkolivegreen2") \
        o(6629, 0xA2CD5A, "darkolivegreen3") \
        o(6630, 0x6E8B3D, "darkolivegreen4") \
        o(6648, 0x4D4D4D, "grey30") \
        o(6649, 0x4F4F4F, "grey31") \
        o(6650, 0x525252, "grey32") \
        o(6651, 0x545454, "grey33") \
        o(6652, 0x575757, "grey34") \
        o(6653, 0x595959, "grey35") \
        o(6654, 0x5C5C5C, "grey36") \
        o(6655, 0x5E5E5E, "grey37") \
        o(6656, 0x616161, "grey38") \
        o(6657, 0x636363, "grey39") \
        o(6687, 0xFFF68F, "khaki1") \
        o(6688, 0xEEE685, "khaki2") \
        o(6689, 0xCDC673, "khaki3") \
        o(6690, 0x8B864E, "khaki4") \
        o(6706, 0x8A2BE2, "blue violet") \
        o(6714, 0x32CD32, "limegreen") \
        o(6726, 0x999999, "grey60") \
        o(6727, 0x9C9C9C, "grey61") \
        o(6728, 0x9E9E9E, "grey62") \
        o(6729, 0xA1A1A1, "grey63") \
        o(6730, 0xA3A3A3, "grey64") \
        o(6731, 0xA6A6A6, "grey65") \
        o(6732, 0xA8A8A8, "grey66") \
        o(6733, 0xABABAB, "grey67") \
        o(6734, 0xADADAD, "grey68") \
        o(6735, 0xB0B0B0, "grey69") \
        o(6737, 0x66CDAA, "medium aquamarine") \
        o(6756, 0xB03060, "maroon") \
        o(6764, 0xCCCCCC, "gray80") \
        o(6765, 0xCFCFCF, "gray81") \
        o(6766, 0xD1D1D1, "gray82") \
        o(6767, 0xD4D4D4, "gray83") \
        o(6768, 0xD6D6D6, "gray84") \
        o(6769, 0xD9D9D9, "gray85") \
        o(6770, 0xDBDBDB, "gray86") \
        o(6771, 0xDEDEDE, "gray87") \
        o(6772, 0xE0E0E0, "gray88") \
        o(6773, 0xE3E3E3, "gray89") \
        o(6818, 0x9932CC, "darkorchid") \
        o(6824, 0x666666, "gray40") \
        o(6825, 0x696969, "gray41") \
        o(6826, 0x6B6B6B, "gray42") \
        o(6827, 0x6E6E6E, "gray43") \
        o(6828, 0x707070, "gray44") \
        o(6829, 0x737373, "gray45") \
        o(6830, 0x757575, "gray46") \
        o(6831, 0x787878, "gray47") \
        o(6832, 0x7A7A7A, "gray48") \
        o(6833, 0x7D7D7D, "gray49") \
        o(6884, 0x90EE90, "lightgreen") \
        o(6920, 0xD2B48C, "tan") \
        o(6957, 0xDDA0DD, "plum") \
        o(6991, 0xFFEFDB, "antiquewhite1") \
        o(6992, 0xEEDFCC, "antiquewhite2") \
        o(6993, 0xCDC0B0, "antiquewhite3") \
        o(6994, 0x8B8378, "antiquewhite4") \
        o(6996, 0xDB7093, "pale violet red") \
        o(7063, 0xFFE4E1, "mistyrose1") \
        o(7064, 0xEED5D2, "mistyrose2") \
        o(7065, 0xCDB7B5, "mistyrose3") \
        o(7066, 0x8B7D7B, "mistyrose4") \
        o(7078, 0xE5E5E5, "gray90") \
        o(7079, 0xE8E8E8, "gray91") \
        o(7080, 0xEBEBEB, "gray92") \
        o(7081, 0xEDEDED, "gray93") \
        o(7082, 0xF0F0F0, "gray94") \
        o(7083, 0xF2F2F2, "gray95") \
        o(7084, 0xF5F5F5, "gray96") \
        o(7085, 0xF7F7F7, "gray97") \
        o(7086, 0xFAFAFA, "gray98") \
        o(7087, 0xFCFCFC, "gray99") \
        o(7099, 0xDCDCDC, "gainsboro") \
        o(7100, 0xDA70D6, "orchid") \
        o(7133, 0x000000, "black") \
        o(7172, 0x228B22, "forest green") \
        o(7177, 0xB0E2FF, "lightskyblue1") \
        o(7178, 0xA4D3EE, "lightskyblue2") \
        o(7179, 0x8DB6CD, "lightskyblue3") \
        o(7180, 0x607B8B, "lightskyblue4") \
        o(7210, 0xB8860B, "darkgoldenrod") \
        o(7261, 0xFF69B4, "hotpink") \
        o(7271, 0x4169E1, "royal blue") \
        o(7282, 0xBC8F8F, "rosybrown") \
        o(7291, 0xF0FFFF, "azure") \
        o(7297, 0x87CEFF, "skyblue1") \
        o(7298, 0x7EC0EE, "skyblue2") \
        o(7299, 0x6CA6CD, "skyblue3") \
        o(7300, 0x4A708B, "skyblue4") \
        o(7316, 0xFF4500, "orangered") \
        o(7320, 0xFFD700, "gold") \
        o(7331, 0x00008B, "dark blue") \
        o(7335, 0x708090, "slate grey") \
        o(7362, 0x9400D3, "dark violet") \
        o(7370, 0xFF7F50, "coral") \
        o(7380, 0x1A1A1A, "grey10") \
        o(7381, 0x1C1C1C, "grey11") \
        o(7382, 0x1F1F1F, "grey12") \
        o(7383, 0x212121, "grey13") \
        o(7384, 0x242424, "grey14") \
        o(7385, 0x262626, "grey15") \
        o(7386, 0x292929, "grey16") \
        o(7387, 0x2B2B2B, "grey17") \
        o(7388, 0x2E2E2E, "grey18") \
        o(7389, 0x303030, "grey19") \
        o(7393, 0x708090, "slategray") \
        o(7405, 0xC0FF3E, "olivedrab1") \
        o(7406, 0xB3EE3A, "olivedrab2") \
        o(7407, 0x9ACD32, "olivedrab3") \
        o(7408, 0x698B22, "olivedrab4") \
        o(7455, 0xFF3030, "firebrick1") \
        o(7456, 0xEE2C2C, "firebrick2") \
        o(7457, 0xCD2626, "firebrick3") \
        o(7458, 0x8B1A1A, "firebrick4") \
        o(7469, 0xFF4500, "orangered1") \
        o(7470, 0xEE4000, "orangered2") \
        o(7471, 0xCD3700, "orangered3") \
        o(7472, 0x8B2500, "orangered4") \
        o(7476, 0xFA8072, "salmon") \
        o(7479, 0xFF8C00, "dark orange") \
        o(7490, 0xFF4500, "orange red") \
        o(7543, 0xFFBBFF, "plum1") \
        o(7544, 0xEEAEEE, "plum2") \
        o(7545, 0xCD96CD, "plum3") \
        o(7546, 0x8B668B, "plum4") \
        o(7560, 0x98FB98, "pale green") \
        o(7575, 0xFFDEAD, "navajowhite1") \
        o(7576, 0xEECFA1, "navajowhite2") \
        o(7577, 0xCDB38B, "navajowhite3") \
        o(7578, 0x8B795E, "navajowhite4") \
        o(7591, 0xA020F0, "purple") \
        o(7620, 0x20B2AA, "light sea green") \
        o(7642, 0xDEB887, "burlywood") \
        o(7651, 0x00008B, "darkblue") \
        o(7659, 0xCAE1FF, "lightsteelblue1") \
        o(7660, 0xBCD2EE, "lightsteelblue2") \
        o(7661, 0xA2B5CD, "lightsteelblue3") \
        o(7662, 0x6E7B8B, "lightsteelblue4") \
        o(7667, 0xBFEFFF, "lightblue1") \
        o(7668, 0xB2DFEE, "lightblue2") \
        o(7669, 0x9AC0CD, "lightblue3") \
        o(7670, 0x68838B, "lightblue4") \
        o(7727, 0x2F4F4F, "dark slate grey") \
        o(7813, 0x48D1CC, "medium turquoise") \
        o(7839, 0xFFDAB9, "peachpuff1") \
        o(7840, 0xEECBAD, "peachpuff2") \
        o(7841, 0xCDAF95, "peachpuff3") \
        o(7842, 0x8B7765, "peachpuff4") \
        o(7849, 0x9370DB, "mediumpurple") \
        o(7903, 0x6495ED, "cornflowerblue") \
        o(7922, 0x4D4D4D, "gray30") \
        o(7923, 0x4F4F4F, "gray31") \
        o(7924, 0x525252, "gray32") \
        o(7925, 0x545454, "gray33") \
        o(7926, 0x575757, "gray34") \
        o(7927, 0x595959, "gray35") \
        o(7928, 0x5C5C5C, "gray36") \
        o(7929, 0x5E5E5E, "gray37") \
        o(7930, 0x616161, "gray38") \
        o(7931, 0x636363, "gray39") \
        o(7971, 0xB0C4DE, "lightsteelblue") \
        o(7983, 0xFFFFE0, "lightyellow1") \
        o(7984, 0xEEEED1, "lightyellow2") \
        o(7985, 0xCDCDB4, "lightyellow3") \
        o(7986, 0x8B8B7A, "lightyellow4") \
        o(8034, 0x666666, "grey40") \
        o(8035, 0x696969, "grey41") \
        o(8036, 0x6B6B6B, "grey42") \
        o(8037, 0x6E6E6E, "grey43") \
        o(8038, 0x707070, "grey44") \
        o(8039, 0x737373, "grey45") \
        o(8040, 0x757575, "grey46") \
        o(8041, 0x787878, "grey47") \
        o(8042, 0x7A7A7A, "grey48") \
        o(8043, 0x7D7D7D, "grey49") \
        o(8068, 0xFFA07A, "light salmon") \
        o(8105, 0xFFFAFA, "snow") \
        o(8125, 0xFFFAF0, "floralwhite") \
        o(8133, 0xFFF8DC, "cornsilk") \
        o(8182, 0x6B8E23, "olive drab") \

#define v(a,b,s) static_assert(hash(s, sizeof(s)-1) == a);
#define o(a,b,s) a,
#define q(a,b,s) b,
static const unsigned short colorkeys[] { docolors(o) };
static const unsigned int colorvalues[] { docolors(q) };
docolors(v);
#undef v
#undef q
#undef o

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

unsigned ParseColorName(std::string_view s)
{
    std::string lc(s);
    for(char& c: lc) c = tolower(c);

    if(lc.size() >= 4 && lc.compare(0,1,"#") == 0) // shortest: "#FFF"
    {
        const char* p = lc.data()+1;
        auto[val, length] = ParseHex(p);
        unsigned result = 0;
        unsigned bits = length/3*4;
        for(unsigned n=0; n<3; ++n)
        {
            unsigned mask = ((0x1u << bits)-1);
            unsigned temp = val & mask; val >>= bits;
            temp = temp * 0xFF / mask;
            result += temp << (8*n);
        }
        return result;
    }
    if(lc.size() >= 9 && lc.compare(0,4,"rgb:") == 0) // shortest: "rgb:F/F/F"
    {
        const char* p = lc.data()+4;
    	unsigned result = 0;
    	for(unsigned n=0; n<3; ++n)
    	{
    	    while(*p == '/') ++p;
    	    auto[val, length] = ParseHex(p);
    	    unsigned bits = length*4;
            unsigned mask = ((0x1u << bits)-1);
            unsigned temp = val & mask; val >>= bits;
            result <<= 8;
            result += temp * 0xFF / mask;
    	}
    	return result;
    }
    if(lc.size() >= 10 && lc.compare(0,5,"rgbi:") == 0) // shortest: "rgbi:1/1/1"
    {
        const char* p = lc.data()+5;
    	unsigned result = 0;
    	for(unsigned n=0; n<3; ++n)
    	{
    	    while(*p == '/') ++p;
            result <<= 8;
            result += ParseDbl(p);
    	}
    	return result;
    }

    unsigned short h = hash(lc.data(), lc.size());
#if 0
    #define o(a,b,s) case a: return b;
    switch(h) { docolors(o) }
    #undef o
#else
    auto i = std::lower_bound(std::begin(colorkeys), std::end(colorkeys), h);
    if(i != std::end(colorkeys) && *i == h)
        return colorvalues[ i-std::begin(colorkeys) ];
#endif
    return 0;
}
