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

#define docolors(o) \
        o(4, 0x8B6508) /*DARKGOLDENROD4*/ \
        o(12, 0x668B8B) /*PALETURQUOISE4*/ \
        o(14, 0xF0FFFF) /*AZURE*/ \
        o(42, 0xF0F0F0) /*GREY94*/ \
        o(43, 0xEE3B3B) /*BROWN2*/ \
        o(64, 0xCD3278) /*VIOLETRED3*/ \
        o(77, 0xCDC0B0) /*ANTIQUEWHITE3*/ \
        o(90, 0xF7F7F7) /*GRAY97*/ \
        o(127, 0x98FB98) /*PALEGREEN*/ \
        o(129, 0xD9D9D9) /*GREY85*/ \
        o(134, 0x737373) /*GREY45*/ \
        o(140, 0x00E5EE) /*TURQUOISE2*/ \
        o(148, 0x00CD66) /*SPRINGGREEN3*/ \
        o(162, 0xFFE4C4) /*BISQUE1*/ \
        o(170, 0x548B54) /*PALEGREEN4*/ \
        o(177, 0xA1A1A1) /*GREY63*/ \
        o(185, 0x000000) /*GREY0*/ \
        o(223, 0x000080) /*NAVY*/ \
        o(251, 0xEE7942) /*SIENNA2*/ \
        o(255, 0x8B636C) /*PINK4*/ \
        o(296, 0x778899) /*LIGHT SLATE GREY*/ \
        o(308, 0x949494) /*GREY58*/ \
        o(330, 0xCD661D) /*CHOCOLATE3*/ \
        o(335, 0x4682B4) /*STEELBLUE*/ \
        o(339, 0x8B4513) /*SADDLEBROWN*/ \
        o(344, 0xE8E8E8) /*GRAY91*/ \
        o(413, 0xCD0000) /*RED3*/ \
        o(428, 0xEEC591) /*BURLYWOOD2*/ \
        o(446, 0x262626) /*GREY15*/ \
        o(461, 0xFAFAFA) /*GRAY98*/ \
        o(463, 0xE0FFFF) /*LIGHTCYAN1*/ \
        o(469, 0xB452CD) /*MEDIUMORCHID3*/ \
        o(471, 0x8B008B) /*DARKMAGENTA*/ \
        o(479, 0x8470FF) /*LIGHT SLATE BLUE*/ \
        o(483, 0x8B7B8B) /*THISTLE4*/ \
        o(505, 0x757575) /*GREY46*/ \
        o(522, 0xB0E2FF) /*LIGHTSKYBLUE1*/ \
        o(539, 0xFF00FF) /*MAGENTA1*/ \
        o(592, 0xBABABA) /*GREY73*/ \
        o(597, 0x575757) /*GREY34*/ \
        o(600, 0xEE2C2C) /*FIREBRICK2*/ \
        o(628, 0xFFFF00) /*YELLOW1*/ \
        o(659, 0xEE799F) /*PALEVIOLETRED2*/ \
        o(693, 0xCDAF95) /*PEACHPUFF3*/ \
        o(740, 0xADD8E6) /*LIGHTBLUE*/ \
        o(742, 0xEE7600) /*DARKORANGE2*/ \
        o(745, 0x0000FF) /*BLUE1*/ \
        o(761, 0x8B0000) /*DARK RED*/ \
        o(766, 0xA3A3A3) /*GRAY64*/ \
        o(799, 0xFFD39B) /*BURLYWOOD1*/ \
        o(803, 0xD3D3D3) /*LIGHTGREY*/ \
        o(805, 0xA9A9A9) /*DARK GREY*/ \
        o(814, 0x8B4513) /*SADDLE BROWN*/ \
        o(817, 0x292929) /*GREY16*/ \
        o(854, 0xCDB5CD) /*THISTLE3*/ \
        o(882, 0x00868B) /*TURQUOISE4*/ \
        o(911, 0x00FFFF) /*CYAN*/ \
        o(912, 0x90EE90) /*PALEGREEN2*/ \
        o(922, 0x000080) /*NAVY BLUE*/ \
        o(929, 0xFFFFFF) /*GRAY100*/ \
        o(951, 0x48D1CC) /*MEDIUMTURQUOISE*/ \
        o(963, 0xC2C2C2) /*GREY76*/ \
        o(966, 0xF8F8FF) /*GHOSTWHITE*/ \
        o(968, 0x5E5E5E) /*GREY37*/ \
        o(971, 0xFF3030) /*FIREBRICK1*/ \
        o(984, 0xF8F8FF) /*GHOST WHITE*/ \
        o(999, 0xEEEE00) /*YELLOW2*/ \
        o(1009, 0xFF8C00) /*DARK ORANGE*/ \
        o(1013, 0x00CED1) /*DARKTURQUOISE*/ \
        o(1055, 0x333333) /*GREY20*/ \
        o(1064, 0x8B7765) /*PEACHPUFF4*/ \
        o(1069, 0x8B814C) /*LIGHTGOLDENROD4*/ \
        o(1082, 0xBA55D3) /*MEDIUMORCHID*/ \
        o(1083, 0x006400) /*DARKGREEN*/ \
        o(1092, 0xFFA54F) /*TAN1*/ \
        o(1113, 0xFF7F00) /*DARKORANGE1*/ \
        o(1137, 0x9C9C9C) /*GRAY61*/ \
        o(1164, 0xEE00EE) /*MAGENTA2*/ \
        o(1182, 0x1E90FF) /*DODGERBLUE*/ \
        o(1188, 0x212121) /*GREY13*/ \
        o(1200, 0xEEE5DE) /*SEASHELL2*/ \
        o(1203, 0x4169E1) /*ROYAL BLUE*/ \
        o(1204, 0xBDB76B) /*DARKKHAKI*/ \
        o(1222, 0x4F4F4F) /*GREY31*/ \
        o(1229, 0x1F1F1F) /*GRAY12*/ \
        o(1232, 0x8B8B7A) /*LIGHTYELLOW4*/ \
        o(1236, 0x00CD00) /*GREEN3*/ \
        o(1244, 0xFFFF00) /*YELLOW*/ \
        o(1284, 0xFF82AB) /*PALEVIOLETRED1*/ \
        o(1291, 0x8A8A8A) /*GREY54*/ \
        o(1341, 0x828282) /*GRAY51*/ \
        o(1348, 0xE3E3E3) /*GRAY89*/ \
        o(1373, 0x4876FF) /*ROYALBLUE1*/ \
        o(1381, 0xFFA07A) /*LIGHT SALMON*/ \
        o(1401, 0xA2CD5A) /*DARKOLIVEGREEN3*/ \
        o(1412, 0x9A32CD) /*DARKORCHID3*/ \
        o(1419, 0x2F4F4F) /*DARK SLATE GREY*/ \
        o(1422, 0xEE9A00) /*ORANGE2*/ \
        o(1426, 0x404040) /*GREY25*/ \
        o(1427, 0x696969) /*DIMGREY*/ \
        o(1446, 0x556B2F) /*DARK OLIVE GREEN*/ \
        o(1449, 0x636363) /*GRAY39*/ \
        o(1455, 0xFFA500) /*ORANGE*/ \
        o(1463, 0x8B5A2B) /*TAN4*/ \
        o(1470, 0x778899) /*LIGHTSLATEGRAY*/ \
        o(1502, 0x32CD32) /*LIME GREEN*/ \
        o(1508, 0x9E9E9E) /*GRAY62*/ \
        o(1539, 0xF0F8FF) /*ALICE BLUE*/ \
        o(1566, 0x4D4D4D) /*GRAY30*/ \
        o(1582, 0x00FF7F) /*SPRING GREEN*/ \
        o(1595, 0x919191) /*GRAY57*/ \
        o(1600, 0x2B2B2B) /*GRAY17*/ \
        o(1603, 0xFFFFE0) /*LIGHTYELLOW1*/ \
        o(1634, 0xFFDEAD) /*NAVAJOWHITE*/ \
        o(1676, 0x8B5A00) /*ORANGE4*/ \
        o(1688, 0xFFFACD) /*LEMONCHIFFON*/ \
        o(1690, 0xA9A9A9) /*DARK GRAY*/ \
        o(1711, 0xD1D1D1) /*GREY82*/ \
        o(1720, 0xCDC9C9) /*SNOW3*/ \
        o(1744, 0x436EEE) /*ROYALBLUE2*/ \
        o(1764, 0xEEE8CD) /*CORNSILK2*/ \
        o(1768, 0x8B5F65) /*LIGHTPINK4*/ \
        o(1772, 0x6E8B3D) /*DARKOLIVEGREEN4*/ \
        o(1797, 0x424242) /*GREY26*/ \
        o(1806, 0x00FA9A) /*MEDIUMSPRINGGREEN*/ \
        o(1830, 0x9932CC) /*DARKORCHID*/ \
        o(1836, 0xD9D9D9) /*GRAY85*/ \
        o(1879, 0xFFEBCD) /*BLANCHED ALMOND*/ \
        o(1881, 0x8B0000) /*DARKRED*/ \
        o(1916, 0x828282) /*GREY51*/ \
        o(1920, 0x43CD80) /*SEAGREEN3*/ \
        o(1923, 0xBFBFBF) /*GRAY75*/ \
        o(1928, 0x4682B4) /*STEEL BLUE*/ \
        o(1930, 0x303030) /*GREY19*/ \
        o(1934, 0xB03060) /*MAROON*/ \
        o(1937, 0x595959) /*GRAY35*/ \
        o(1948, 0x8B7E66) /*WHEAT4*/ \
        o(1955, 0xFAEBD7) /*ANTIQUE WHITE*/ \
        o(1966, 0x858585) /*GRAY52*/ \
        o(1971, 0x242424) /*GRAY14*/ \
        o(1981, 0x87CEFF) /*SKYBLUE1*/ \
        o(2018, 0x8B8878) /*CORNSILK4*/ \
        o(2051, 0xEEEEE0) /*IVORY2*/ \
        o(2060, 0xEE82EE) /*VIOLET*/ \
        o(2065, 0x6B8E23) /*OLIVE DRAB*/ \
        o(2085, 0x121212) /*GRAY7*/ \
        o(2090, 0xD4D4D4) /*GRAY83*/ \
        o(2102, 0x4F94CD) /*STEELBLUE3*/ \
        o(2111, 0x7A7A7A) /*GRAY48*/ \
        o(2117, 0xFFDEAD) /*NAVAJOWHITE1*/ \
        o(2129, 0xF5FFFA) /*MINTCREAM*/ \
        o(2139, 0xCD8C95) /*LIGHTPINK3*/ \
        o(2155, 0xFFFFFF) /*WHITE*/ \
        o(2174, 0xBDBDBD) /*GREY74*/ \
        o(2227, 0xADD8E6) /*LIGHT BLUE*/ \
        o(2235, 0x5F9EA0) /*CADETBLUE*/ \
        o(2249, 0xFF8C00) /*DARKORANGE*/ \
        o(2254, 0xADADAD) /*GREY68*/ \
        o(2291, 0xFF7F50) /*CORAL*/ \
        o(2294, 0xB3B3B3) /*GRAY70*/ \
        o(2308, 0x5C5C5C) /*GRAY36*/ \
        o(2319, 0xFFE7BA) /*WHEAT1*/ \
        o(2348, 0x2F4F4F) /*DARKSLATEGRAY*/ \
        o(2349, 0x8B864E) /*KHAKI4*/ \
        o(2376, 0xEE1289) /*DEEPPINK2*/ \
        o(2379, 0xFFA07A) /*LIGHTSALMON1*/ \
        o(2382, 0x8A2BE2) /*BLUE VIOLET*/ \
        o(2386, 0x3B3B3B) /*GRAY23*/ \
        o(2417, 0x708090) /*SLATE GRAY*/ \
        o(2422, 0xFFFFF0) /*IVORY1*/ \
        o(2438, 0xFFE4E1) /*MISTYROSE*/ \
        o(2443, 0x708090) /*SLATE GREY*/ \
        o(2453, 0xE0E0E0) /*GREY88*/ \
        o(2456, 0x0A0A0A) /*GRAY4*/ \
        o(2461, 0xDBDBDB) /*GRAY86*/ \
        o(2471, 0x87CEEB) /*SKYBLUE*/ \
        o(2488, 0xEECFA1) /*NAVAJOWHITE2*/ \
        o(2496, 0xB2DFEE) /*LIGHTBLUE2*/ \
        o(2500, 0xCD3700) /*ORANGERED3*/ \
        o(2565, 0x9932CC) /*DARK ORCHID*/ \
        o(2572, 0x7B68EE) /*MEDIUMSLATEBLUE*/ \
        o(2573, 0x000080) /*NAVYBLUE*/ \
        o(2606, 0xCAE1FF) /*LIGHTSTEELBLUE1*/ \
        o(2627, 0xCD9B1D) /*GOLDENROD3*/ \
        o(2656, 0xE0EEEE) /*AZURE2*/ \
        o(2665, 0xBABABA) /*GRAY73*/ \
        o(2667, 0x00BFFF) /*DEEPSKYBLUE1*/ \
        o(2700, 0x7AC5CD) /*CADETBLUE3*/ \
        o(2717, 0x79CDCD) /*DARKSLATEGRAY3*/ \
        o(2734, 0xFF6EB4) /*HOTPINK1*/ \
        o(2744, 0xE6E6FA) /*LAVENDER*/ \
        o(2750, 0xEE9572) /*LIGHTSALMON2*/ \
        o(2757, 0x424242) /*GRAY26*/ \
        o(2778, 0x8B4C39) /*SALMON4*/ \
        o(2783, 0xBA55D3) /*MEDIUM ORCHID*/ \
        o(2805, 0x7CFC00) /*LAWNGREEN*/ \
        o(2825, 0x66CD00) /*CHARTREUSE3*/ \
        o(2827, 0x030303) /*GRAY1*/ \
        o(2832, 0xEEE9BF) /*LEMONCHIFFON2*/ \
        o(2837, 0x303030) /*GRAY19*/ \
        o(2868, 0x6C7B8B) /*SLATEGRAY4*/ \
        o(2870, 0xCDC1C5) /*LAVENDERBLUSH3*/ \
        o(2974, 0xDA70D6) /*ORCHID*/ \
        o(2977, 0x6E7B8B) /*LIGHTSTEELBLUE4*/ \
        o(2996, 0xA8A8A8) /*GREY66*/ \
        o(3004, 0x0D0D0D) /*GREY5*/ \
        o(3027, 0xF0FFFF) /*AZURE1*/ \
        o(3054, 0xC1FFC1) /*DARKSEAGREEN1*/ \
        o(3071, 0x53868B) /*CADETBLUE4*/ \
        o(3091, 0xEEE685) /*KHAKI2*/ \
        o(3094, 0x707070) /*GRAY44*/ \
        o(3105, 0xEE6AA7) /*HOTPINK2*/ \
        o(3128, 0x404040) /*GRAY25*/ \
        o(3144, 0x473C8B) /*SLATEBLUE4*/ \
        o(3148, 0x1874CD) /*DODGERBLUE3*/ \
        o(3149, 0xFF8C69) /*SALMON1*/ \
        o(3196, 0x458B00) /*CHARTREUSE4*/ \
        o(3202, 0xAEEEEE) /*PALETURQUOISE2*/ \
        o(3203, 0xFFFACD) /*LEMONCHIFFON1*/ \
        o(3207, 0x7A7A7A) /*GREY48*/ \
        o(3226, 0x7D26CD) /*PURPLE3*/ \
        o(3232, 0xF5F5F5) /*GREY96*/ \
        o(3239, 0xC6E2FF) /*SLATEGRAY1*/ \
        o(3243, 0xDB7093) /*PALE VIOLET RED*/ \
        o(3245, 0x9ACD32) /*YELLOWGREEN*/ \
        o(3249, 0xFF6347) /*TOMATO*/ \
        o(3251, 0xCD5B45) /*CORAL3*/ \
        o(3271, 0xEEB4B4) /*ROSYBROWN2*/ \
        o(3276, 0x8968CD) /*MEDIUMPURPLE3*/ \
        o(3354, 0xCD5C5C) /*INDIAN RED*/ \
        o(3359, 0x8B3A62) /*HOTPINK4*/ \
        o(3361, 0xA52A2A) /*BROWN*/ \
        o(3367, 0x9C9C9C) /*GREY61*/ \
        o(3375, 0x0F0F0F) /*GREY6*/ \
        o(3398, 0x838B8B) /*AZURE4*/ \
        o(3428, 0xFFFFE0) /*LIGHTYELLOW*/ \
        o(3445, 0xEEA9B8) /*PINK2*/ \
        o(3453, 0x3CB371) /*MEDIUM SEA GREEN*/ \
        o(3468, 0x32CD32) /*LIMEGREEN*/ \
        o(3469, 0xEE7AE9) /*ORCHID2*/ \
        o(3498, 0x8F8F8F) /*GREY56*/ \
        o(3499, 0x474747) /*GRAY28*/ \
        o(3503, 0xFF69B4) /*HOTPINK*/ \
        o(3535, 0x87CEEB) /*SKY BLUE*/ \
        o(3564, 0x9ACD32) /*YELLOW GREEN*/ \
        o(3565, 0xCD950C) /*DARKGOLDENROD3*/ \
        o(3574, 0x8B8970) /*LEMONCHIFFON4*/ \
        o(3575, 0xFF4500) /*ORANGERED*/ \
        o(3611, 0xFFA07A) /*LIGHTSALMON*/ \
        o(3628, 0x7FFF00) /*CHARTREUSE*/ \
        o(3653, 0xB4CDCD) /*LIGHTCYAN3*/ \
        o(3670, 0x636363) /*GREY39*/ \
        o(3682, 0x8B7D7B) /*MISTYROSE4*/ \
        o(3687, 0xFFFFFF) /*GREY100*/ \
        o(3690, 0xDBDBDB) /*GREY86*/ \
        o(3691, 0x66CDAA) /*MEDIUMAQUAMARINE*/ \
        o(3701, 0x00F5FF) /*TURQUOISE1*/ \
        o(3713, 0xDEB887) /*BURLYWOOD*/ \
        o(3719, 0x696969) /*GRAY41*/ \
        o(3746, 0x080808) /*GREY3*/ \
        o(3750, 0xBC8F8F) /*ROSY BROWN*/ \
        o(3825, 0xAFEEEE) /*PALETURQUOISE*/ \
        o(3839, 0xF5DEB3) /*WHEAT*/ \
        o(3840, 0xFF83FA) /*ORCHID1*/ \
        o(3857, 0xE8E8E8) /*GREY91*/ \
        o(3869, 0x969696) /*GREY59*/ \
        o(3878, 0x009ACD) /*DEEPSKYBLUE3*/ \
        o(3882, 0x90EE90) /*LIGHT GREEN*/ \
        o(3883, 0xFFDAB9) /*PEACHPUFF1*/ \
        o(3886, 0x7A67EE) /*SLATEBLUE2*/ \
        o(3905, 0xE5E5E5) /*GRAY90*/ \
        o(3911, 0xDDA0DD) /*PLUM*/ \
        o(3932, 0x8B4500) /*DARKORANGE4*/ \
        o(3933, 0xFFF5EE) /*SEASHELL*/ \
        o(3974, 0xEE0000) /*RED2*/ \
        o(3977, 0x8B7D6B) /*BISQUE4*/ \
        o(3985, 0x6495ED) /*CORNFLOWER BLUE*/ \
        o(4004, 0x474747) /*GREY28*/ \
        o(4007, 0x242424) /*GREY14*/ \
        o(4070, 0xFFB5C5) /*PINK1*/ \
        o(4083, 0xA4D3EE) /*LIGHTSKYBLUE2*/ \
        o(4110, 0xF5F5F5) /*WHITESMOKE*/ \
        o(4137, 0xCDAD00) /*GOLD3*/ \
        o(4146, 0x9370DB) /*MEDIUMPURPLE*/ \
        o(4153, 0xB3B3B3) /*GREY70*/ \
        o(4202, 0x778899) /*LIGHT SLATE GRAY*/ \
        o(4221, 0xFFE4C4) /*BISQUE*/ \
        o(4247, 0xC7C7C7) /*GRAY78*/ \
        o(4248, 0xFFFACD) /*LEMON CHIFFON*/ \
        o(4254, 0xEECBAD) /*PEACHPUFF2*/ \
        o(4263, 0xB0C4DE) /*LIGHT STEEL BLUE*/ \
        o(4276, 0xF2F2F2) /*GRAY95*/ \
        o(4290, 0x969696) /*GRAY59*/ \
        o(4303, 0xCD6600) /*DARKORANGE3*/ \
        o(4320, 0x6E6E6E) /*GREY43*/ \
        o(4327, 0xABABAB) /*GRAY67*/ \
        o(4378, 0x1C1C1C) /*GREY11*/ \
        o(4390, 0x8B8682) /*SEASHELL4*/ \
        o(4439, 0x7FFFD4) /*AQUAMARINE*/ \
        o(4462, 0xFF7256) /*CORAL1*/ \
        o(4482, 0xD2B48C) /*TAN*/ \
        o(4506, 0xEEE8AA) /*PALEGOLDENROD*/ \
        o(4508, 0x8B7500) /*GOLD4*/ \
        o(4524, 0xC4C4C4) /*GREY77*/ \
        o(4538, 0xE0FFFF) /*LIGHT CYAN*/ \
        o(4581, 0xB0B0B0) /*GRAY69*/ \
        o(4584, 0xF5F5F5) /*WHITE SMOKE*/ \
        o(4591, 0xCAFF70) /*DARKOLIVEGREEN1*/ \
        o(4600, 0xCD3333) /*BROWN3*/ \
        o(4608, 0x66CDAA) /*AQUAMARINE3*/ \
        o(4614, 0x8B7355) /*BURLYWOOD4*/ \
        o(4616, 0x3B3B3B) /*GREY23*/ \
        o(4621, 0xEE3A8C) /*VIOLETRED2*/ \
        o(4647, 0xF5F5F5) /*GRAY96*/ \
        o(4653, 0xEE9A49) /*TAN2*/ \
        o(4690, 0xF0E68C) /*KHAKI*/ \
        o(4691, 0x707070) /*GREY44*/ \
        o(4698, 0x999999) /*GRAY60*/ \
        o(4704, 0x00BFFF) /*DEEP SKY BLUE*/ \
        o(4716, 0x8B0000) /*RED4*/ \
        o(4719, 0xEED5B7) /*BISQUE2*/ \
        o(4725, 0xCD00CD) /*MAGENTA3*/ \
        o(4732, 0xD3D3D3) /*LIGHT GRAY*/ \
        o(4749, 0x1F1F1F) /*GREY12*/ \
        o(4761, 0xCDC5BF) /*SEASHELL3*/ \
        o(4774, 0x696969) /*DIM GREY*/ \
        o(4783, 0x525252) /*GREY32*/ \
        o(4786, 0x8B1A1A) /*FIREBRICK4*/ \
        o(4793, 0xCDCDB4) /*LIGHTYELLOW3*/ \
        o(4797, 0x00EE00) /*GREEN2*/ \
        o(4800, 0x4A708B) /*SKYBLUE4*/ \
        o(4811, 0xFFC0CB) /*PINK*/ \
        o(4833, 0x8B3E2F) /*CORAL4*/ \
        o(4870, 0xFAEBD7) /*ANTIQUEWHITE*/ \
        o(4891, 0xBEBEBE) /*GREY*/ \
        o(4925, 0xCD2990) /*MAROON3*/ \
        o(4931, 0x0000CD) /*BLUE3*/ \
        o(4955, 0x556B2F) /*DARKOLIVEGREEN*/ \
        o(4962, 0xBCEE68) /*DARKOLIVEGREEN2*/ \
        o(4975, 0x00FA9A) /*MEDIUM SPRING GREEN*/ \
        o(4983, 0x171717) /*GREY9*/ \
        o(4985, 0xCDAA7D) /*BURLYWOOD3*/ \
        o(4987, 0x3D3D3D) /*GREY24*/ \
        o(5005, 0x40E0D0) /*TURQUOISE*/ \
        o(5010, 0x616161) /*GRAY38*/ \
        o(5082, 0xCD4F39) /*TOMATO3*/ \
        o(5154, 0x595959) /*GREY35*/ \
        o(5156, 0x8A8A8A) /*GRAY54*/ \
        o(5157, 0xCD2626) /*FIREBRICK3*/ \
        o(5161, 0x292929) /*GRAY16*/ \
        o(5192, 0x00EEEE) /*CYAN2*/ \
        o(5197, 0xFAFAD2) /*LIGHT GOLDENROD YELLOW*/ \
        o(5227, 0xD3D3D3) /*LIGHT GREY*/ \
        o(5236, 0xFFDAB9) /*PEACH PUFF*/ \
        o(5241, 0x8B8B83) /*IVORY4*/ \
        o(5286, 0x20B2AA) /*LIGHTSEAGREEN*/ \
        o(5290, 0xCDB7B5) /*MISTYROSE3*/ \
        o(5296, 0x8B1C62) /*MAROON4*/ \
        o(5301, 0x757575) /*GRAY46*/ \
        o(5358, 0x4A4A4A) /*GREY29*/ \
        o(5359, 0xD02090) /*VIOLET RED*/ \
        o(5362, 0x696969) /*DIMGRAY*/ \
        o(5385, 0x778899) /*LIGHTSLATEGREY*/ \
        o(5417, 0xFF1493) /*DEEP PINK*/ \
        o(5455, 0x708090) /*SLATEGREY*/ \
        o(5469, 0x7CCD7C) /*PALEGREEN3*/ \
        o(5474, 0xFFF8DC) /*CORNSILK*/ \
        o(5477, 0x858585) /*GREY52*/ \
        o(5481, 0x4EEE94) /*SEAGREEN2*/ \
        o(5486, 0x00688B) /*DEEPSKYBLUE4*/ \
        o(5491, 0x2E2E2E) /*GREY18*/ \
        o(5504, 0xDCDCDC) /*GAINSBORO*/ \
        o(5509, 0xCDBA96) /*WHEAT3*/ \
        o(5527, 0x878787) /*GRAY53*/ \
        o(5539, 0x008B00) /*GREEN4*/ \
        o(5542, 0x7EC0EE) /*SKYBLUE2*/ \
        o(5547, 0xFFEFD5) /*PAPAYAWHIP*/ \
        o(5563, 0x00FFFF) /*CYAN1*/ \
        o(5612, 0xCDCDC1) /*IVORY3*/ \
        o(5651, 0xCCCCCC) /*GRAY80*/ \
        o(5652, 0x9400D3) /*DARK VIOLET*/ \
        o(5662, 0xE9967A) /*DARKSALMON*/ \
        o(5672, 0x7D7D7D) /*GRAY49*/ \
        o(5673, 0x7B68EE) /*MEDIUM SLATE BLUE*/ \
        o(5697, 0xDAA520) /*GOLDENROD*/ \
        o(5735, 0xEEAEEE) /*PLUM2*/ \
        o(5761, 0x228B22) /*FORESTGREEN*/ \
        o(5786, 0x1C1C1C) /*GRAY11*/ \
        o(5791, 0xFF8247) /*SIENNA1*/ \
        o(5794, 0xB0E0E6) /*POWDER BLUE*/ \
        o(5811, 0xFF4040) /*BROWN1*/ \
        o(5848, 0x8C8C8C) /*GREY55*/ \
        o(5855, 0xB5B5B5) /*GRAY71*/ \
        o(5875, 0xD02090) /*VIOLETRED*/ \
        o(5889, 0xF0FFF0) /*HONEYDEW*/ \
        o(5890, 0x98F5FF) /*CADETBLUE1*/ \
        o(5897, 0xCCCCCC) /*GREY80*/ \
        o(5905, 0xFF69B4) /*HOT PINK*/ \
        o(5934, 0x008B8B) /*CYAN4*/ \
        o(5936, 0xA020F0) /*PURPLE*/ \
        o(5937, 0xFF1493) /*DEEPPINK1*/ \
        o(5979, 0xCD8500) /*ORANGE3*/ \
        o(6014, 0xE3E3E3) /*GREY89*/ \
        o(6017, 0x080808) /*GRAY3*/ \
        o(6020, 0x616161) /*GREY38*/ \
        o(6022, 0xDEDEDE) /*GRAY87*/ \
        o(6083, 0xD2691E) /*CHOCOLATE*/ \
        o(6106, 0xFFBBFF) /*PLUM1*/ \
        o(6123, 0x545454) /*GRAY33*/ \
        o(6128, 0x006400) /*DARK GREEN*/ \
        o(6155, 0xB0E0E6) /*POWDERBLUE*/ \
        o(6167, 0xBCD2EE) /*LIGHTSTEELBLUE2*/ \
        o(6182, 0x8B2323) /*BROWN4*/ \
        o(6200, 0x9370DB) /*MEDIUM PURPLE*/ \
        o(6217, 0xC1CDCD) /*AZURE3*/ \
        o(6221, 0xFF1493) /*DEEPPINK*/ \
        o(6223, 0x2E8B57) /*SEAGREEN4*/ \
        o(6228, 0x00B2EE) /*DEEPSKYBLUE2*/ \
        o(6261, 0x8EE5EE) /*CADETBLUE2*/ \
        o(6267, 0x9400D3) /*DARKVIOLET*/ \
        o(6268, 0xD4D4D4) /*GREY83*/ \
        o(6271, 0x171717) /*GRAY9*/ \
        o(6288, 0x63B8FF) /*STEELBLUE1*/ \
        o(6305, 0xFCFCFC) /*GREY99*/ \
        o(6307, 0x8B008B) /*MAGENTA4*/ \
        o(6321, 0xCDC8B1) /*CORNSILK3*/ \
        o(6338, 0x1E90FF) /*DODGERBLUE1*/ \
        o(6339, 0xCD7054) /*SALMON3*/ \
        o(6386, 0x76EE00) /*CHARTREUSE2*/ \
        o(6388, 0x000000) /*GRAY0*/ \
        o(6393, 0xCDC9A5) /*LEMONCHIFFON3*/ \
        o(6397, 0xE0FFFF) /*LIGHTCYAN*/ \
        o(6400, 0xD8BFD8) /*THISTLE*/ \
        o(6416, 0x9B30FF) /*PURPLE1*/ \
        o(6429, 0x9FB6CD) /*SLATEGRAY3*/ \
        o(6461, 0x8B6969) /*ROSYBROWN4*/ \
        o(6477, 0x8B668B) /*PLUM4*/ \
        o(6480, 0xC2C2C2) /*GRAY76*/ \
        o(6487, 0xB0C4DE) /*LIGHTSTEELBLUE*/ \
        o(6494, 0x575757) /*GRAY34*/ \
        o(6505, 0x5F9EA0) /*CADET BLUE*/ \
        o(6517, 0x191970) /*MIDNIGHT BLUE*/ \
        o(6518, 0x6A5ACD) /*SLATE BLUE*/ \
        o(6562, 0x8B0A50) /*DEEPPINK4*/ \
        o(6587, 0xF4A460) /*SANDYBROWN*/ \
        o(6609, 0xCDBE70) /*LIGHTGOLDENROD3*/ \
        o(6615, 0xB4EEB4) /*DARKSEAGREEN2*/ \
        o(6620, 0x008B8B) /*DARKCYAN*/ \
        o(6621, 0xCD5555) /*INDIANRED3*/ \
        o(6642, 0x0F0F0F) /*GRAY6*/ \
        o(6652, 0xCDC673) /*KHAKI3*/ \
        o(6672, 0x6495ED) /*CORNFLOWERBLUE*/ \
        o(6682, 0x68838B) /*LIGHTBLUE4*/ \
        o(6685, 0x8B8386) /*LAVENDERBLUSH4*/ \
        o(6686, 0xA9A9A9) /*DARKGRAY*/ \
        o(6709, 0x1C86EE) /*DODGERBLUE2*/ \
        o(6731, 0xBFBFBF) /*GREY75*/ \
        o(6763, 0x96CDCD) /*PALETURQUOISE3*/ \
        o(6773, 0x228B22) /*FOREST GREEN*/ \
        o(6784, 0x2F4F4F) /*DARKSLATEGREY*/ \
        o(6787, 0x551A8B) /*PURPLE4*/ \
        o(6811, 0xB0B0B0) /*GREY69*/ \
        o(6828, 0xEEDFCC) /*ANTIQUEWHITE2*/ \
        o(6832, 0xFFC1C1) /*ROSYBROWN1*/ \
        o(6847, 0xC1CDC1) /*HONEYDEW3*/ \
        o(6869, 0x698B69) /*DARKSEAGREEN4*/ \
        o(6877, 0xD70751) /*DEBIANRED*/ \
        o(6899, 0xFFE4E1) /*MISTY ROSE*/ \
        o(6903, 0x97FFFF) /*DARKSLATEGRAY1*/ \
        o(6907, 0x66CDAA) /*MEDIUM AQUAMARINE*/ \
        o(6928, 0x9E9E9E) /*GREY62*/ \
        o(6930, 0x8B6914) /*GOLDENROD4*/ \
        o(6936, 0x030303) /*GREY1*/ \
        o(6943, 0x333333) /*GRAY20*/ \
        o(6963, 0x104E8B) /*DODGERBLUE4*/ \
        o(6968, 0xFFF5EE) /*SEASHELL1*/ \
        o(6992, 0x8B3A3A) /*INDIANRED4*/ \
        o(7011, 0x7FFF00) /*CHARTREUSE1*/ \
        o(7021, 0xFFB6C1) /*LIGHT PINK*/ \
        o(7043, 0x00008B) /*DARKBLUE*/ \
        o(7047, 0xEDEDED) /*GREY93*/ \
        o(7053, 0xBFEFFF) /*LIGHTBLUE1*/ \
        o(7056, 0xFFF0F5) /*LAVENDERBLUSH1*/ \
        o(7057, 0xEE4000) /*ORANGERED2*/ \
        o(7059, 0x919191) /*GREY57*/ \
        o(7060, 0x4A4A4A) /*GRAY29*/ \
        o(7099, 0xE9967A) /*DARK SALMON*/ \
        o(7102, 0xC7C7C7) /*GREY78*/ \
        o(7126, 0xEEAD0E) /*DARKGOLDENROD2*/ \
        o(7182, 0xA3A3A3) /*GREY64*/ \
        o(7184, 0xEEB422) /*GOLDENROD2*/ \
        o(7185, 0x3CB371) /*MEDIUMSEAGREEN*/ \
        o(7186, 0xFFFAF0) /*FLORALWHITE*/ \
        o(7199, 0xFFEFDB) /*ANTIQUEWHITE1*/ \
        o(7203, 0xBC8F8F) /*ROSYBROWN*/ \
        o(7220, 0xD15FEE) /*MEDIUMORCHID2*/ \
        o(7251, 0xDEDEDE) /*GREY87*/ \
        o(7259, 0x0000CD) /*MEDIUM BLUE*/ \
        o(7261, 0xFF4500) /*ORANGE RED*/ \
        o(7270, 0x00FF7F) /*SPRINGGREEN1*/ \
        o(7280, 0x6B6B6B) /*GRAY42*/ \
        o(7290, 0xFAF0E6) /*LINEN*/ \
        o(7306, 0x8FBC8F) /*DARKSEAGREEN*/ \
        o(7307, 0x050505) /*GREY2*/ \
        o(7314, 0x454545) /*GRAY27*/ \
        o(7327, 0xFFD700) /*GOLD1*/ \
        o(7342, 0x00CED1) /*DARK TURQUOISE*/ \
        o(7380, 0x483D8B) /*DARK SLATE BLUE*/ \
        o(7394, 0x2E2E2E) /*GRAY18*/ \
        o(7418, 0xE5E5E5) /*GREY90*/ \
        o(7427, 0xEEE0E5) /*LAVENDERBLUSH2*/ \
        o(7428, 0xADFF2F) /*GREEN YELLOW*/ \
        o(7447, 0x6959CD) /*SLATEBLUE3*/ \
        o(7456, 0x00FF7F) /*SPRINGGREEN*/ \
        o(7465, 0xB8860B) /*DARK GOLDENROD*/ \
        o(7466, 0xEDEDED) /*GRAY93*/ \
        o(7497, 0xFFE4E1) /*MISTYROSE1*/ \
        o(7509, 0xF4A460) /*SANDY BROWN*/ \
        o(7510, 0x696969) /*GREY41*/ \
        o(7514, 0x6A5ACD) /*SLATEBLUE*/ \
        o(7547, 0xFF00FF) /*MAGENTA*/ \
        o(7553, 0xABABAB) /*GREY67*/ \
        o(7561, 0x0A0A0A) /*GREY4*/ \
        o(7570, 0x8B8378) /*ANTIQUEWHITE4*/ \
        o(7577, 0x9ACD32) /*OLIVEDRAB3*/ \
        o(7591, 0xE066FF) /*MEDIUMORCHID1*/ \
        o(7625, 0x98FB98) /*PALE GREEN*/ \
        o(7651, 0x737373) /*GRAY45*/ \
        o(7655, 0x8B4789) /*ORCHID4*/ \
        o(7698, 0xEEC900) /*GOLD2*/ \
        o(7725, 0xFFEBCD) /*BLANCHEDALMOND*/ \
        o(7789, 0xF2F2F2) /*GREY95*/ \
        o(7811, 0x8B2252) /*VIOLETRED4*/ \
        o(7837, 0xF0F0F0) /*GRAY94*/ \
        o(7853, 0x2E8B57) /*SEA GREEN*/ \
        o(7876, 0xD6D6D6) /*GREY84*/ \
        o(7881, 0x6B6B6B) /*GREY42*/ \
        o(7887, 0x00C5CD) /*TURQUOISE3*/ \
        o(7894, 0x20B2AA) /*LIGHT SEA GREEN*/ \
        o(7895, 0x00EE76) /*SPRINGGREEN2*/ \
        o(7948, 0xFFFFF0) /*IVORY*/ \
        o(7956, 0x7A8B8B) /*LIGHTCYAN4*/ \
        o(7962, 0x7A378B) /*MEDIUMORCHID4*/ \
        o(7973, 0x4D4D4D) /*GREY30*/ \
        o(7998, 0xCD6839) /*SIENNA3*/ \
        o(8026, 0xCD69C9) /*ORCHID3*/ \
        o(8038, 0xAFEEEE) /*PALE TURQUOISE*/ \
        o(8046, 0x90EE90) /*LIGHTGREEN*/ \
        o(8057, 0x87CEFA) /*LIGHT SKY BLUE*/ \
        o(8077, 0xEE7621) /*CHOCOLATE2*/ \
        o(8121, 0x8B8B00) /*YELLOW4*/ \
        o(8142, 0xADADAD) /*GRAY68*/ \
        o(8163, 0xB23AEE) /*DARKORCHID2*/ \
        o(8182, 0xFF3E96) /*VIOLETRED1*/ \
        o(8208, 0xFCFCFC) /*GRAY99*/ \
        o(8210, 0xD1EEEE) /*LIGHTCYAN2*/ \
        o(8214, 0xCD853F) /*TAN3*/ \
        o(8252, 0x787878) /*GREY47*/ \
        o(8280, 0xCDB79E) /*BISQUE3*/ \
        o(8339, 0xB8B8B8) /*GREY72*/ \
        o(8344, 0x545454) /*GREY33*/ \
        o(8346, 0x8F8F8F) /*GRAY56*/ \
        o(8354, 0xEEEED1) /*LIGHTYELLOW2*/ \
        o(8369, 0x8B4726) /*SIENNA4*/ \
        o(8394, 0x191970) /*MIDNIGHTBLUE*/ \
        o(8406, 0xCD6889) /*PALEVIOLETRED3*/ \
        o(8423, 0x458B74) /*AQUAMARINE4*/ \
        o(8448, 0xFF7F24) /*CHOCOLATE1*/ \
        o(8459, 0x2E8B57) /*SEAGREEN*/ \
        o(8486, 0xEE30A7) /*MAROON2*/ \
        o(8492, 0x0000EE) /*BLUE2*/ \
        o(8495, 0x3A5FCD) /*ROYALBLUE3*/ \
        o(8513, 0xA6A6A6) /*GRAY65*/ \
        o(8531, 0xFF0000) /*RED1*/ \
        o(8534, 0xBF3EFF) /*DARKORCHID1*/ \
        o(8544, 0x141414) /*GREY8*/ \
        o(8546, 0xEEDD82) /*LIGHTGOLDENROD*/ \
        o(8564, 0x2B2B2B) /*GREY17*/ \
        o(8576, 0xD3D3D3) /*LIGHTGRAY*/ \
        o(8601, 0xEED2EE) /*THISTLE2*/ \
        o(8640, 0x8DB6CD) /*LIGHTSKYBLUE3*/ \
        o(8659, 0x9AFF9A) /*PALEGREEN1*/ \
        o(8667, 0x7F7F7F) /*GREY50*/ \
        o(8710, 0xB5B5B5) /*GREY71*/ \
        o(8715, 0x5C5C5C) /*GREY36*/ \
        o(8717, 0x8C8C8C) /*GRAY55*/ \
        o(8730, 0xB8860B) /*DARKGOLDENROD*/ \
        o(8746, 0xCDCD00) /*YELLOW3*/ \
        o(8752, 0xB22222) /*FIREBRICK*/ \
        o(8753, 0x00CDCD) /*CYAN3*/ \
        o(8774, 0x2F4F4F) /*DARK SLATE GRAY*/ \
        o(8777, 0x8B475D) /*PALEVIOLETRED4*/ \
        o(8794, 0x7FFFD4) /*AQUAMARINE1*/ \
        o(8802, 0x363636) /*GREY21*/ \
        o(8804, 0xC9C9C9) /*GRAY79*/ \
        o(8841, 0xD1D1D1) /*GRAY82*/ \
        o(8851, 0xEED5D2) /*MISTYROSE2*/ \
        o(8852, 0x8FBC8F) /*DARK SEA GREEN*/ \
        o(8862, 0x787878) /*GRAY47*/ \
        o(8866, 0x27408B) /*ROYALBLUE4*/ \
        o(8884, 0xA8A8A8) /*GRAY66*/ \
        o(8897, 0x8B3626) /*TOMATO4*/ \
        o(8905, 0x68228B) /*DARKORCHID4*/ \
        o(8935, 0x1A1A1A) /*GREY10*/ \
        o(8972, 0xFFE1FF) /*THISTLE1*/ \
        o(8976, 0x212121) /*GRAY13*/ \
        o(9038, 0x878787) /*GREY53*/ \
        o(9042, 0x54FF9F) /*SEAGREEN1*/ \
        o(9046, 0xC71585) /*MEDIUMVIOLETRED*/ \
        o(9088, 0x7F7F7F) /*GRAY50*/ \
        o(9095, 0xE0E0E0) /*GRAY88*/ \
        o(9103, 0x6CA6CD) /*SKYBLUE3*/ \
        o(9107, 0x36648B) /*STEELBLUE4*/ \
        o(9111, 0xFF34B3) /*MAROON1*/ \
        o(9145, 0x483D8B) /*DARKSLATEBLUE*/ \
        o(9165, 0x76EEC6) /*AQUAMARINE2*/ \
        o(9169, 0xFFA500) /*ORANGE1*/ \
        o(9173, 0x383838) /*GREY22*/ \
        o(9212, 0xCFCFCF) /*GRAY81*/ \
        o(9222, 0xDB7093) /*PALEVIOLETRED*/ \
        o(9234, 0x00008B) /*BLUE4*/ \
        o(9238, 0xFA8072) /*SALMON*/ \
        o(9255, 0xA1A1A1) /*GRAY63*/ \
        o(9259, 0x00FF00) /*GREEN*/ \
        o(9268, 0xFF6347) /*TOMATO1*/ \
        o(9273, 0xCD5C5C) /*INDIANRED*/ \
        o(9276, 0x7CFC00) /*LAWN GREEN*/ \
        o(9281, 0xFFDEAD) /*NAVAJO WHITE*/ \
        o(9296, 0xCD96CD) /*PLUM3*/ \
        o(9309, 0xFFF0F5) /*LAVENDERBLUSH*/ \
        o(9313, 0x4F4F4F) /*GRAY31*/ \
        o(9340, 0xFFEFD5) /*PAPAYA WHIP*/ \
        o(9341, 0x4169E1) /*ROYALBLUE*/ \
        o(9347, 0x1A1A1A) /*GRAY10*/ \
        o(9350, 0x48D1CC) /*MEDIUM TURQUOISE*/ \
        o(9354, 0x00FF00) /*GREEN1*/ \
        o(9403, 0xEEE8AA) /*PALE GOLDENROD*/ \
        o(9458, 0xCFCFCF) /*GREY81*/ \
        o(9467, 0xEEE9E9) /*SNOW2*/ \
        o(9511, 0xFFF8DC) /*CORNSILK1*/ \
        o(9544, 0x454545) /*GREY27*/ \
        o(9563, 0x000000) /*BLACK*/ \
        o(9583, 0xD6D6D6) /*GRAY84*/ \
        o(9585, 0x0000FF) /*BLUE*/ \
        o(9616, 0xFFB6C1) /*LIGHTPINK*/ \
        o(9639, 0xEE5C42) /*TOMATO2*/ \
        o(9670, 0xBDBDBD) /*GRAY74*/ \
        o(9684, 0x525252) /*GRAY32*/ \
        o(9716, 0xCD853F) /*PERU*/ \
        o(9718, 0x262626) /*GRAY15*/ \
        o(9722, 0x528B8B) /*DARKSLATEGRAY4*/ \
        o(9777, 0xBDB76B) /*DARK KHAKI*/ \
        o(9795, 0xADFF2F) /*GREENYELLOW*/ \
        o(9799, 0xFFEC8B) /*LIGHTGOLDENROD1*/ \
        o(9809, 0xFFE4B5) /*MOCCASIN*/ \
        o(9822, 0xBEBEBE) /*GRAY*/ \
        o(9832, 0x141414) /*GRAY8*/ \
        o(9838, 0xFFFAFA) /*SNOW1*/ \
        o(9849, 0x5CACEE) /*STEELBLUE2*/ \
        o(9886, 0xEEA2AD) /*LIGHTPINK2*/ \
        o(9900, 0xEE8262) /*SALMON2*/ \
        o(9971, 0x00BFFF) /*DEEPSKYBLUE*/ \
        o(9977, 0x912CEE) /*PURPLE2*/ \
        o(9982, 0xFFF0F5) /*LAVENDER BLUSH*/ \
        o(9990, 0xB9D3EE) /*SLATEGRAY2*/ \
        o(10002, 0xEE6A50) /*CORAL2*/ \
        o(10022, 0xCD9B9B) /*ROSYBROWN3*/ \
        o(10027, 0x9F79EE) /*MEDIUMPURPLE2*/ \
        o(10041, 0xC4C4C4) /*GRAY77*/ \
        o(10055, 0x5E5E5E) /*GRAY37*/ \
        o(10058, 0xFDF5E6) /*OLD LACE*/ \
        o(10066, 0xEED8AE) /*WHEAT2*/ \
        o(10083, 0xA0522D) /*SIENNA*/ \
        o(10123, 0xCD1076) /*DEEPPINK3*/ \
        o(10133, 0x383838) /*GRAY22*/ \
        o(10141, 0x1E90FF) /*DODGER BLUE*/ \
        o(10145, 0xF5FFFA) /*MINT CREAM*/ \
        o(10170, 0xEEDC82) /*LIGHTGOLDENROD2*/ \
        o(10189, 0xFFFAFA) /*SNOW*/ \
        o(10203, 0x0D0D0D) /*GRAY5*/ \
        o(10209, 0x8B8989) /*SNOW4*/ \
        o(10235, 0xCDB38B) /*NAVAJOWHITE3*/ \
        o(10243, 0x9AC0CD) /*LIGHTBLUE3*/ \
        o(10247, 0x8B2500) /*ORANGERED4*/ \
        o(10248, 0x607B8B) /*LIGHTSKYBLUE4*/ \
        o(10257, 0xFFAEB9) /*LIGHTPINK1*/ \
        o(10308, 0x0000CD) /*MEDIUMBLUE*/ \
        o(10339, 0xC71585) /*MEDIUM VIOLET RED*/ \
        o(10373, 0xFDF5E6) /*OLDLACE*/ \
        o(10376, 0xFFFAF0) /*FLORAL WHITE*/ \
        o(10396, 0x698B22) /*OLIVEDRAB4*/ \
        o(10398, 0xAB82FF) /*MEDIUMPURPLE1*/ \
        o(10408, 0xE0EEE0) /*HONEYDEW2*/ \
        o(10412, 0xB8B8B8) /*GRAY72*/ \
        o(10464, 0x8DEEEE) /*DARKSLATEGRAY2*/ \
        o(10470, 0x666666) /*GRAY40*/ \
        o(10497, 0xCD8162) /*LIGHTSALMON3*/ \
        o(10504, 0x363636) /*GRAY21*/ \
        o(10513, 0x87CEFA) /*LIGHTSKYBLUE*/ \
        o(10526, 0xFFFFE0) /*LIGHT YELLOW*/ \
        o(10532, 0xF08080) /*LIGHT CORAL*/ \
        o(10561, 0xFAFAD2) /*LIGHTGOLDENRODYELLOW*/ \
        o(10574, 0x050505) /*GRAY2*/ \
        o(10575, 0xFFDAB9) /*PEACHPUFF*/ \
        o(10606, 0x8B795E) /*NAVAJOWHITE4*/ \
        o(10608, 0xEBEBEB) /*GREY92*/ \
        o(10618, 0xFF4500) /*ORANGERED1*/ \
        o(10632, 0xF0F8FF) /*ALICEBLUE*/ \
        o(10637, 0x836FFF) /*SLATEBLUE1*/ \
        o(10662, 0x838B83) /*HONEYDEW4*/ \
        o(10663, 0xC9C9C9) /*GREY79*/ \
        o(10687, 0xFFB90F) /*DARKGOLDENROD1*/ \
        o(10699, 0xF5F5DC) /*BEIGE*/ \
        o(10724, 0xA2B5CD) /*LIGHTSTEELBLUE3*/ \
        o(10743, 0xA6A6A6) /*GREY65*/ \
        o(10745, 0xFFC125) /*GOLDENROD1*/ \
        o(10767, 0xC0FF3E) /*OLIVEDRAB1*/ \
        o(10769, 0x5D478B) /*MEDIUMPURPLE4*/ \
        o(10807, 0xFF6A6A) /*INDIANRED1*/ \
        o(10838, 0xFFF68F) /*KHAKI1*/ \
        o(10841, 0x6E6E6E) /*GRAY43*/ \
        o(10843, 0x708090) /*SLATEGRAY*/ \
        o(10846, 0x6B8E23) /*OLIVEDRAB*/ \
        o(10852, 0xCD6090) /*HOTPINK3*/ \
        o(10862, 0xFAFAFA) /*GREY98*/ \
        o(10868, 0x8B5742) /*LIGHTSALMON4*/ \
        o(10875, 0x3D3D3D) /*GRAY24*/ \
        o(10887, 0x00008B) /*DARK BLUE*/ \
        o(10909, 0x8470FF) /*LIGHTSLATEBLUE*/ \
        o(10949, 0xBBFFFF) /*PALETURQUOISE1*/ \
        o(10954, 0x7D7D7D) /*GREY49*/ \
        o(10979, 0xF7F7F7) /*GREY97*/ \
        o(11018, 0x8A2BE2) /*BLUEVIOLET*/ \
        o(11027, 0xEBEBEB) /*GRAY92*/ \
        o(11033, 0xF0FFF0) /*HONEYDEW1*/ \
        o(11041, 0x949494) /*GRAY58*/ \
        o(11060, 0xFF0000) /*RED*/ \
        o(11071, 0x666666) /*GREY40*/ \
        o(11085, 0x008B45) /*SPRINGGREEN4*/ \
        o(11087, 0xA9A9A9) /*DARKGREY*/ \
        o(11098, 0xF08080) /*LIGHTCORAL*/ \
        o(11107, 0xEEDD82) /*LIGHT GOLDENROD*/ \
        o(11114, 0x999999) /*GREY60*/ \
        o(11122, 0x121212) /*GREY7*/ \
        o(11134, 0xFFD700) /*GOLD*/ \
        o(11138, 0xB3EE3A) /*OLIVEDRAB2*/ \
        o(11172, 0x9BCD9B) /*DARKSEAGREEN3*/ \
        o(11178, 0xEE6363) /*INDIANRED2*/ \
        o(11189, 0x696969) /*DIM GRAY*/ \
        o(11192, 0xCD919E) /*PINK3*/ \
        o(11245, 0x008B8B) /*DARK CYAN*/ \
        o(11267, 0x8B4513) /*CHOCOLATE4*/ \
        o(11307, 0x8B008B) /*DARK MAGENTA*/

