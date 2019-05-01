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

static constexpr std::uint_least64_t a = 0xC5959135F5F4EE52;
static constexpr std::uint_least64_t b = 0x9833CB76814DF6DB;
static constexpr std::uint_least64_t c = 0x3B4236FA60071A28;
static constexpr std::uint_least64_t d = 0xD43C17959D1B8457;
static constexpr unsigned mod          = 7709;
static constexpr unsigned shift        = 0x37;

static constexpr unsigned hash(const char* s, unsigned length)
{
    std::uint_least64_t result = a;
    unsigned s1 = 1 + 16 * ((shift>>0)&3);
    unsigned s2 = 1 + 16 * ((shift>>2)&3);
    unsigned s3 = 1 + 16 * ((shift>>4)&3);
    while(length--)
    {
        result = (result ^ (result >> s1)) * b;
        result = (result ^ (result >> s2)) * c;
        result = (result ^ (result >> s3)) * d;
        result += (unsigned char)*s++;
    }
    return result % mod;
}

#define docolors(o) \
        o(  27, 0xAFEEEE, "paleturquoise") \
        o(  73, 0xCCCCCC, "grey80") \
        o(  74, 0xCFCFCF, "grey81") \
        o(  75, 0xD1D1D1, "grey82") \
        o(  76, 0xD4D4D4, "grey83") \
        o(  77, 0xD6D6D6, "grey84") \
        o(  78, 0xD9D9D9, "grey85") \
        o(  79, 0xDBDBDB, "grey86") \
        o(  80, 0xDEDEDE, "grey87") \
        o(  81, 0xE0E0E0, "grey88") \
        o(  82, 0xE3E3E3, "grey89") \
        o( 133, 0x696969, "dim grey") \
        o( 200, 0x333333, "gray20") \
        o( 201, 0x363636, "gray21") \
        o( 202, 0x383838, "gray22") \
        o( 203, 0x3B3B3B, "gray23") \
        o( 204, 0x3D3D3D, "gray24") \
        o( 205, 0x404040, "gray25") \
        o( 206, 0x424242, "gray26") \
        o( 207, 0x454545, "gray27") \
        o( 208, 0x474747, "gray28") \
        o( 209, 0x4A4A4A, "gray29") \
        o( 218, 0xAB82FF, "mediumpurple1") \
        o( 219, 0x9F79EE, "mediumpurple2") \
        o( 220, 0x8968CD, "mediumpurple3") \
        o( 221, 0x5D478B, "mediumpurple4") \
        o( 258, 0x3CB371, "mediumseagreen") \
        o( 269, 0xADFF2F, "greenyellow") \
        o( 285, 0xE6E6FA, "lavender") \
        o( 313, 0x00BFFF, "deepskyblue1") \
        o( 314, 0x00B2EE, "deepskyblue2") \
        o( 315, 0x009ACD, "deepskyblue3") \
        o( 316, 0x00688B, "deepskyblue4") \
        o( 326, 0xFFE4E1, "misty rose") \
        o( 327, 0x000000, "gray0") \
        o( 328, 0x030303, "gray1") \
        o( 329, 0x050505, "gray2") \
        o( 330, 0x080808, "gray3") \
        o( 331, 0x0A0A0A, "gray4") \
        o( 332, 0x0D0D0D, "gray5") \
        o( 333, 0x0F0F0F, "gray6") \
        o( 334, 0x121212, "gray7") \
        o( 335, 0x141414, "gray8") \
        o( 336, 0x171717, "gray9") \
        o( 355, 0xFFFACD, "lemonchiffon1") \
        o( 356, 0xEEE9BF, "lemonchiffon2") \
        o( 357, 0xCDC9A5, "lemonchiffon3") \
        o( 358, 0x8B8970, "lemonchiffon4") \
        o( 370, 0xF0F8FF, "alice blue") \
        o( 397, 0xFF4500, "orangered") \
        o( 416, 0x2E8B57, "seagreen") \
        o( 435, 0xFFFACD, "lemonchiffon") \
        o( 436, 0x87CEFF, "skyblue1") \
        o( 437, 0x7EC0EE, "skyblue2") \
        o( 438, 0x6CA6CD, "skyblue3") \
        o( 439, 0x4A708B, "skyblue4") \
        o( 442, 0xF5FFFA, "mintcream") \
        o( 469, 0x1E90FF, "dodger blue") \
        o( 481, 0xFFD39B, "burlywood1") \
        o( 482, 0xEEC591, "burlywood2") \
        o( 483, 0xCDAA7D, "burlywood3") \
        o( 484, 0x8B7355, "burlywood4") \
        o( 485, 0x008B8B, "dark cyan") \
        o( 503, 0xBF3EFF, "darkorchid1") \
        o( 504, 0xB23AEE, "darkorchid2") \
        o( 505, 0x9A32CD, "darkorchid3") \
        o( 506, 0x68228B, "darkorchid4") \
        o( 568, 0x20B2AA, "lightseagreen") \
        o( 589, 0x000080, "navy") \
        o( 616, 0xFFC1C1, "rosybrown1") \
        o( 617, 0xEEB4B4, "rosybrown2") \
        o( 618, 0xCD9B9B, "rosybrown3") \
        o( 619, 0x8B6969, "rosybrown4") \
        o( 622, 0x0000FF, "blue") \
        o( 629, 0xD3D3D3, "lightgrey") \
        o( 664, 0x8470FF, "lightslateblue") \
        o( 671, 0x666666, "gray40") \
        o( 672, 0x696969, "gray41") \
        o( 673, 0x6B6B6B, "gray42") \
        o( 674, 0x6E6E6E, "gray43") \
        o( 675, 0x707070, "gray44") \
        o( 676, 0x737373, "gray45") \
        o( 677, 0x757575, "gray46") \
        o( 678, 0x787878, "gray47") \
        o( 679, 0x7A7A7A, "gray48") \
        o( 680, 0x7D7D7D, "gray49") \
        o( 681, 0xFFFFE0, "light yellow") \
        o( 699, 0x8B008B, "darkmagenta") \
        o( 717, 0xFFDAB9, "peach puff") \
        o( 759, 0xB0C4DE, "lightsteelblue") \
        o( 782, 0xDA70D6, "orchid") \
        o( 784, 0x9370DB, "medium purple") \
        o( 794, 0xF5FFFA, "mint cream") \
        o( 802, 0xB22222, "firebrick") \
        o( 852, 0xFA8072, "salmon") \
        o( 867, 0xE0FFFF, "light cyan") \
        o( 878, 0xF0FFFF, "azure1") \
        o( 879, 0xE0EEEE, "azure2") \
        o( 880, 0xC1CDCD, "azure3") \
        o( 881, 0x838B8B, "azure4") \
        o( 883, 0x00008B, "darkblue") \
        o( 887, 0x48D1CC, "mediumturquoise") \
        o( 892, 0x5F9EA0, "cadet blue") \
        o( 896, 0xE5E5E5, "gray90") \
        o( 897, 0xE8E8E8, "gray91") \
        o( 898, 0xEBEBEB, "gray92") \
        o( 899, 0xEDEDED, "gray93") \
        o( 900, 0xF0F0F0, "gray94") \
        o( 901, 0xF2F2F2, "gray95") \
        o( 902, 0xF5F5F5, "gray96") \
        o( 903, 0xF7F7F7, "gray97") \
        o( 904, 0xFAFAFA, "gray98") \
        o( 905, 0xFCFCFC, "gray99") \
        o( 932, 0xA9A9A9, "dark grey") \
        o( 945, 0xCD5C5C, "indian red") \
        o( 948, 0xA9A9A9, "dark gray") \
        o( 952, 0xFFFAFA, "snow") \
        o( 968, 0xFF00FF, "magenta1") \
        o( 969, 0xEE00EE, "magenta2") \
        o( 970, 0xCD00CD, "magenta3") \
        o( 971, 0x8B008B, "magenta4") \
        o( 973, 0xDB7093, "pale violet red") \
        o( 982, 0xFFE4B5, "moccasin") \
        o(1016, 0xFFA07A, "lightsalmon1") \
        o(1017, 0xEE9572, "lightsalmon2") \
        o(1018, 0xCD8162, "lightsalmon3") \
        o(1019, 0x8B5742, "lightsalmon4") \
        o(1036, 0x00CED1, "dark turquoise") \
        o(1039, 0xD02090, "violetred") \
        o(1040, 0x9932CC, "darkorchid") \
        o(1052, 0xFF82AB, "palevioletred1") \
        o(1053, 0xEE799F, "palevioletred2") \
        o(1054, 0xCD6889, "palevioletred3") \
        o(1055, 0x8B475D, "palevioletred4") \
        o(1122, 0xFFB5C5, "pink1") \
        o(1123, 0xEEA9B8, "pink2") \
        o(1124, 0xCD919E, "pink3") \
        o(1125, 0x8B636C, "pink4") \
        o(1141, 0xE0FFFF, "lightcyan1") \
        o(1142, 0xD1EEEE, "lightcyan2") \
        o(1143, 0xB4CDCD, "lightcyan3") \
        o(1144, 0x7A8B8B, "lightcyan4") \
        o(1222, 0x8B4513, "saddlebrown") \
        o(1275, 0x708090, "slate gray") \
        o(1277, 0xEEE8AA, "pale goldenrod") \
        o(1281, 0x228B22, "forest green") \
        o(1285, 0xFFDEAD, "navajowhite1") \
        o(1286, 0xEECFA1, "navajowhite2") \
        o(1287, 0xCDB38B, "navajowhite3") \
        o(1288, 0x8B795E, "navajowhite4") \
        o(1394, 0xFFFFFF, "white") \
        o(1405, 0xE0FFFF, "lightcyan") \
        o(1419, 0xFFE7BA, "wheat1") \
        o(1420, 0xEED8AE, "wheat2") \
        o(1421, 0xCDBA96, "wheat3") \
        o(1422, 0x8B7E66, "wheat4") \
        o(1430, 0xFFF8DC, "cornsilk1") \
        o(1431, 0xEEE8CD, "cornsilk2") \
        o(1432, 0xCDC8B1, "cornsilk3") \
        o(1433, 0x8B8878, "cornsilk4") \
        o(1464, 0xBBFFFF, "paleturquoise1") \
        o(1465, 0xAEEEEE, "paleturquoise2") \
        o(1466, 0x96CDCD, "paleturquoise3") \
        o(1467, 0x668B8B, "paleturquoise4") \
        o(1472, 0x4682B4, "steelblue") \
        o(1506, 0xFF6EB4, "hotpink1") \
        o(1507, 0xEE6AA7, "hotpink2") \
        o(1508, 0xCD6090, "hotpink3") \
        o(1509, 0x8B3A62, "hotpink4") \
        o(1518, 0x8B4513, "saddle brown") \
        o(1530, 0x8FBC8F, "darkseagreen") \
        o(1546, 0xFFEFD5, "papaya whip") \
        o(1553, 0xB03060, "maroon") \
        o(1557, 0xBA55D3, "mediumorchid") \
        o(1598, 0xD2691E, "chocolate") \
        o(1604, 0xE9967A, "darksalmon") \
        o(1616, 0xF0FFF0, "honeydew") \
        o(1626, 0xFFE4C4, "bisque1") \
        o(1627, 0xEED5B7, "bisque2") \
        o(1628, 0xCDB79E, "bisque3") \
        o(1629, 0x8B7D6B, "bisque4") \
        o(1683, 0xD2B48C, "tan") \
        o(1690, 0xD3D3D3, "light grey") \
        o(1713, 0x87CEFA, "light sky blue") \
        o(1717, 0x90EE90, "lightgreen") \
        o(1741, 0x90EE90, "light green") \
        o(1742, 0xBC8F8F, "rosybrown") \
        o(1750, 0xDCDCDC, "gainsboro") \
        o(1759, 0xFFB6C1, "light pink") \
        o(1782, 0x1E90FF, "dodgerblue") \
        o(1792, 0xA52A2A, "brown") \
        o(1813, 0xF0FFF0, "honeydew1") \
        o(1814, 0xE0EEE0, "honeydew2") \
        o(1815, 0xC1CDC1, "honeydew3") \
        o(1816, 0x838B83, "honeydew4") \
        o(1817, 0xFF1493, "deeppink") \
        o(1819, 0xF0FFFF, "azure") \
        o(1820, 0x63B8FF, "steelblue1") \
        o(1821, 0x5CACEE, "steelblue2") \
        o(1822, 0x4F94CD, "steelblue3") \
        o(1823, 0x36648B, "steelblue4") \
        o(1830, 0x4169E1, "royal blue") \
        o(1845, 0x9932CC, "dark orchid") \
        o(1853, 0x999999, "grey60") \
        o(1854, 0x9C9C9C, "grey61") \
        o(1855, 0x9E9E9E, "grey62") \
        o(1856, 0xA1A1A1, "grey63") \
        o(1857, 0xA3A3A3, "grey64") \
        o(1858, 0xA6A6A6, "grey65") \
        o(1859, 0xA8A8A8, "grey66") \
        o(1860, 0xABABAB, "grey67") \
        o(1861, 0xADADAD, "grey68") \
        o(1862, 0xB0B0B0, "grey69") \
        o(1871, 0xFFBBFF, "plum1") \
        o(1872, 0xEEAEEE, "plum2") \
        o(1873, 0xCD96CD, "plum3") \
        o(1874, 0x8B668B, "plum4") \
        o(1903, 0xFF3E96, "violetred1") \
        o(1904, 0xEE3A8C, "violetred2") \
        o(1905, 0xCD3278, "violetred3") \
        o(1906, 0x8B2252, "violetred4") \
        o(1907, 0xEEDD82, "light goldenrod") \
        o(1916, 0x8FBC8F, "dark sea green") \
        o(1919, 0x191970, "midnight blue") \
        o(1924, 0x00FA9A, "medium spring green") \
        o(1948, 0x708090, "slategrey") \
        o(1960, 0x48D1CC, "medium turquoise") \
        o(1973, 0xFF6A6A, "indianred1") \
        o(1974, 0xEE6363, "indianred2") \
        o(1975, 0xCD5555, "indianred3") \
        o(1976, 0x8B3A3A, "indianred4") \
        o(2006, 0xBA55D3, "medium orchid") \
        o(2064, 0xE9967A, "dark salmon") \
        o(2067, 0x7FFF00, "chartreuse1") \
        o(2068, 0x76EE00, "chartreuse2") \
        o(2069, 0x66CD00, "chartreuse3") \
        o(2070, 0x458B00, "chartreuse4") \
        o(2125, 0x6495ED, "cornflower blue") \
        o(2147, 0xFFA500, "orange") \
        o(2164, 0xFFF0F5, "lavenderblush") \
        o(2191, 0x3CB371, "medium sea green") \
        o(2199, 0x97FFFF, "darkslategray1") \
        o(2200, 0x8DEEEE, "darkslategray2") \
        o(2201, 0x79CDCD, "darkslategray3") \
        o(2202, 0x528B8B, "darkslategray4") \
        o(2227, 0xFFFF00, "yellow") \
        o(2239, 0x6A5ACD, "slateblue") \
        o(2257, 0xD8BFD8, "thistle") \
        o(2292, 0x4169E1, "royalblue") \
        o(2342, 0x4D4D4D, "gray30") \
        o(2343, 0x4F4F4F, "gray31") \
        o(2344, 0x525252, "gray32") \
        o(2345, 0x545454, "gray33") \
        o(2346, 0x575757, "gray34") \
        o(2347, 0x595959, "gray35") \
        o(2348, 0x5C5C5C, "gray36") \
        o(2349, 0x5E5E5E, "gray37") \
        o(2350, 0x616161, "gray38") \
        o(2351, 0x636363, "gray39") \
        o(2362, 0x7FFFD4, "aquamarine") \
        o(2399, 0xFFA07A, "lightsalmon") \
        o(2403, 0xFF69B4, "hotpink") \
        o(2429, 0x8470FF, "light slate blue") \
        o(2435, 0xE066FF, "mediumorchid1") \
        o(2436, 0xD15FEE, "mediumorchid2") \
        o(2437, 0xB452CD, "mediumorchid3") \
        o(2438, 0x7A378B, "mediumorchid4") \
        o(2462, 0x7FFF00, "chartreuse") \
        o(2470, 0xFFF68F, "khaki1") \
        o(2471, 0xEEE685, "khaki2") \
        o(2472, 0xCDC673, "khaki3") \
        o(2473, 0x8B864E, "khaki4") \
        o(2480, 0xFFF8DC, "cornsilk") \
        o(2492, 0x8A2BE2, "blue violet") \
        o(2494, 0xFFF5EE, "seashell") \
        o(2495, 0x2F4F4F, "dark slate gray") \
        o(2501, 0xD02090, "violet red") \
        o(2507, 0xB3B3B3, "grey70") \
        o(2508, 0xB5B5B5, "grey71") \
        o(2509, 0xB8B8B8, "grey72") \
        o(2510, 0xBABABA, "grey73") \
        o(2511, 0xBDBDBD, "grey74") \
        o(2512, 0xBFBFBF, "grey75") \
        o(2513, 0xC2C2C2, "grey76") \
        o(2514, 0xC4C4C4, "grey77") \
        o(2515, 0xC7C7C7, "grey78") \
        o(2516, 0xC9C9C9, "grey79") \
        o(2525, 0x4D4D4D, "grey30") \
        o(2526, 0x4F4F4F, "grey31") \
        o(2527, 0x525252, "grey32") \
        o(2528, 0x545454, "grey33") \
        o(2529, 0x575757, "grey34") \
        o(2530, 0x595959, "grey35") \
        o(2531, 0x5C5C5C, "grey36") \
        o(2532, 0x5E5E5E, "grey37") \
        o(2533, 0x616161, "grey38") \
        o(2534, 0x636363, "grey39") \
        o(2543, 0xFF6347, "tomato1") \
        o(2544, 0xEE5C42, "tomato2") \
        o(2545, 0xCD4F39, "tomato3") \
        o(2546, 0x8B3626, "tomato4") \
        o(2575, 0xADD8E6, "light blue") \
        o(2620, 0x333333, "grey20") \
        o(2621, 0x363636, "grey21") \
        o(2622, 0x383838, "grey22") \
        o(2623, 0x3B3B3B, "grey23") \
        o(2624, 0x3D3D3D, "grey24") \
        o(2625, 0x404040, "grey25") \
        o(2626, 0x424242, "grey26") \
        o(2627, 0x454545, "grey27") \
        o(2628, 0x474747, "grey28") \
        o(2629, 0x4A4A4A, "grey29") \
        o(2641, 0x00F5FF, "turquoise1") \
        o(2642, 0x00E5EE, "turquoise2") \
        o(2643, 0x00C5CD, "turquoise3") \
        o(2644, 0x00868B, "turquoise4") \
        o(2669, 0x708090, "slate grey") \
        o(2674, 0x66CDAA, "medium aquamarine") \
        o(2702, 0x54FF9F, "seagreen1") \
        o(2703, 0x4EEE94, "seagreen2") \
        o(2704, 0x43CD80, "seagreen3") \
        o(2705, 0x2E8B57, "seagreen4") \
        o(2710, 0xFF0000, "red") \
        o(2734, 0xFFC0CB, "pink") \
        o(2755, 0xADFF2F, "green yellow") \
        o(2775, 0xA020F0, "purple") \
        o(2785, 0xFFE4C4, "bisque") \
        o(2821, 0x9ACD32, "yellow green") \
        o(2838, 0x0000CD, "mediumblue") \
        o(2926, 0x32CD32, "limegreen") \
        o(2928, 0xFFA07A, "light salmon") \
        o(2958, 0x5F9EA0, "cadetblue") \
        o(3025, 0xBDB76B, "darkkhaki") \
        o(3085, 0xFF7F00, "darkorange1") \
        o(3086, 0xEE7600, "darkorange2") \
        o(3087, 0xCD6600, "darkorange3") \
        o(3088, 0x8B4500, "darkorange4") \
        o(3091, 0xBEBEBE, "gray") \
        o(3173, 0x9400D3, "darkviolet") \
        o(3177, 0x2F4F4F, "darkslategray") \
        o(3232, 0xFFA54F, "tan1") \
        o(3233, 0xEE9A49, "tan2") \
        o(3234, 0xCD853F, "tan3") \
        o(3235, 0x8B5A2B, "tan4") \
        o(3238, 0xFF1493, "deep pink") \
        o(3240, 0xFFFFE0, "lightyellow1") \
        o(3241, 0xEEEED1, "lightyellow2") \
        o(3242, 0xCDCDB4, "lightyellow3") \
        o(3243, 0x8B8B7A, "lightyellow4") \
        o(3273, 0x87CEEB, "skyblue") \
        o(3348, 0x32CD32, "lime green") \
        o(3365, 0xCCCCCC, "gray80") \
        o(3366, 0xCFCFCF, "gray81") \
        o(3367, 0xD1D1D1, "gray82") \
        o(3368, 0xD4D4D4, "gray83") \
        o(3369, 0xD6D6D6, "gray84") \
        o(3370, 0xD9D9D9, "gray85") \
        o(3371, 0xDBDBDB, "gray86") \
        o(3372, 0xDEDEDE, "gray87") \
        o(3373, 0xE0E0E0, "gray88") \
        o(3374, 0xE3E3E3, "gray89") \
        o(3380, 0xF4A460, "sandy brown") \
        o(3396, 0xFFEBCD, "blanched almond") \
        o(3399, 0xFF8C00, "dark orange") \
        o(3407, 0xFAF0E6, "linen") \
        o(3521, 0x98FB98, "pale green") \
        o(3523, 0xF8F8FF, "ghostwhite") \
        o(3531, 0xB8860B, "darkgoldenrod") \
        o(3543, 0x1E90FF, "dodgerblue1") \
        o(3544, 0x1C86EE, "dodgerblue2") \
        o(3545, 0x1874CD, "dodgerblue3") \
        o(3546, 0x104E8B, "dodgerblue4") \
        o(3564, 0x00FF7F, "spring green") \
        o(3578, 0x8B008B, "dark magenta") \
        o(3590, 0xCD853F, "peru") \
        o(3647, 0xADD8E6, "lightblue") \
        o(3668, 0x006400, "darkgreen") \
        o(3679, 0xD3D3D3, "light gray") \
        o(3730, 0xFFDAB9, "peachpuff") \
        o(3799, 0xFF1493, "deeppink1") \
        o(3800, 0xEE1289, "deeppink2") \
        o(3801, 0xCD1076, "deeppink3") \
        o(3802, 0x8B0A50, "deeppink4") \
        o(3817, 0x778899, "light slate gray") \
        o(3837, 0xF4A460, "sandybrown") \
        o(3877, 0xA9A9A9, "darkgray") \
        o(3934, 0x000000, "grey0") \
        o(3935, 0x030303, "grey1") \
        o(3936, 0x050505, "grey2") \
        o(3937, 0x080808, "grey3") \
        o(3938, 0x0A0A0A, "grey4") \
        o(3939, 0x0D0D0D, "grey5") \
        o(3940, 0x0F0F0F, "grey6") \
        o(3941, 0x121212, "grey7") \
        o(3942, 0x141414, "grey8") \
        o(3943, 0x171717, "grey9") \
        o(3976, 0xF8F8FF, "ghost white") \
        o(3996, 0x00CED1, "darkturquoise") \
        o(4018, 0xFFE1FF, "thistle1") \
        o(4019, 0xEED2EE, "thistle2") \
        o(4020, 0xCDB5CD, "thistle3") \
        o(4021, 0x8B7B8B, "thistle4") \
        o(4041, 0x8B0000, "dark red") \
        o(4062, 0x483D8B, "darkslateblue") \
        o(4079, 0x00FFFF, "cyan1") \
        o(4080, 0x00EEEE, "cyan2") \
        o(4081, 0x00CDCD, "cyan3") \
        o(4082, 0x008B8B, "cyan4") \
        o(4089, 0xF5DEB3, "wheat") \
        o(4092, 0xFF8247, "sienna1") \
        o(4093, 0xEE7942, "sienna2") \
        o(4094, 0xCD6839, "sienna3") \
        o(4095, 0x8B4726, "sienna4") \
        o(4120, 0xFF7F50, "coral") \
        o(4126, 0xFFFFFF, "gray100") \
        o(4132, 0xFFF5EE, "seashell1") \
        o(4133, 0xEEE5DE, "seashell2") \
        o(4134, 0xCDC5BF, "seashell3") \
        o(4135, 0x8B8682, "seashell4") \
        o(4185, 0x2F4F4F, "darkslategrey") \
        o(4208, 0xFF4040, "brown1") \
        o(4209, 0xEE3B3B, "brown2") \
        o(4210, 0xCD3333, "brown3") \
        o(4211, 0x8B2323, "brown4") \
        o(4213, 0xFAFAD2, "lightgoldenrodyellow") \
        o(4232, 0xB0C4DE, "light steel blue") \
        o(4279, 0xF0F8FF, "aliceblue") \
        o(4351, 0xFFE4E1, "mistyrose1") \
        o(4352, 0xEED5D2, "mistyrose2") \
        o(4353, 0xCDB7B5, "mistyrose3") \
        o(4354, 0x8B7D7B, "mistyrose4") \
        o(4386, 0x8B0000, "darkred") \
        o(4425, 0x7CFC00, "lawn green") \
        o(4426, 0xBEBEBE, "grey") \
        o(4435, 0x00FFFF, "cyan") \
        o(4445, 0x1A1A1A, "grey10") \
        o(4446, 0x1C1C1C, "grey11") \
        o(4447, 0x1F1F1F, "grey12") \
        o(4448, 0x212121, "grey13") \
        o(4449, 0x242424, "grey14") \
        o(4450, 0x262626, "grey15") \
        o(4451, 0x292929, "grey16") \
        o(4452, 0x2B2B2B, "grey17") \
        o(4453, 0x2E2E2E, "grey18") \
        o(4454, 0x303030, "grey19") \
        o(4468, 0xF5F5F5, "white smoke") \
        o(4517, 0x7F7F7F, "grey50") \
        o(4518, 0x828282, "grey51") \
        o(4519, 0x858585, "grey52") \
        o(4520, 0x878787, "grey53") \
        o(4521, 0x8A8A8A, "grey54") \
        o(4522, 0x8C8C8C, "grey55") \
        o(4523, 0x8F8F8F, "grey56") \
        o(4524, 0x919191, "grey57") \
        o(4525, 0x949494, "grey58") \
        o(4526, 0x969696, "grey59") \
        o(4533, 0xDDA0DD, "plum") \
        o(4534, 0x00BFFF, "deep sky blue") \
        o(4540, 0x696969, "dim gray") \
        o(4560, 0xB3B3B3, "gray70") \
        o(4561, 0xB5B5B5, "gray71") \
        o(4562, 0xB8B8B8, "gray72") \
        o(4563, 0xBABABA, "gray73") \
        o(4564, 0xBDBDBD, "gray74") \
        o(4565, 0xBFBFBF, "gray75") \
        o(4566, 0xC2C2C2, "gray76") \
        o(4567, 0xC4C4C4, "gray77") \
        o(4568, 0xC7C7C7, "gray78") \
        o(4569, 0xC9C9C9, "gray79") \
        o(4592, 0xFFA500, "orange1") \
        o(4593, 0xEE9A00, "orange2") \
        o(4594, 0xCD8500, "orange3") \
        o(4595, 0x8B5A00, "orange4") \
        o(4616, 0xFFFAF0, "floralwhite") \
        o(4652, 0xEEDD82, "lightgoldenrod") \
        o(4654, 0xA9A9A9, "darkgrey") \
        o(4659, 0x556B2F, "darkolivegreen") \
        o(4661, 0xFFFFF0, "ivory1") \
        o(4662, 0xEEEEE0, "ivory2") \
        o(4663, 0xCDCDC1, "ivory3") \
        o(4664, 0x8B8B83, "ivory4") \
        o(4701, 0x6495ED, "cornflowerblue") \
        o(4711, 0x00FF7F, "springgreen1") \
        o(4712, 0x00EE76, "springgreen2") \
        o(4713, 0x00CD66, "springgreen3") \
        o(4714, 0x008B45, "springgreen4") \
        o(4746, 0x000000, "black") \
        o(4801, 0xFFFACD, "lemon chiffon") \
        o(4803, 0x9B30FF, "purple1") \
        o(4804, 0x912CEE, "purple2") \
        o(4805, 0x7D26CD, "purple3") \
        o(4806, 0x551A8B, "purple4") \
        o(4870, 0xFFE4E1, "mistyrose") \
        o(4907, 0xDAA520, "goldenrod") \
        o(4925, 0x0000CD, "medium blue") \
        o(4929, 0xFF3030, "firebrick1") \
        o(4930, 0xEE2C2C, "firebrick2") \
        o(4931, 0xCD2626, "firebrick3") \
        o(4932, 0x8B1A1A, "firebrick4") \
        o(4987, 0xFF69B4, "hot pink") \
        o(5011, 0x00FA9A, "mediumspringgreen") \
        o(5028, 0xBC8F8F, "rosy brown") \
        o(5037, 0xF08080, "lightcoral") \
        o(5048, 0xFAEBD7, "antiquewhite") \
        o(5093, 0xFFB6C1, "lightpink") \
        o(5146, 0x000080, "navyblue") \
        o(5165, 0x00FF00, "green") \
        o(5169, 0xCD5C5C, "indianred") \
        o(5170, 0x556B2F, "dark olive green") \
        o(5171, 0x7F7F7F, "gray50") \
        o(5172, 0x828282, "gray51") \
        o(5173, 0x858585, "gray52") \
        o(5174, 0x878787, "gray53") \
        o(5175, 0x8A8A8A, "gray54") \
        o(5176, 0x8C8C8C, "gray55") \
        o(5177, 0x8F8F8F, "gray56") \
        o(5178, 0x919191, "gray57") \
        o(5179, 0x949494, "gray58") \
        o(5180, 0x969696, "gray59") \
        o(5190, 0x20B2AA, "light sea green") \
        o(5226, 0x008B8B, "darkcyan") \
        o(5227, 0xDB7093, "palevioletred") \
        o(5231, 0x98F5FF, "cadetblue1") \
        o(5232, 0x8EE5EE, "cadetblue2") \
        o(5233, 0x7AC5CD, "cadetblue3") \
        o(5234, 0x53868B, "cadetblue4") \
        o(5250, 0xF5F5DC, "beige") \
        o(5283, 0x9AFF9A, "palegreen1") \
        o(5284, 0x90EE90, "palegreen2") \
        o(5285, 0x7CCD7C, "palegreen3") \
        o(5286, 0x548B54, "palegreen4") \
        o(5291, 0xC71585, "medium violet red") \
        o(5303, 0x87CEFA, "lightskyblue") \
        o(5308, 0x66CDAA, "mediumaquamarine") \
        o(5335, 0xFFFAFA, "snow1") \
        o(5336, 0xEEE9E9, "snow2") \
        o(5337, 0xCDC9C9, "snow3") \
        o(5338, 0x8B8989, "snow4") \
        o(5374, 0x228B22, "forestgreen") \
        o(5378, 0xFFAEB9, "lightpink1") \
        o(5379, 0xEEA2AD, "lightpink2") \
        o(5380, 0xCD8C95, "lightpink3") \
        o(5381, 0x8B5F65, "lightpink4") \
        o(5408, 0xFF0000, "red1") \
        o(5409, 0xEE0000, "red2") \
        o(5410, 0xCD0000, "red3") \
        o(5411, 0x8B0000, "red4") \
        o(5419, 0x006400, "dark green") \
        o(5421, 0x9ACD32, "yellowgreen") \
        o(5432, 0xD70751, "debianred") \
        o(5547, 0xFF34B3, "maroon1") \
        o(5548, 0xEE30A7, "maroon2") \
        o(5549, 0xCD2990, "maroon3") \
        o(5550, 0x8B1C62, "maroon4") \
        o(5561, 0xC6E2FF, "slategray1") \
        o(5562, 0xB9D3EE, "slategray2") \
        o(5563, 0x9FB6CD, "slategray3") \
        o(5564, 0x6C7B8B, "slategray4") \
        o(5590, 0x2E8B57, "sea green") \
        o(5594, 0xFFFFFF, "grey100") \
        o(5597, 0x7B68EE, "medium slate blue") \
        o(5633, 0x4876FF, "royalblue1") \
        o(5634, 0x436EEE, "royalblue2") \
        o(5635, 0x3A5FCD, "royalblue3") \
        o(5636, 0x27408B, "royalblue4") \
        o(5704, 0xFDF5E6, "old lace") \
        o(5731, 0xFFDEAD, "navajo white") \
        o(5746, 0xC1FFC1, "darkseagreen1") \
        o(5747, 0xB4EEB4, "darkseagreen2") \
        o(5748, 0x9BCD9B, "darkseagreen3") \
        o(5749, 0x698B69, "darkseagreen4") \
        o(5776, 0xFF6347, "tomato") \
        o(5778, 0x191970, "midnightblue") \
        o(5788, 0x000080, "navy blue") \
        o(5810, 0xFFB90F, "darkgoldenrod1") \
        o(5811, 0xEEAD0E, "darkgoldenrod2") \
        o(5812, 0xCD950C, "darkgoldenrod3") \
        o(5813, 0x8B6508, "darkgoldenrod4") \
        o(5827, 0xFFDEAD, "navajowhite") \
        o(5844, 0x999999, "gray60") \
        o(5845, 0x9C9C9C, "gray61") \
        o(5846, 0x9E9E9E, "gray62") \
        o(5847, 0xA1A1A1, "gray63") \
        o(5848, 0xA3A3A3, "gray64") \
        o(5849, 0xA6A6A6, "gray65") \
        o(5850, 0xA8A8A8, "gray66") \
        o(5851, 0xABABAB, "gray67") \
        o(5852, 0xADADAD, "gray68") \
        o(5853, 0xB0B0B0, "gray69") \
        o(5864, 0xFFEFD5, "papayawhip") \
        o(5888, 0xFFFFE0, "lightyellow") \
        o(5911, 0x7CFC00, "lawngreen") \
        o(5965, 0xB0E0E6, "powderblue") \
        o(5974, 0xEE82EE, "violet") \
        o(5978, 0x7FFFD4, "aquamarine1") \
        o(5979, 0x76EEC6, "aquamarine2") \
        o(5980, 0x66CDAA, "aquamarine3") \
        o(5981, 0x458B74, "aquamarine4") \
        o(5990, 0xAFEEEE, "pale turquoise") \
        o(6013, 0x7B68EE, "mediumslateblue") \
        o(6041, 0xC0FF3E, "olivedrab1") \
        o(6042, 0xB3EE3A, "olivedrab2") \
        o(6043, 0x9ACD32, "olivedrab3") \
        o(6044, 0x698B22, "olivedrab4") \
        o(6057, 0x8A2BE2, "blueviolet") \
        o(6063, 0x2F4F4F, "dark slate grey") \
        o(6068, 0xFF4500, "orange red") \
        o(6089, 0xFF7256, "coral1") \
        o(6090, 0xEE6A50, "coral2") \
        o(6091, 0xCD5B45, "coral3") \
        o(6092, 0x8B3E2F, "coral4") \
        o(6096, 0x00FF00, "green1") \
        o(6097, 0x00EE00, "green2") \
        o(6098, 0x00CD00, "green3") \
        o(6099, 0x008B00, "green4") \
        o(6136, 0xFFEBCD, "blanchedalmond") \
        o(6141, 0x4682B4, "steel blue") \
        o(6147, 0xFFDAB9, "peachpuff1") \
        o(6148, 0xEECBAD, "peachpuff2") \
        o(6149, 0xCDAF95, "peachpuff3") \
        o(6150, 0x8B7765, "peachpuff4") \
        o(6161, 0xFFFF00, "yellow1") \
        o(6162, 0xEEEE00, "yellow2") \
        o(6163, 0xCDCD00, "yellow3") \
        o(6164, 0x8B8B00, "yellow4") \
        o(6207, 0x40E0D0, "turquoise") \
        o(6223, 0xB0E0E6, "powder blue") \
        o(6278, 0xB0E2FF, "lightskyblue1") \
        o(6279, 0xA4D3EE, "lightskyblue2") \
        o(6280, 0x8DB6CD, "lightskyblue3") \
        o(6281, 0x607B8B, "lightskyblue4") \
        o(6282, 0xFF7F24, "chocolate1") \
        o(6283, 0xEE7621, "chocolate2") \
        o(6284, 0xCD661D, "chocolate3") \
        o(6285, 0x8B4513, "chocolate4") \
        o(6292, 0xFF8C00, "darkorange") \
        o(6316, 0x696969, "dimgrey") \
        o(6372, 0xDEB887, "burlywood") \
        o(6374, 0xFF8C69, "salmon1") \
        o(6375, 0xEE8262, "salmon2") \
        o(6376, 0xCD7054, "salmon3") \
        o(6377, 0x8B4C39, "salmon4") \
        o(6503, 0xEEE8AA, "palegoldenrod") \
        o(6515, 0xC71585, "mediumvioletred") \
        o(6542, 0x778899, "lightslategrey") \
        o(6547, 0xD3D3D3, "lightgray") \
        o(6551, 0xFFFAF0, "floral white") \
        o(6563, 0x00FF7F, "springgreen") \
        o(6584, 0xFFF0F5, "lavenderblush1") \
        o(6585, 0xEEE0E5, "lavenderblush2") \
        o(6586, 0xCDC1C5, "lavenderblush3") \
        o(6587, 0x8B8386, "lavenderblush4") \
        o(6595, 0x778899, "lightslategray") \
        o(6608, 0xE5E5E5, "grey90") \
        o(6609, 0xE8E8E8, "grey91") \
        o(6610, 0xEBEBEB, "grey92") \
        o(6611, 0xEDEDED, "grey93") \
        o(6612, 0xF0F0F0, "grey94") \
        o(6613, 0xF2F2F2, "grey95") \
        o(6614, 0xF5F5F5, "grey96") \
        o(6615, 0xF7F7F7, "grey97") \
        o(6616, 0xFAFAFA, "grey98") \
        o(6617, 0xFCFCFC, "grey99") \
        o(6619, 0xF08080, "light coral") \
        o(6702, 0x6B8E23, "olive drab") \
        o(6707, 0xF0E68C, "khaki") \
        o(6721, 0xFFD700, "gold") \
        o(6742, 0x778899, "light slate grey") \
        o(6762, 0xFFFFF0, "ivory") \
        o(6780, 0x9400D3, "dark violet") \
        o(6803, 0x98FB98, "palegreen") \
        o(6815, 0x87CEEB, "sky blue") \
        o(6859, 0x483D8B, "dark slate blue") \
        o(6884, 0xFAEBD7, "antique white") \
        o(6922, 0xFFF0F5, "lavender blush") \
        o(6939, 0xFF83FA, "orchid1") \
        o(6940, 0xEE7AE9, "orchid2") \
        o(6941, 0xCD69C9, "orchid3") \
        o(6942, 0x8B4789, "orchid4") \
        o(6953, 0xA0522D, "sienna") \
        o(7024, 0xFF00FF, "magenta") \
        o(7029, 0x1A1A1A, "gray10") \
        o(7030, 0x1C1C1C, "gray11") \
        o(7031, 0x1F1F1F, "gray12") \
        o(7032, 0x212121, "gray13") \
        o(7033, 0x242424, "gray14") \
        o(7034, 0x262626, "gray15") \
        o(7035, 0x292929, "gray16") \
        o(7036, 0x2B2B2B, "gray17") \
        o(7037, 0x2E2E2E, "gray18") \
        o(7038, 0x303030, "gray19") \
        o(7074, 0x00BFFF, "deepskyblue") \
        o(7097, 0xFFEFDB, "antiquewhite1") \
        o(7098, 0xEEDFCC, "antiquewhite2") \
        o(7099, 0xCDC0B0, "antiquewhite3") \
        o(7100, 0x8B8378, "antiquewhite4") \
        o(7120, 0xBFEFFF, "lightblue1") \
        o(7121, 0xB2DFEE, "lightblue2") \
        o(7122, 0x9AC0CD, "lightblue3") \
        o(7123, 0x68838B, "lightblue4") \
        o(7127, 0xCAE1FF, "lightsteelblue1") \
        o(7128, 0xBCD2EE, "lightsteelblue2") \
        o(7129, 0xA2B5CD, "lightsteelblue3") \
        o(7130, 0x6E7B8B, "lightsteelblue4") \
        o(7183, 0x0000FF, "blue1") \
        o(7184, 0x0000EE, "blue2") \
        o(7185, 0x0000CD, "blue3") \
        o(7186, 0x00008B, "blue4") \
        o(7189, 0xB8860B, "dark goldenrod") \
        o(7210, 0x708090, "slategray") \
        o(7222, 0x6A5ACD, "slate blue") \
        o(7257, 0xF5F5F5, "whitesmoke") \
        o(7284, 0xCAFF70, "darkolivegreen1") \
        o(7285, 0xBCEE68, "darkolivegreen2") \
        o(7286, 0xA2CD5A, "darkolivegreen3") \
        o(7287, 0x6E8B3D, "darkolivegreen4") \
        o(7297, 0x666666, "grey40") \
        o(7298, 0x696969, "grey41") \
        o(7299, 0x6B6B6B, "grey42") \
        o(7300, 0x6E6E6E, "grey43") \
        o(7301, 0x707070, "grey44") \
        o(7302, 0x737373, "grey45") \
        o(7303, 0x757575, "grey46") \
        o(7304, 0x787878, "grey47") \
        o(7305, 0x7A7A7A, "grey48") \
        o(7306, 0x7D7D7D, "grey49") \
        o(7372, 0xFFD700, "gold1") \
        o(7373, 0xEEC900, "gold2") \
        o(7374, 0xCDAD00, "gold3") \
        o(7375, 0x8B7500, "gold4") \
        o(7376, 0x9370DB, "mediumpurple") \
        o(7398, 0xFFC125, "goldenrod1") \
        o(7399, 0xEEB422, "goldenrod2") \
        o(7400, 0xCD9B1D, "goldenrod3") \
        o(7401, 0x8B6914, "goldenrod4") \
        o(7429, 0xFDF5E6, "oldlace") \
        o(7464, 0xFF4500, "orangered1") \
        o(7465, 0xEE4000, "orangered2") \
        o(7466, 0xCD3700, "orangered3") \
        o(7467, 0x8B2500, "orangered4") \
        o(7475, 0xFAFAD2, "light goldenrod yellow") \
        o(7477, 0xFFEC8B, "lightgoldenrod1") \
        o(7478, 0xEEDC82, "lightgoldenrod2") \
        o(7479, 0xCDBE70, "lightgoldenrod3") \
        o(7480, 0x8B814C, "lightgoldenrod4") \
        o(7535, 0x6B8E23, "olivedrab") \
        o(7604, 0x00008B, "dark blue") \
        o(7619, 0xBDB76B, "dark khaki") \
        o(7665, 0x696969, "dimgray") \
        o(7666, 0x836FFF, "slateblue1") \
        o(7667, 0x7A67EE, "slateblue2") \
        o(7668, 0x6959CD, "slateblue3") \
        o(7669, 0x473C8B, "slateblue4") \

#define v(a,b,s) static_assert(hash(s, sizeof(s)-1) == a);
// ^ Verify that the hashes in o() match the implementation in hash()
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
    if(h < mod)
    {
        auto i = std::lower_bound(std::begin(colorkeys), std::end(colorkeys), h);
        if(i != std::end(colorkeys) && *i == h)
            return colorvalues[ i-std::begin(colorkeys) ];
    }
#endif
    return 0;
}
