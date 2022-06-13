#ifndef bqtTermUI_HH
#define bqtTermUI_HH

#include <string_view>
#include <cstdint>
#include <utility>
#include <variant>

#include "keysym.hh"

class UI
{
    // Font geometry in pixels
    unsigned VidCellWidth  = 8;
    unsigned VidCellHeight = 16;
    // Window geometry in cells
    unsigned WindowWidth  = 106;
    unsigned WindowHeight = 30;
    /// Headless? Disables window creation (useless without autoinput & video recording)
    bool Headless = false;

    /// Allow windows bigger than desktop? Setting this "true"
    /// also disables reacting to window resizes.
    bool Allow_Windows_Bigger_Than_Desktop = false;
public:
    UI();
    ~UI();

    /** @returns currently configured font cell size */
    std::pair<unsigned,unsigned> GetCellSize() const
    {
        return {VidCellWidth,VidCellHeight};
    }
    /** @returns currently configured window size in cells */
    std::pair<unsigned,unsigned> GetWindowSize() const
    {
        return {WindowWidth,WindowHeight};
    }
    /** @returns true if the terminal is started in headless mode */
    bool IsHeadless() const
    {
        return Headless;
    }

    /** Attempts to resize the terminal window.
     * @param cellx New font cell width
     * @param celly New font cell height
     * @param width New window width in cells
     * @param height New window height in cells
     */
    void ResizeTo(unsigned cellx,unsigned celly, unsigned width,unsigned height);

    /** Changes the window title to @param str new window title. */
    void SetWindowTitle(std::string_view str);
    /** Changes the icon name. Unimplemented. @param str new icon name. */
    void SetIconName(std::string_view str);
    /** Issues a short beep, if supported. */
    void BeepOn();

    /** Updates the graphics within the window to the contents of the supplied buffer.
     * @param pixels RGB pixels. The buffer size must be at least cellx*celly*winx*winy units.
     */
    void PresentGraphics(const std::uint32_t* pixels);

    /** Possible kinds of resize, @see EventType */
    enum resizetype { cellx,celly,winx,winy };

    /** The return value of HandleEvents.
     * The first element is text input (string).
     * The second element is one of the following:
     *   std::pair<int,int>, expressing new window size in pixels if the window has been resized from GUI.<br>
     *   std::pair<int,resizetype>, expressing the user's wish to perform a change in rendering proportions.<br>
     *   bool, true if the terminal should be terminated; false if none of above happened.
     */
    using EventType = std::pair<
        std::string,                    ///< text input
        std::variant<
            std::pair<int,int>,         ///< window resized from GUI to these pixel sizes
            std::pair<int, resizetype>, ///< size changed: -1=dec, 1=inc, 0= nothing
            bool                        ///< quit flag
        >>;

    /** Polls (non-blocking) for UI events. @see EventType for possible events.
     * @param permit_text_input If this parameter is false, text input from keyboard is not accepted.
     */
    EventType HandleEvents(bool permit_text_input);

private:
    // Delete copies
    UI(const UI&) = delete;
    UI(UI&&) = delete;
    UI& operator=(const UI&) = delete;
    UI& operator=(UI&&) = delete;
};

extern UI ui;

#endif
