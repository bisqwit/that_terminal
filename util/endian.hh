#ifndef bqtEndianHH
#define bqtEndianHH

#include <cstdint>
#include <type_traits>
#include <tuple>

#if defined(__cpp_lib_endian) && __cpp_lib_endian >= 201907L
# include <bit>
static constexpr bool LITTLE_ENDIAN_AND_UNALIGNED_ACCESS_OK = (std::endian::native == std::endian::little);
static constexpr bool BIG_ENDIAN_AND_UNALIGNED_ACCESS_OK = (std::endian::native == std::endian::big);
#else

#if defined(__x86_64) || defined(__i386)
static constexpr bool LITTLE_ENDIAN_AND_UNALIGNED_ACCESS_OK = true;
static constexpr bool BIG_ENDIAN_AND_UNALIGNED_ACCESS_OK = false;
#else
static constexpr bool LITTLE_ENDIAN_AND_UNALIGNED_ACCESS_OK = false;
static constexpr bool BIG_ENDIAN_AND_UNALIGNED_ACCESS_OK = true;
#endif

#endif

extern inline std::uint_least16_t BSwap16(std::uint_least16_t value)
{
#ifdef __GNUC
    return __builtin_bswap16(value);
#else
    return (value >> 8) | (value << 8);
#endif
}
extern inline std::uint_least32_t BSwap32(std::uint_least32_t value)
{
#ifdef __GNUC
    return __builtin_bswap32(value);
#else
    return (BSwap16(value >> 16)) | (BSwap16(value) << 16);
#endif
}
extern inline std::uint_least64_t BSwap64(std::uint_least64_t value)
{
#ifdef __GNUC
    return __builtin_bswap64(value);
#else
    return (BSwap32(value >> 32)) | ((uint_fast64_t)BSwap32(value) << 32);
#endif
}

template<std::size_t... Indexes>
extern inline std::uint_fast64_t Read(const void* p)
{
    const unsigned char* data = (const unsigned char*)p;

    std::tuple ord{ Indexes... };
    return [&]<std::size_t... I>(std::index_sequence<I...>)
    {
        return ((std::uint_fast64_t(data[ std::get<I>(ord) ]) << (8*I))
              | ...);
    }( std::make_index_sequence<sizeof...(Indexes)>{} );
}
template<std::size_t... Indexes>
extern inline void Write(void* p, std::uint_fast64_t value)
{
    unsigned char* data = (unsigned char*)p;

    std::tuple ord{ Indexes... };
    return [&]<std::size_t... I>(std::index_sequence<I...>)
    {
        ( (data[ std::get<I>(ord) ] = (value >> (8*I))), ... );
    }( std::make_index_sequence<sizeof...(Indexes)>{} );
}

extern inline std::uint_fast16_t R8(const void* p)
{
    const unsigned char* data = (const unsigned char*)p;
    return data[0];
}
extern inline std::uint_fast16_t R16(const void* p)
{
    if constexpr(LITTLE_ENDIAN_AND_UNALIGNED_ACCESS_OK)
        return *(const std::uint_least16_t*)p;
    else if constexpr(BIG_ENDIAN_AND_UNALIGNED_ACCESS_OK)
        return BSwap16(*(const std::uint_least16_t*)p);
    else
        return Read<0,1>(p);
}
extern inline std::uint_fast16_t R16r(const void* p)
{
    if constexpr(BIG_ENDIAN_AND_UNALIGNED_ACCESS_OK)
        return *(const std::uint_least16_t*)p;
    else if constexpr(LITTLE_ENDIAN_AND_UNALIGNED_ACCESS_OK)
        return BSwap16(*(const std::uint_least16_t*)p);
    else
        return Read<1,0>(p);
}
extern inline std::uint_fast32_t R24(const void* p)
{
    /* Note: This might be faster if implemented through R32 and a bitwise and,
     * but we cannot do that because we don't know if the third byte is a valid
     * memory location.
     */
    return Read<0,1,2>(p);
}
extern inline std::uint_fast32_t R24r(const void* p)
{
    return Read<2,1,0>(p);
}
extern inline std::uint_fast32_t R32(const void* p)
{
    if constexpr(LITTLE_ENDIAN_AND_UNALIGNED_ACCESS_OK)
        return *(const std::uint_least32_t*)p;
    else if constexpr(BIG_ENDIAN_AND_UNALIGNED_ACCESS_OK)
        return BSwap32(*(const std::uint_least32_t*)p);
    else
        return Read<0,1,2,3>(p);
}
extern inline std::uint_fast32_t R32r(const void* p)
{
    if constexpr(BIG_ENDIAN_AND_UNALIGNED_ACCESS_OK)
        return *(const std::uint_least32_t*)p;
    else if constexpr(LITTLE_ENDIAN_AND_UNALIGNED_ACCESS_OK)
        return BSwap32(*(const std::uint_least32_t*)p);
    else
        return Read<3,2,1,0>(p);
}

