#ifdef RUN_TESTS
# include <gtest/gtest.h>
#endif

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
    ui.ResizeTo(8,8, 10,10);
    ui.SetWindowTitle("gruu");
    ui.SetIconName("gruu");
    ui.BeepOn();
    std::vector<std::uint32_t> pixbuf(10*10*8*8, 0xFF00FF);
    ui.PresentGraphics(&pixbuf[0]);
    ui.HandleEvents(true);
    ui.ResizeTo(16,16, 10,10);
}
#endif
