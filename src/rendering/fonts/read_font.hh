#ifndef bqtReadFontHH
#define bqtReadFontHH

#include <filesystem>
#include <vector>
#include <map>
#include <unordered_map>
/** @file rendering/fonts/read_font.hh
 * @brief Contains low-level functions for parsing font files. Used by ReadFontsList and FontPlan.
 */

/** GlyphList is the result of ReadFont()_* functions.
 * It contains information about the font in a format that the terminal emulator can use.
 */
struct GlyphList
{
    unsigned height;              ///< Height of all glyphs.
    std::vector<unsigned> widths; ///< Widths of each glyph.

    /** For each supported unicode codepoint, indicates the starting offset to bitmap where this glyph is found. */
    std::unordered_map<char32_t, std::size_t/*start offset*/> glyphs;

    std::vector<unsigned char> bitmap; ///< Bitmap data.

    bool unicode; ///< False if the font encoding is not known; otherwise codepoints are unicode.
};

/** Reads a character encoding table from a PSF.GZ file. */
std::multimap<std::size_t, char32_t> Read_PSFgzEncoding(std::string_view filename);

/** Attempts to convert a character index into unicode using the given encoding.
 *  @param index Character index to convert
 *  @param reg   First part of encoding name (registry)
 *  @param enc   Second part of encoding name (encoding)
 *
 *  If both reg and enc are supplied, they are joined with a dash.
 *  The following encodings are supported by built-in code:
 *     iso10646-1 and iso-8859-1:  Index is returned verbatim
 *     jisx0201.1976-0:            Conversion is done using built-in code
 *
 *  For any other encoding, a conversion table file is searched in share/encodings/.
 *  If the file is found, and the table contains information about this index,
 *  that table is used for the translation.
 *
 *  Additionally, indexes 00-1F and 7F are converted as in CP437;
 *  indexes 81-9F follow the CP/M plus character set with a shift.
 *  The result is an array that contains multiple choices for the encoding.
 */
std::vector<char32_t> BDFtranslateToUnicode(int index, std::string_view reg, std::string_view enc);

/** Reads a PDF.gz format font file. */
GlyphList Read_PSFgz(std::string_view filename, unsigned width, unsigned height, std::string_view guess_encoding);

/** Reads a BDF format font file. */
GlyphList Read_BDF(std::string_view filename, unsigned width, unsigned height, std::string_view guess_encoding);

/** Reads an .inc format font file. */
GlyphList Read_Inc(std::string_view filename, unsigned width, unsigned height, std::string_view guess_encoding);

/** Reads an .asm format font file. */
GlyphList Read_ASM(std::string_view filename, unsigned width, unsigned height, std::string_view guess_encoding);

/** Locates a font file by the given name (@see FindShareFile),
 * and attempts to detect its format and parses it using one of the Read functions listed above.
 * @param filename File to read
 * @param width    Width of glyphs in the font
 * @param height   Height of glyphs in the font
 * @param find     If true, uses FindShareFile to locate the file. If false, just reads the given path.
 * @param guess_encoding Guessed encoding for the font. @see BDFtranslateToUnicode
 * @returns List of glyphs found in the font.
 */
GlyphList Read_Font(
    std::filesystem::path filename,
    unsigned width, unsigned height,
    bool find = false,
    std::string_view guess_encoding = "");

/** Reads a GZIP-compressed or deflated file and
 * decompresses it using TinyDeflate.
 * @returns Decompressed contents.
 */
std::string ReadGZ(std::string_view filename);

#endif
