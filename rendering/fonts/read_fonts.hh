#ifndef bqtReadFontsHH
#define bqtReadFontsHH
/** @file rendering/fonts/read_fonts.hh
 * @brief Loads the list of available fonts and the list of characters in each. Used by FontPlan.
 */
#include <map>
#include <string>
#include <vector>

/** Information returned by ReadFontsList(). */
using FontsInfo = std::map<std::pair<std::string/*filename*/, std::pair<unsigned/*x*/,unsigned/*y*/>>,
                           std::pair<std::string/*guessed encoding*/, std::vector<bool>/*supported characters*/>>;

/** Reads the list of fonts in the system.
 * This information is cached in fonts-list.dat (@see FindCacheFile).
 * The function reads all files in share/fonts/files and collects information on them.
 *
 * @returns Font information.
 *          The information maps the following key: {filename, width, height}
 *          into following values: {guessed encoding, bitset of supported characters}.
 */
FontsInfo ReadFontsList();

#endif
