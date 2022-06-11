#ifndef bqtNewNewLog2HH
#define bqtNewNewLog2HH
/** @file log2.hh
 * @brief Base-2 logarithm utilities.
 */

#include <cstdint>

/** This header defines
  * c++11 compliant compile-time unsigned integer base2-logarithm
  * with rounding down (floor) and rounding up (ceil).
  */

/** Returns base-2 logarithm, rounded down.
 * @param n The value to be calculated for. If less than two, the result is zero.
 */
static constexpr std::size_t log2floor(std::size_t n) { return n<2 ? 0 : (1+log2floor((n+1)/2)); }

/** Returns base-2 logarithm, rounded up.
 * @param n The value to be calculated for.
 */
static constexpr std::size_t log2ceil(std::size_t n)  { return log2floor(n) + !(n & (n-1));      }

#endif
