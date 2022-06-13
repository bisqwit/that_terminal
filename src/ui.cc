#ifdef RUN_TESTS
# include <gtest/gtest.h>
#endif
/** @file ui.cc
 * @brief User interface.
 */

#include <unordered_map>
#include <cstdlib>
#include <string>
#include <vector>
#include <cstdio>
#include <cmath>

#include "ui.hh"

#include <SDL.h>
#include <SDL_syswm.h>

#ifdef SDL_VIDEO_DRIVER_X11
# include <X11/Xlib.h>
#endif
#ifdef SDL_VIDEO_DRIVER_WINDOWS
# include <Utilapiset.h>
#endif

namespace
{
    std::unordered_map<SDL_Keycode, bool> keys; ///< List of currently depressed keys

    SDL_Window*   window   = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture*  texture  = nullptr;
    unsigned cells_horiz, cell_width_pixels,  pixels_width,  bufpixels_width, texturewidth;
    unsigned cells_vert,  cell_height_pixels, pixels_height, bufpixels_height, textureheight;
    std::vector<std::uint32_t> pixbuf;
    // Scale factor
    float ScaleX = 1.f;
    float ScaleY = 1.f;

    void SDL_ReInitialize(unsigned cells_horizontal, unsigned cells_vertical)
    {
        cells_horiz = cells_horizontal;
        cells_vert  = cells_vertical;
        cell_width_pixels  = ui.GetCellSize().first;
        cell_height_pixels = ui.GetCellSize().second;
        pixels_width  = cells_horizontal * cell_width_pixels,
        pixels_height = cells_vertical   * cell_height_pixels;
        bufpixels_width = cells_horizontal * ui.GetCellSize().first;
        bufpixels_height = cells_vertical  * ui.GetCellSize().second;

        if(!ui.IsHeadless())
        {
            bool had_window = window;
            if(!window)
            {
                window = SDL_CreateWindow("that terminal",
                    //0,0,
                    SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                    pixels_width*ScaleX, pixels_height*ScaleY,
                    SDL_WINDOW_RESIZABLE);
            }
            else
            {
                SDL_SetWindowSize(window, pixels_width*ScaleX, pixels_height*ScaleY);
            }
            if(!renderer)
            {
                renderer = SDL_CreateRenderer(window, -1, 0);
            }

            if(texture && (texturewidth<bufpixels_width || textureheight<bufpixels_height))
            {
                SDL_DestroyTexture(texture);
                texture = nullptr;
            }
            if(!texture)
            {
                texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_BGRA32,
                    SDL_TEXTUREACCESS_STREAMING,
                    texturewidth  = bufpixels_width,
                    textureheight = bufpixels_height);
            }

            int w,h;
            SDL_GetWindowSize(window, &w,&h);
            ScaleX = w/(float)pixels_width;
            ScaleY = h/(float)pixels_height;

            if(!had_window)
                SDL_StartTextInput();
        }

        pixbuf.resize(bufpixels_width*bufpixels_height);

        fprintf(stderr, "Cells: %ux%u, pix sizes: %ux%u (%u), pixels: %ux%u, buf: %ux%u scale:%gx%g\n",
            cells_horiz,cells_vert,
            cell_width_pixels,cell_height_pixels, ui.GetCellSize().second,
            pixels_width,pixels_height,
            bufpixels_width,bufpixels_height,
            ScaleX, ScaleY);
    }


}

