#ifndef bqtMakeSimHH
#define bqtMakeSimHH
/** @file rendering/fonts/make_similarities.hh
 * @brief Builds a similarity map between glyphs. Used internally by FontPlan.
 */

#include <vector>
#include <utility>

/** Parses the file similarities.dat within ~/.config/that_terminal.
 *
 * If the file is not found, generates it.
 *
 * If the file fonts/alias.txt is found (@see FindShareFile),
 * it improves the result using that data.
 *
 * For generating, it reads the following file (@see FindShareFile):
 * unicode/UnicodeData.txt
 *
 * @returns a vector containing character substitution pairs.
 * Each pair contains a target glyph (unicode character index),
 * and a candidate recipe (approximately similar glyph) for substituting
 * if the target glyph is not available in the font.
 */
std::vector<std::pair<char32_t/*goal*/, char32_t/*recipe*/>> ParseSimilarities();

#endif
