#include <string>

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

bool isupper(char32_t c);
bool islower(char32_t c);
bool isalpha(char32_t c);
bool isalnum(char32_t c);
bool isalnum_(char32_t c);
bool isdigit(char32_t c);
bool isxdigit(char32_t c);
bool ispunct(char32_t c);
bool isspace(char32_t c);
bool isspace_punct(char32_t c);
bool isblank(char32_t c);
bool isctrl(char32_t c);
bool isprint(char32_t c);
bool isgraph(char32_t c);
bool isnotword(char32_t c);
bool isdouble(char32_t c);
char32_t tolower(char32_t c);
char32_t toupper(char32_t c);
char32_t totitle(char32_t c);

std::u32string FromUTF8(std::string_view s);
std::u32string FromCP437(std::string_view s);
std::string    ToUTF8(std::u32string_view s);
std::string    ToUTF8(std::u16string_view s);
std::size_t    CountIndent(std::u32string_view s, std::size_t begin=0);