void UI::BeepOn()
{
    if(IsHeadless()) return;

    SDL_SysWMinfo info = {};
    SDL_VERSION(&info.version);
    if(!SDL_GetWindowWMInfo(window,&info))
        fprintf(stderr, "GetWindowWMInfo failed %s\n", SDL_GetError());

#ifdef SDL_VIDEO_DRIVER_X11
    if(info.subsystem == SDL_SYSWM_X11)
    {
        if(auto display = info.info.x11.display)
        {
            XKeyboardControl ctrl = {};
            ctrl.bell_percent = 100;
            ctrl.bell_pitch   = 440;
            ctrl.bell_duration = 50;
            XChangeKeyboardControl(display, KBBellPitch|KBBellPercent|KBBellDuration, &ctrl);
            XBell(display, 100);
            return;
        }
    }
#endif
#ifdef SDL_VIDEO_DRIVER_WINDOWS
    if(info.subsystem == SDL_SYSWM_WINDOWS)
    {
        Beep(440, 50);
        return;
    }
#endif
    // Unknown video driver.
    // TODO: Create an audio interface.
    // TODO: Synthesize beep.
}

void UI::SetWindowTitle(std::string_view str)
{
    if(window)
        SDL_SetWindowTitle(window, std::string(str).c_str()); // Ensure nul-terminated
}

void UI::SetIconName(std::string_view)
{
    /* Unimplemented */
}

void UI::ResizeTo(unsigned cellx,unsigned celly, unsigned width,unsigned height)
{
    VidCellWidth  = cellx;
    VidCellHeight = celly;
    WindowWidth  = width;
    WindowHeight = height;
    SDL_ReInitialize(WindowWidth, WindowHeight);
}

void UI::PresentGraphics(const std::uint32_t* pixbuf)
{
    if(ui.IsHeadless() || !window)
        return;

    SDL_Rect rect;
    rect.x=0; rect.w=bufpixels_width;
    rect.y=0; rect.h=0;
    unsigned errors = 0;
    auto RenderFlushLines = [&]()
    {
        if(!rect.h) return;

        int w,h;
        SDL_GetWindowSize(window, &w,&h);

        SDL_Rect trect;
        trect.x = rect.x * w / bufpixels_width;
        trect.y = rect.y * h / bufpixels_height;
        trect.w = (rect.x + rect.w) * w / bufpixels_width - trect.x;
        trect.h = (rect.y + rect.h) * h / bufpixels_height - trect.y;

        if(SDL_UpdateTexture(texture, &rect,
                             &pixbuf[0] + rect.y*bufpixels_width,
                             bufpixels_width*sizeof(pixbuf[0]))) ++errors;
        if(SDL_RenderCopy(renderer, texture, &rect, &trect)) ++errors;

        rect.y += rect.h;
        rect.h = 0;
    };
    auto RenderAddLine = [&](unsigned line)
    {
        if(line > unsigned(rect.y+rect.h+15) || line < unsigned(rect.y))
        {
            RenderFlushLines();
        }
        if(!rect.h) { rect.y = line; rect.h = 1; }
        else rect.h = line+1-rect.y;
    };

    for(unsigned y=0; y<cells_vert*VidCellHeight; ++y)
        RenderAddLine(y);

    RenderFlushLines();

    if(rect.y) { SDL_RenderPresent(renderer); }
}

