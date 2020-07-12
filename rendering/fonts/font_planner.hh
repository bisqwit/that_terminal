#include <atomic>
#include <memory>
#include <vector>
#include <mutex>
#include <unordered_set>
#include <condition_variable>
#include "read_font.hh"

class FontPlan
{
public:
    void Create(unsigned width, unsigned height, char32_t firstch, char32_t numch);

    const unsigned char* Load(std::size_t index) const
    {
        return bitmap_pointers[index];
    }

    // Bitmap for pixels on single scanline
    struct Glyph
    {
        unsigned long bitmap;
        bool          bold;
    };
    Glyph LoadGlyph(char32_t ch, unsigned scanline, unsigned render_width) const;

private:
    std::vector<bool>                       bold_list;
    std::vector<const unsigned char*>       bitmap_pointers;
    std::vector<unsigned char>              resized_bitmaps;
    std::vector<std::shared_ptr<GlyphList>> loaded_fonts;
    std::unordered_set<std::string> font_filenames;

    std::tuple<unsigned,unsigned,char32_t,char32_t> prev {};
    std::atomic<bool> ready { true };
    mutable std::condition_variable ready_notification { };
    mutable std::mutex working;
};

void FontPlannerTick();
