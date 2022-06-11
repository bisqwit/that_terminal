#ifndef bqtFontPlannerHH
#define bqtFontPlannerHH
/** @file rendering/fonts/font_planner.hh
 * @brief Defines an interface that converts unicode characters into pixels,
 * through scanning available font files for a specified range in unicode at specified size.
 */
#include <atomic>
#include <memory>
#include <vector>
#include <mutex>
#include <unordered_set>
#include <condition_variable>
#include "read_font.hh"

/** FontPlan manages a set of fonts. Multiple independent instances of this class are allowed. */
class FontPlan
{
public:
    /** Initializes a background process (thread) for preparing information to be used by LoadGlyph.
     *
     * @param width   Width of font in pixels
     * @param height  Height of font in pixels
     * @param firstch First unicode codepoint to initialize
     * @param numch   Number of unicode codepoints to initialize
     *
     * Attempts to find the best representation for each glyph in this range,
     * using information in share/fonts/preferences.txt .
     * If a font by the exact requested size is not found, attempts to find closest match.
     *
     * If the parameters are identical to when the function was last called, does nothing.
     */
    void Create(unsigned width, unsigned height, char32_t firstch, char32_t numch);

    /** Bitmap for pixels on single scanline */
    struct Glyph
    {
        unsigned long bitmap; ///< bitmap: maximum 64 pixels.
        bool          bold;   ///< True if the font is "bold" style
    };

    /** Loads glyph for the given codepoint.
     * Waits until the background process started by Create() has finished, if necessary.
     *
     * @param ch           Codepoint to load information for. Should be >= firstch and <= firstch+numch.
     * @param scanline     Scanline to load. Should be < height.
     * @param render_width Width of font in pixels, when it is rendered.
     *
     * @returns Glyph structure
     */
    Glyph LoadGlyph(char32_t ch, unsigned scanline, unsigned render_width) const;

private:
    std::vector<bool>                       bold_list;       ///< Bold flags for each glyph in the requested range
    std::vector<const unsigned char*>       bitmap_pointers; ///< Bitmap pointers -"-
    std::vector<unsigned char>              resized_bitmaps; ///< Resized bitmaps -"-
    std::vector<std::shared_ptr<GlyphList>> loaded_fonts;    ///< List of fonts currently being used, by pointer
    std::unordered_set<std::string> font_filenames;          ///< List of fonts currently being used, by name

    /** Caches the parameters of the last Create() call. */
    std::tuple<unsigned,unsigned,char32_t,char32_t> prev {};

    /** Thread-safety support. */
    std::atomic<bool> ready { true };
    mutable std::condition_variable ready_notification { };
    mutable std::mutex working;
};

/** A callback function that unloads fonts that have not been accessed for 60 seconds.
 * This check is only done if 10 or more seconds has elapsed since last check.
 */
void FontPlannerTick();

#endif
