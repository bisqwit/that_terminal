#include <utility>

class FontHandler
{
public:
    // Bitmap for pixels on single scanline
    struct Glyph
    {
        unsigned long bitmap;
        bool          bold;
    };
    Glyph LoadGlyph(char32_t ch, unsigned scanline, unsigned render_width);
    void  Load(std::size_t fx, std::size_t fy);

private:
    std::size_t fx, fy;
    struct
    {
        std::size_t actual_fx, actual_fy;
        const unsigned char* font;
        std::pair<unsigned,bool>(*    map)(char32_t);
        std::pair<unsigned,bool>(*realmap)(char32_t);
        unsigned font_row_size_in_bytes;
        unsigned character_size_in_bytes;
        bool bold;
    } choices[2];
};
FontHandler LoadFont(std::size_t fx, std::size_t fy);
