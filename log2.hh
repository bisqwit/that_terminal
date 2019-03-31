#ifndef bqtNewNewLog2HH
#define bqtNewNewLog2HH

#include <cstdint>

/* C++11 compile-time unsigned integer base2-logarithm with rounding down (floor) and rounding up (ceil) */

template<std::size_t N> struct log2_floor_st{ static constexpr std::size_t result = 1+log2_floor_st<(N+1)/2>::result; };
template<> struct log2_floor_st<0> { static constexpr std::size_t result = 0; };
template<> struct log2_floor_st<1> { static constexpr std::size_t result = 0; };
// 0:0  1:0  2:1   3:1  4:2  5:2  6:2  7:2  8:3   256:8

template<std::size_t N> struct log2_ceil_st
{
    static constexpr std::size_t result = log2_floor_st<N>::result + (N != (1ul << log2_floor_st<N>::result));
};

static constexpr std::size_t log2floor(std::size_t n) { return n<2 ? 0 : (1+log2floor((n+1)/2)); }
static constexpr std::size_t log2ceil(std::size_t n)  { return log2floor(n) + !(n & (n-1));      }

#endif
