#include <string>
#include <variant>
/* Special sequences:
 *
 *  7FFD fontsize termsize
 *      where fontsize = newfx + 32*newfy
 *            termsize = newsx + 1024*newsy
 *  7FFE delay
 *      where delay  unit is 250ms
 *  7FFF speed
 */
using AutoInputResponse =
    std::variant<
        std::string,            /* input */
        unsigned,               /* milliseconds wait */
        std::array<unsigned,4>  /* terminal resize */
    >;                          /* nothing */

AutoInputResponse GetAutoInput();
void AutoInputStart();