UI::EventType UI::HandleEvents(bool permit_text_input)
{
    std::string outbuffer, pending_input;
    auto retval = [&](auto&& v) -> UI::EventType
    {
        return std::pair(std::move(outbuffer) + std::move(pending_input),
                         std::move(v));
    };
    if(!IsHeadless())
        for(SDL_Event ev; SDL_PollEvent(&ev); )
        {
            switch(ev.type)
            {
                case SDL_WINDOWEVENT:
                    switch(ev.window.event)
                    {
                        case SDL_WINDOWEVENT_EXPOSED:
                        case SDL_WINDOWEVENT_RESIZED:
                        case SDL_WINDOWEVENT_SIZE_CHANGED:
                        {
                            if(!Allow_Windows_Bigger_Than_Desktop)
                            {
                                int w,h;
                                SDL_GetWindowSize(window, &w,&h);
                                return retval(std::pair<int,int>(w/ScaleX, h/ScaleY));
                            }
                            break;
                        }
                        default:
                            break;
                    }
                    break;
                case SDL_QUIT:
                    return retval(true); // quit
                case SDL_TEXTINPUT:
                    //std::fprintf(stderr, "Text input(%s)\n", ev.text.text);
                    if(permit_text_input)
                    {
                        pending_input.clear(); // Overrides any input events from SDL_KEYDOWN
                        outbuffer += ev.text.text;
                    }
                    break;
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                {
                    keys[ev.key.keysym.sym] = (ev.type == SDL_KEYDOWN);
                    if(permit_text_input && ev.type == SDL_KEYDOWN)
                    {
                        bool shift = keys[SDLK_LSHIFT] || keys[SDLK_RSHIFT];
                        bool alt   = keys[SDLK_LALT]   || keys[SDLK_RALT];
                        bool ctrl  = keys[SDLK_LCTRL]  || keys[SDLK_RCTRL];
                        if(!shift && !alt && !ctrl)
                            switch(ev.key.keysym.sym)
                            {
                                case SDLK_F1: return retval(std::pair(-1, resizetype::winy));
                                case SDLK_F2: return retval(std::pair(+1, resizetype::winy));
                                case SDLK_F3: return retval(std::pair(-1, resizetype::winx));
                                case SDLK_F4: return retval(std::pair(+1, resizetype::winx));
                                case SDLK_F5: return retval(std::pair(-1, resizetype::celly));
                                case SDLK_F6: return retval(std::pair(+1, resizetype::celly));
                                case SDLK_F7: return retval(std::pair(-1, resizetype::cellx));
                                case SDLK_F8: return retval(std::pair(+1, resizetype::cellx));
                                case SDLK_F9: // Decrease Y-scale:
                                    if(ScaleY >= 2) --ScaleY; else ScaleY /= std::sqrt(2);
                                    if(ScaleY < 1.5) ScaleY = 1;
                                    return retval(std::pair(0, resizetype::winy));
                                case SDLK_F10:// Increase Y-scale:
                                    if(ScaleY < 2) ScaleY *= std::sqrt(2); else ++ScaleY;
                                    if(ScaleY >= 1.9) ScaleY = int(ScaleY+0.1);
                                    return retval(std::pair(0, resizetype::winy));
                                case SDLK_F11:// Decrease X-scale:
                                    if(ScaleX >= 2) --ScaleX; else ScaleX /= std::sqrt(2);
                                    if(ScaleX < 1.5) ScaleX = 1;
                                    return retval(std::pair(0, resizetype::winx));
                                case SDLK_F12:// Increase X-scale:
                                    if(ScaleX < 2) ScaleX *= std::sqrt(2); else ++ScaleX;
                                    if(ScaleX >= 1.9) ScaleX = int(ScaleX+0.1);
                                    return retval(std::pair(0, resizetype::winx));
                            }
                        // Put the input in "pending_input", so that it gets automatically
                        // cancelled if a textinput event is generated.
                        pending_input += InterpretInput(shift, alt, ctrl, ev.key.keysym.sym);
                    }
                    break;
                }
            }
        }
    return retval(false);
}

UI::UI()
{
}

UI::~UI()
{
    SDL_Quit();
}

UI ui;

#ifdef RUN_TESTS
TEST(ui, general)
{
    ui.SetHeadless(true);
    ui.BeepOn();
    ui.ResizeTo(8,8, 10,10);
    std::vector<std::uint32_t> pixbuf(10*10*8*8, 0xFF00FF);
    ui.PresentGraphics(&pixbuf[0]);
    ui.HandleEvents(true);

    ui.SetHeadless(false);

    ui.ResizeTo(8,8, 10,10);
    ui.SetWindowTitle("gruu");
    ui.SetIconName("gruu");
    ui.BeepOn();
    ui.PresentGraphics(&pixbuf[0]);
    ui.HandleEvents(true);
    ui.ResizeTo(16,16, 10,10);
    pixbuf.resize(10*10*16*16);
    ui.PresentGraphics(&pixbuf[0]);
}