#define o(a,b) a,
#define q(a,b) b,
static const unsigned short colorkeys[] { docolors(o) };
static const unsigned int colorvalues[] { docolors(q) };
#undef q
#undef o

/*
Generated with:

grep -v '!' /usr/share/X11/rgb.txt \
| perl -pe "s/(\d+)\s+(\d+)\s+(\d+)\s+(.*)/ {\"\\4\", Repack({\\1,\\2,\\3}) },/" \
| tr A-Z a-z

*/

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
    for(char& c: lc) c = toupper(c);

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
    if(lc.size() >= 9 && lc.compare(0,4,"RGB:") == 0) // shortest: "rgb:F/F/F"
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
    if(lc.size() >= 10 && lc.compare(0,5,"RGBI:") == 0) // shortest: "rgbi:1/1/1"
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

    std::uint_least64_t h = 0;
    for(unsigned char c: lc)
    {
        h = (h << 17) | (h >> (64-17));
        h ^= c;
        h *= 0xc6a4a7935bd1e995LLU;
    }
    h *= 5676u;
    h = (h << 29) | (h >> (64-29));
    h %= 11308u;
    auto i = std::lower_bound(std::begin(colorkeys), std::end(colorkeys), (unsigned short)h);
    if(i != std::end(colorkeys) && *i == h)
        return colorvalues[ i-std::begin(colorkeys) ];

    return 0;
}
