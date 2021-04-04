#include <array>
#include <string>
#include <algorithm>

inline constexpr std::array<unsigned,3> Unpack(unsigned rgb)
{
    return { rgb>>16, (rgb>>8)&0xFF, rgb&0xFF };
}
inline constexpr unsigned Repack(std::array<unsigned,3> rgb)
{
    if(rgb[0] > 255 || rgb[1] > 255 || rgb[2] > 255) [[unlikely]]
    {
        // Clamp with desaturation:
        float l = (rgb[0]*299u + rgb[1]*587u + rgb[2]*114u)*1e-3f, s = 1.f;
        if(rgb[0] > 255) s = std::min(s, (l-255.f) / (l-rgb[0]));
        if(rgb[1] > 255) s = std::min(s, (l-255.f) / (l-rgb[1]));
        if(rgb[2] > 255) s = std::min(s, (l-255.f) / (l-rgb[2]));
        rgb[0] = (rgb[0] - l) * s + l + 0.5f;
        rgb[1] = (rgb[1] - l) * s + l + 0.5f;
        rgb[2] = (rgb[2] - l) * s + l + 0.5f;
        return (std::min(rgb[0],255u)<<16)
             + (std::min(rgb[1],255u)<<8)
             + (std::min(rgb[2],255u)<<0);
    }
    return (rgb[0] << 16)
         + (rgb[1] << 8)
         + (rgb[2] << 0);
}

inline constexpr unsigned Mix(unsigned color1,unsigned color2, unsigned fac1,unsigned fac2,unsigned sum)
{
    auto a = Unpack(color1), b = Unpack(color2);
    for(unsigned n=0; n<3; ++n) a[n] = (a[n]*fac1 + b[n]*fac2)/(sum);
    return Repack(a);
}

unsigned ParseColorName(std::string_view s);
unsigned ParseColorName(std::u32string_view s);

inline unsigned cmy2rgb(unsigned cmy)
{
    return ~cmy & 0xFFFFFF;
}
inline unsigned cmyk2rgb(unsigned cmyk)
{
    return Mix(0u, ~cmyk >> 8, cmyk & 255, ~cmyk & 255, 255);
}