#define L (uint_fast64_t)

extern inline std::uint_fast64_t R64(const void* p)
{
    if constexpr(LITTLE_ENDIAN_AND_UNALIGNED_ACCESS_OK)
        return *(const std::uint_least64_t*)p;
    else if constexpr(BIG_ENDIAN_AND_UNALIGNED_ACCESS_OK)
        return BSwap64(*(const std::uint_least64_t*)p);
    else
        return Read<0,1,2,3,4,5,6,7>(p);
}
extern inline std::uint_fast64_t R64r(const void* p)
{
    if constexpr(BIG_ENDIAN_AND_UNALIGNED_ACCESS_OK)
        return *(const std::uint_least64_t*)p;
    else if constexpr(LITTLE_ENDIAN_AND_UNALIGNED_ACCESS_OK)
        return BSwap64(*(const std::uint_least64_t*)p);
    else
        return Read<7,6,5,4,3,2,1,0>(p);
}

#undef L

extern inline std::uint_fast64_t Rn(const void* p, unsigned bytes)
{
    const unsigned char* data = (const unsigned char*)p;
    switch(bytes)
    {
        #ifdef __GNUC__
        default: __builtin_unreachable();
        #else
        case 0: return 0;
        default: [[fallthrough]];
        #endif
        case 8: return R64(p);
        case 4: return R32(p);
        case 2: return R16(p);
        case 1: return R8(p);
        case 7: return R32(p) | (std::uint_fast64_t(R24(data+4)) << 32);
        case 6: return R32(p) | (std::uint_fast64_t(R16(data+4)) << 32);
        case 5: return R32(p) | (std::uint_fast64_t(R8(data+4)) << 32);
        case 3: return R24(p);
    }
}

extern inline void W8(void* p, std::uint_fast8_t value)
{
    unsigned char* data = (unsigned char*)p;
    data[0] = value;
}
extern inline void W16(void* p, std::uint_fast16_t value)
{
    if constexpr(LITTLE_ENDIAN_AND_UNALIGNED_ACCESS_OK)
        *(uint_least16_t*)p = value;
    else if constexpr(BIG_ENDIAN_AND_UNALIGNED_ACCESS_OK)
        *(uint_least16_t*)p = BSwap16(value);
    else
        Write<0,1>(p, value);
}
extern inline void W24(void* p, std::uint_fast32_t value)
{
    unsigned char* data = (unsigned char*)p;
    W16(data+0, value);
    W8(data+2,  value >> 16u);
}
extern inline void W32(void* p, std::uint_fast32_t value)
{
    if constexpr(LITTLE_ENDIAN_AND_UNALIGNED_ACCESS_OK)
        *(uint_least32_t*)p = value;
    else if constexpr(BIG_ENDIAN_AND_UNALIGNED_ACCESS_OK)
        *(uint_least32_t*)p = BSwap32(value);
    else
        Write<0,1,2,3>(p, value);
}
extern inline void W64(void* p, std::uint_fast64_t value)
{
    if constexpr(LITTLE_ENDIAN_AND_UNALIGNED_ACCESS_OK)
        *(uint_least64_t*)p = value;
    else if constexpr(BIG_ENDIAN_AND_UNALIGNED_ACCESS_OK)
        *(uint_least64_t*)p = BSwap64(value);
    else
        Write<0,1,2,3,4,5,6,7>(p, value);
}

extern inline void Wn(void* p, std::uint_fast64_t value, unsigned bytes)
{
    unsigned char* data = (unsigned char*)p;
    switch(bytes)
    {
        case 8: W64(p, value); break;
        case 7: W8(data+6, value>>48); [[fallthrough]];
        case 6: W8(data+5, value>>40); [[fallthrough]];
        case 5: W8(data+4, value>>32); [[fallthrough]];
        case 4: W32(p, value); break;
        case 3: W24(p, value); break;
        case 2: W16(p, value); break;
        case 1: W8(p, value); break;
    #ifdef __GNUC__
        default: __builtin_unreachable();
    #endif
    }
}

#endif