using namespace std::string_view_literals;

#include <cstring>
TEST(ui, eventsim)
{
    std::string result{};
    SDL_Event ev{}, dummy{};

    auto DoEvent = [&](auto type)
    {
        ev.type = type;
        while(SDL_PollEvent(&dummy)) {} // Delete all real events
        SDL_PushEvent(&ev);             // Insert the fabricated event
        // Now read events
        for(unsigned n=0; n<8; ++n)
            result += ui.HandleEvents(true).first;
    };
    auto DoKeyEvent = [&](auto type, auto sym)
    {
        ev.key.keysym.sym = sym;
        DoEvent(type);
    };
    auto CtrlAltShiftOff = [&]()
    {
        for(auto c: {SDLK_LSHIFT, SDLK_RSHIFT, SDLK_LALT, SDLK_RALT, SDLK_LCTRL, SDLK_RCTRL})
            DoKeyEvent(SDL_KEYUP, c);
    };

    for(auto c: {SDL_WINDOWEVENT_EXPOSED, SDL_WINDOWEVENT_RESIZED, SDL_WINDOWEVENT_SIZE_CHANGED})
    {
        ev.window.event = c;
        SDL_PushEvent(&ev);
        DoEvent(SDL_WINDOWEVENT);
    }
    std::strcpy(ev.text.text, "kupo");
    DoEvent(SDL_TEXTINPUT);

    static constexpr const std::string_view expected_results[25] =
    {
        "kupo"sv,
        // None:
        ""sv,
        "\33OD\33OC\33OA\33OB\33[H\33[F\33[2~\33[3~\33[5~\33[6~"sv,
        "\033.,-\r\x7F\t \r"sv,
        // Shift:
        "\33O1;2P\33O1;2Q\33O1;2R\33O1;2S\33[15;2~\33[17;2~\33[18;2~\33[19;2~\33[20;2~\33[21;2~\33[23;2~\33[24;2~"sv,
        "\33[1;2D\33[1;2C\33[1;2A\33[1;2B\33[1;2H\33[1;2F\33[2;2~\33[3;2~\33[5;2~\33[6;2~"sv,
        "\033.,-\r\x7F\33[Z \r"sv,
        // Ctrl:
        "\33O1;5P\33O1;5Q\33O1;5R\33O1;5S\33[15;5~\33[17;5~\33[18;5~\33[19;5~\33[20;5~\33[21;5~\33[23;5~\33[24;5~"sv,
        "\33[1;5D\33[1;5C\33[1;5A\33[1;5B\33[1;5H\33[1;5F\33[2;5~\33[3;5~\33[5;5~\33[6;5~"sv,
        "\x1\x2\x3\x4\x5\x6\a\b\t\n\v\f\r\xE\xF\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\033" "01\0\033\x1C\x1D\x1E\x1F\x7F" "9\xE\f\r\r\b\t\0\r"sv,
        // Shift+Ctrl:
        "\33O1;6P\33O1;6Q\33O1;6R\33O1;6S\33[15;6~\33[17;6~\33[18;6~\33[19;6~\33[20;6~\33[21;6~\33[23;6~\33[24;6~"sv,
        "\33[1;6D\33[1;6C\33[1;6A\33[1;6B\33[1;6H\33[1;6F\33[2;6~\33[3;6~\33[5;6~\33[6;6~"sv,
        "\x1\x2\x3\x4\x5\x6\a\b\33[Z\n\v\f\r\xE\xF\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\033" "01\0\033\x1C\x1D\x1E\x1F\x7F" "9\xE\f\r\r\b\33[Z\0\r"sv,
        // Alt:
        "\33O1;3P\33O1;3Q\33O1;3R\33O1;3S\33[15;3~\33[17;3~\33[18;3~\33[19;3~\33[20;3~\33[21;3~\33[23;3~\33[24;3~"sv,
        "\33[1;3D\33[1;3C\33[1;3A\33[1;3B\33[1;3H\33[1;3F\33[2;3~\33[3;3~\33[5;3~\33[6;3~"sv,
        "\xC3\xA1\xC3\xA2\xC3\xA3\xC3\xA4\xC3\xA5\xC3\xA6\xC3\xA7\xC3\xA8\xC3\xA9\xC3\xAA\xC3\xAB\xC3\xAC\xC3\xAD\xC3\xAE\xC3\xAF\xC3\xB0\xC3\xB1\xC3\xB2\xC3\xB3\xC3\xB4\xC3\xB5\xC3\xB6\xC3\xB7\xC3\xB8\xC3\xB9\xC3\xBA\xC2\x9B\xC2\xB0\xC2\xB1\xC2\xB2\xC2\xB3\xC2\xB4\xC2\xB5\xC2\xB6\xC2\xB7\xC2\xB8\xC2\xB9\xC2\xAE\xC2\xAC\xC2\xAD\xC2\x8D\xC3\xBF\xC2\x89\xC2\xA0\xC2\x8D"sv,
        // Shift+Alt:
        "\33O1;4P\33O1;4Q\33O1;4R\33O1;4S\33[15;4~\33[17;4~\33[18;4~\33[19;4~\33[20;4~\33[21;4~\33[23;4~\33[24;4~"sv,
        "\33[1;4D\33[1;4C\33[1;4A\33[1;4B\33[1;4H\33[1;4F\33[2;4~\33[3;4~\33[5;4~\33[6;4~"sv,
        "\xC3\x81\xC3\x82\xC3\x83\xC3\x84\xC3\x85\xC3\x86\xC3\x87\xC3\x88\xC3\x89\xC3\x8A\xC3\x8B\xC3\x8C\xC3\x8D\xC3\x8E\xC3\x8F\xC3\x90\xC3\x91\xC3\x92\xC3\x93\xC3\x94\xC3\x95\xC3\x96\xC3\x97\xC3\x98\xC3\x99\xC3\x9A\xC2\x9B\xC2\xB0\xC2\xB1\xC2\xB2\xC2\xB3\xC2\xB4\xC2\xB5\xC2\xB6\xC2\xB7\xC2\xB8\xC2\xB9\xC2\xAE\xC2\xAC\xC2\xAD\xC2\x8D\xC3\xBF\xC2\x89\xC2\xA0\xC2\x8D"sv,
        // Ctrl+Alt:
        "\33O1;7P\33O1;7Q\33O1;7R\33O1;7S\33[15;7~\33[17;7~\33[18;7~\33[19;7~\33[20;7~\33[21;7~\33[23;7~\33[24;7~"sv,
        "\33[1;7D\33[1;7C\33[1;7A\33[1;7B\33[1;7H\33[1;7F\33[2;7~\33[3;7~\33[5;7~\33[6;7~"sv,
        "\xC2\x81\xC2\x82\xC2\x83\xC2\x84\xC2\x85\xC2\x86\xC2\x87\xC2\x88\xC2\x89\xC2\x8A\xC2\x8B\xC2\x8C\xC2\x8D\xC2\x8E\xC2\x8F\xC2\x90\xC2\x91\xC2\x92\xC2\x93\xC2\x94\xC2\x95\xC2\x96\xC2\x97\xC2\x98\xC2\x99\xC2\x9A\xC2\x9B\xC2\xB0\xC2\xB1\xC2\x80\xC2\x9B\xC2\x9C\xC2\x9D\xC2\x9E\xC2\x9F\xC3\xBF\xC2\xB9\xC2\x8E\xC2\x8C\xC2\x8D\xC2\x8D\xC2\x88\xC2\x89\xC2\x80\xC2\x8D"sv,
        // Shift+Ctrl+Alt:
        "\33O1;8P\33O1;8Q\33O1;8R\33O1;8S\33[15;8~\33[17;8~\33[18;8~\33[19;8~\33[20;8~\33[21;8~\33[23;8~\33[24;8~"sv,
        "\33[1;8D\33[1;8C\33[1;8A\33[1;8B\33[1;8H\33[1;8F\33[2;8~\33[3;8~\33[5;8~\33[6;8~"sv,
        "\xC2\x81\xC2\x82\xC2\x83\xC2\x84\xC2\x85\xC2\x86\xC2\x87\xC2\x88\xC2\x89\xC2\x8A\xC2\x8B\xC2\x8C\xC2\x8D\xC2\x8E\xC2\x8F\xC2\x90\xC2\x91\xC2\x92\xC2\x93\xC2\x94\xC2\x95\xC2\x96\xC2\x97\xC2\x98\xC2\x99\xC2\x9A\xC2\x9B\xC2\xB0\xC2\xB1\xC2\x80\xC2\x9B\xC2\x9C\xC2\x9D\xC2\x9E\xC2\x9F\xC3\xBF\xC2\xB9\xC2\x8E\xC2\x8C\xC2\x8D\xC2\x8D\xC2\x88\xC2\x89\xC2\x80\xC2\x8D"sv
    };

    const std::string_view* result_ptr = expected_results;
    EXPECT_EQ(result, *result_ptr);
    result.clear(); ++result_ptr;

    for(unsigned ctrlmask=0; ctrlmask<8; ++ctrlmask)
    {
        CtrlAltShiftOff();
        if(ctrlmask&1) DoKeyEvent(SDL_KEYDOWN, SDLK_LSHIFT);
        if(ctrlmask&2) DoKeyEvent(SDL_KEYDOWN, SDLK_LCTRL);
        if(ctrlmask&4) DoKeyEvent(SDL_KEYDOWN, SDLK_LALT);
        for(auto c: {SDLK_F1, SDLK_F2, SDLK_F3, SDLK_F4, SDLK_F5, SDLK_F6, SDLK_F7, SDLK_F8,
                     SDLK_F9, SDLK_F10, SDLK_F11, SDLK_F12})
        {
            DoKeyEvent(SDL_KEYDOWN, c);
        }

        EXPECT_EQ(result, result_ptr[0]);
        result.clear();

        for(auto c: {SDLK_LEFT, SDLK_RIGHT, SDLK_UP, SDLK_DOWN, SDLK_HOME, SDLK_END,
                     SDLK_INSERT, SDLK_DELETE, SDLK_PAGEUP, SDLK_PAGEDOWN})
        {
            DoKeyEvent(SDL_KEYDOWN, c);
        }

        EXPECT_EQ(result, result_ptr[1]);
        result.clear();

        for(auto c: {SDLK_a,SDLK_b,SDLK_c,SDLK_d,SDLK_e,SDLK_f,SDLK_g,SDLK_h,SDLK_i,
                     SDLK_j,SDLK_k,SDLK_l,SDLK_m,SDLK_n,SDLK_o,SDLK_p,SDLK_q,SDLK_r,
                     SDLK_s,SDLK_t,SDLK_u,SDLK_v,SDLK_w,SDLK_x,SDLK_y,SDLK_z,
                     SDLK_ESCAPE,
                     SDLK_0,SDLK_1,SDLK_2,SDLK_3,SDLK_4,
                     SDLK_5,SDLK_6,SDLK_7,SDLK_8,SDLK_9,
                     SDLK_PERIOD,SDLK_COMMA,SDLK_SLASH,SDLK_RETURN,SDLK_BACKSPACE,
                     SDLK_TAB,SDLK_SPACE,SDLK_KP_ENTER})
        {
            DoKeyEvent(SDL_KEYDOWN, c);
        }

        EXPECT_EQ(result, result_ptr[2]);
        result.clear();

        result_ptr += 3;
    }
    DoEvent(SDL_QUIT);
    CtrlAltShiftOff();
}
#endif
