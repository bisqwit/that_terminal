#ifndef bqtKeySymHH
#define bqtKeySymHH
#include <string>

/** @file keysym.hh
 * @brief Defines InterpretInput()
 */

#include <SDL.h>

/** Interprets the button that has been pressed and converts it into a sequence of characters. */
std::string InterpretInput(bool shift, bool alt, bool ctrl, SDL_Keycode sym);

#endif
