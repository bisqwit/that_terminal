#ifndef bqt256ColorHH
#define bqt256ColorHH
/**@file tty/256color.hh
 * @brief Defines xterm256table.
 */
#include <array>

/** A mapping of xterm-256color indexes into RGB colors. Initialized at compile-time. */
extern const constinit std::array<unsigned,256> xterm256table;

#endif
