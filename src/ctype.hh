#ifndef bqtCTypeHH
#define bqtCTypeHH
/** @file ctype.hh
 * @brief Functions for identifying and converting unicode characters and representation forms.
 */

#include <string>

/* Make sure that these functions are not #defined as macros in a library */
#undef isupper
#undef islower
#undef isalpha
#undef isalnum
#undef isalnum_
#undef isdigit
#undef isxdigit
#undef ispunct
#undef isspace
#undef isspace_punct
#undef isblank
#undef isctrl
#undef isprint
#undef isgraph
#undef isnotword
#undef isdouble
#undef tolower
#undef toupper
#undef totitle

/** @returns true if given code index is uppercase */
bool isupper(char32_t c);
/** @returns true if given code index is lowercase */
bool islower(char32_t c);
/** @returns true if given code index represents an alphabet */
bool isalpha(char32_t c);
/** @returns true if given code index represents an alphabet or a numeric digit */
bool isalnum(char32_t c);
/** @returns true if given code index represents an alphabet or a numeric digit or the underscore */
bool isalnum_(char32_t c);
/** @returns true if given code index represents a numeric digit */
bool isdigit(char32_t c);
/** @returns true if given code index represents a hexadecimal digit */
bool isxdigit(char32_t c);
/** @returns true if given code index represents punctuation */
bool ispunct(char32_t c);
/** @returns true if given code index represents whitespace */
bool isspace(char32_t c);
/** @returns true if given code index represents whitespace or punctuation */
bool isspace_punct(char32_t c);
/** @returns true if given code index represents whitespace */
bool isblank(char32_t c);
/** @returns true if given code index represents a control character */
bool isctrl(char32_t c);
/** @returns true if given code index is printable */
bool isprint(char32_t c);
/** @returns true if given code index is graphics symbol */
bool isgraph(char32_t c);
/** @returns true if given code index is graphics symbol */
bool isnotword(char32_t c);
/** @returns true if given code index is double-wide */
bool isdouble(char32_t c);

/** Converts the given code index to its corresponding lowercase version */
char32_t tolower(char32_t c);
/** Converts the given code index to its corresponding uppercase version */
char32_t toupper(char32_t c);
/** Converts the given code index to its corresponding titlecase version */
char32_t totitle(char32_t c);

/** Converts given text, assumed to be UTF-8 encoded, into a sequence of unicode codepoints */
std::u32string FromUTF8(std::string_view s);

/** Converts given text, assumed to be CP437 encoded, into a sequence of unicode codepoints */
std::u32string FromCP437(std::string_view s);

/** Converts given sequence of unicode code points into UTF-8 encoded string */
std::string    ToUTF8(std::u32string_view s);

/** Converts given sequence of unicode code points into UTF-8 encoded string */
std::string    ToUTF8(std::u16string_view s);

/** Calculates and returns the number of spaces in the beginning of the given string.
 * @param s      The string to be searched.
 * @param begin  An optional beginning index from which to count the spaces.
 */
std::size_t    CountIndent(std::u32string_view s, std::size_t begin=0);

#endif
