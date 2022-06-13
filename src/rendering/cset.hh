#ifndef bqtCSetHH
#define bqtCSetHH
/** @file rendering/cset.hh
 * @brief Defines TranslateCSet()
 */

/** TranslateCSet() translates the given character code into unicode codepoint.
 *
 * The following behaviors are defined:
 * When cset = 0:
 *   Returns the value of c verbatim.
 * When cset = 1:
 *   If c is in range 0x5F..0x7E (inclusive),
 *   returns a symbol from DEC Graphics Set corresponding to that index.
 *   Otherwise returns the value of c verbatim.
 *
 * Otherwise behavior is undefined.
 */
char32_t TranslateCSet(char32_t c, int cset);

#endif
