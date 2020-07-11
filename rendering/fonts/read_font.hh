#include <filesystem>
#include <vector>
#include <map>

struct GlyphList
{
    unsigned height;
    std::vector<unsigned> widths;

    std::map<char32_t, std::size_t/*start offset*/> glyphs;
    std::vector<unsigned char> bitmap;
};

std::string ReadGZ(std::string_view filename);

std::multimap<std::size_t, char32_t> Read_PSFgzEncoding(std::string_view filename);
char32_t BDFtranslateToUnicode(int index, std::string_view reg, std::string_view enc);

GlyphList Read_PSFgz(std::string_view filename, unsigned width, unsigned height);
GlyphList Read_BDF(std::string_view filename, unsigned width, unsigned height);
GlyphList Read_Inc(std::string_view filename, unsigned width, unsigned height);
GlyphList Read_ASM(std::string_view filename, unsigned width, unsigned height);

GlyphList Read_Font(std::filesystem::path filename, unsigned width, unsigned height, bool find = false);

