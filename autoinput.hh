#ifndef bqtAutoInputHH
#define bqtAutoInputHH
/**@file autoinput.hh
 * @brief This module provides the same functionality as INPUTTER (https://bisqwit.iki.fi/source/inputter.html)
 */

#include <string>
#include <variant>
#include <string_view>
/* Special sequences:
 *
 *  7FFD fontsize termsize
 *      where fontsize = newfx + 32*newfy
 *            termsize = newsx + 1024*newsy
 *  7FFE delay
 *      where delay  unit is 250ms
 *  7FFF speed
 */
/** A response element on autoinput. See GetAutoInput() */
using AutoInputResponse =
    std::variant<
        std::string,            /* input */
        std::array<unsigned,4>  /* terminal resize: font size{x,y}, window size{x,y} */
    >;                          /* nothing */

/** Starts automatic input from inputter.dat */
void AutoInputStart(std::string_view filename);

/** Ends automatic input. */
void AutoInputEnd();

/** @returns true if automatic input is active. */
bool AutoInputActive();

/** Returns the next element from the automatic input sequence.
 *
 * @returns a terminal resize specification, or a string of input.
 * An empty string denotes that no further input is available right now.
 */
AutoInputResponse GetAutoInput();

#endif
