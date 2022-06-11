#include <map>
#include <unordered_map>
#include <thread>
#include <atomic>

#include "share.hh"
#include "settings.hh"
#include "autoinput.hh"
#include "font_planner.hh"
#include "terminal.hh"
#include "forkpty.hh"
#include "window.hh"
#include "ctype.hh"
#include "clock.hh"
#include "keysym.hh"


#include <SDL.h>

#include <poll.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>


/* Settings */
// Settings for using terminal normally
static double TimeFactor = 1.0; // You can simulate faster / slower system, 0 = as fast as possible
bool Headless       = false; // Disables window creation (useless without autoinput & video recording)
bool AllowAutoInput = false; // If enabled, reads inputter.dat and streams that into console
static double SimulatedFrameRate = 30; // Defines the time step for autoinput
static unsigned PollInterval = 1; // If you use autoinput and nonzero timefactor,
                                  // you can increase this number to eliminate some syscalls.
                                  // Otherwise keep it as 1.
// Allow windows bigger than desktop? Setting this "true"
// also disables reacting to window resizes.
static bool Allow_Windows_Bigger_Than_Desktop = false;

unsigned VidCellWidth = 8, VidCellHeight = 16, WindowWidth  = 106, WindowHeight = 30;
static float ScaleX = 1.f;
static float ScaleY = 1.f;
/* End settings */

SDL_Window*  window   = nullptr; // external linkage, because needed by beeper.cc

namespace
{
    bool quit = false;                          ///< If set to true, terminal should immediately exit
    std::unordered_map<SDL_Keycode, bool> keys; ///< List of currently depressed keys
    unsigned poll_counter = 0;                  ///< A counter for PollInterval. epoll() is only done when 0.

    SDL_Renderer* renderer = nullptr;
    SDL_Texture*  texture  = nullptr;
    unsigned cells_horiz, cell_width_pixels,  pixels_width,  bufpixels_width, texturewidth;
    unsigned cells_vert,  cell_height_pixels, pixels_height, bufpixels_height, textureheight;
    std::vector<std::uint32_t> pixbuf;

    void SDL_ReInitialize(unsigned cells_horizontal, unsigned cells_vertical)
    {
        cells_horiz = cells_horizontal;
        cells_vert  = cells_vertical;
        cell_width_pixels  = VidCellWidth;
        cell_height_pixels = VidCellHeight;
        pixels_width  = cells_horizontal * cell_width_pixels,
        pixels_height = cells_vertical   * cell_height_pixels;
        bufpixels_width = cells_horizontal * VidCellWidth;
        bufpixels_height = cells_vertical  * VidCellHeight;

        if(!Headless)
        {
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
        }

        pixbuf.resize(bufpixels_width*bufpixels_height);

        fprintf(stderr, "Cells: %ux%u, pix sizes: %ux%u (%u), pixels: %ux%u, buf: %ux%u scale:%gx%g\n",
            cells_horiz,cells_vert,
            cell_width_pixels,cell_height_pixels, VidCellHeight,
            pixels_width,pixels_height,
            bufpixels_width,bufpixels_height,
            ScaleX, ScaleY);
    }
    void SDL_ReDraw(Window& wnd)
    {
        SDL_Rect rect;
        rect.x=0; rect.w=bufpixels_width;
        rect.y=0; rect.h=0;
        unsigned errors = 0;
        auto RenderFlushLines = [&]()
        {
            if(rect.h)
            {
                if(!Headless)
                {
                    int w,h;
                    SDL_GetWindowSize(window, &w,&h);

                    SDL_Rect trect;
                    trect.x = rect.x * w / bufpixels_width;
                    trect.y = rect.y * h / bufpixels_height;
                    trect.w = (rect.x + rect.w) * w / bufpixels_width - trect.x;
                    trect.h = (rect.y + rect.h) * h / bufpixels_height - trect.y;

                    if(SDL_UpdateTexture(texture, &rect,
                                         pixbuf.data() + rect.y*bufpixels_width,
                                         bufpixels_width*sizeof(pixbuf[0]))) ++errors;
                    if(SDL_RenderCopy(renderer, texture, &rect, &trect)) ++errors;
                }
                rect.y += rect.h;
                rect.h = 0;
            }
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
        if(!Headless)
        {
            wnd.Render(VidCellWidth,VidCellHeight, &pixbuf[0]);
            for(unsigned y=0; y<cells_vert*VidCellHeight; ++y)
                RenderAddLine(y);

            RenderFlushLines();
            if(!Headless)
            {
                if(rect.y) { SDL_RenderPresent(renderer); }
            }
        }
    }

    std::string HandleSDLevents(Window& wnd, TerminalWindow& term, ForkPTY& tty)
    {
        std::string outbuffer, pending_input;
        for(SDL_Event ev; SDL_PollEvent(&ev); )
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
                                if(w != pixels_width*ScaleX
                                || h != pixels_height*ScaleY)
                                {
                                    unsigned newxsize = (w/ScaleX) / VidCellWidth;
                                    unsigned newysize = (h/ScaleY) / VidCellHeight;
                                    term.Resize(newxsize, newysize);
                                    SDL_ReInitialize(wnd.xsize, wnd.ysize);
                                    tty.Resize(WindowWidth = wnd.xsize, WindowHeight = wnd.ysize);
                                }
                                wnd.Dirtify();
                            }
                            break;
                        }
                        default:
                            break;
                    }
                    break;
                case SDL_QUIT:
                    quit = true;
                    break;
                case SDL_TEXTINPUT:
                    //std::fprintf(stderr, "Text input(%s)\n", ev.text.text);
                    if(!AllowAutoInput || !AutoInputActive())
                    {
                        pending_input.clear(); // Overrides any input events from SDL_KEYDOWN
                        outbuffer += ev.text.text;
                    }
                    break;
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                {
                    keys[ev.key.keysym.sym] = (ev.type == SDL_KEYDOWN);
                    if((!AllowAutoInput || !AutoInputActive()) && ev.type == SDL_KEYDOWN)
                    {
                        bool shift = keys[SDLK_LSHIFT] || keys[SDLK_RSHIFT];
                        bool alt   = keys[SDLK_LALT]   || keys[SDLK_RALT];
                        bool ctrl  = keys[SDLK_LCTRL]  || keys[SDLK_RCTRL];
                        /*
                        F1 decrease rows
                        F2 increase rows
                        F3 decrease columns
                        F4 increase columns
                        */
                        bool processed = false;
                        bool resized   = false;
                        if(!shift && !alt && !ctrl)
                            switch(ev.key.keysym.sym)
                            {
                                case SDLK_F1: term.Resize(wnd.xsize, wnd.ysize-1); resized = true; break;
                                case SDLK_F2: term.Resize(wnd.xsize, wnd.ysize+1); resized = true; break;
                                case SDLK_F3: term.Resize(wnd.xsize-1, wnd.ysize); resized = true; break;
                                case SDLK_F4: term.Resize(wnd.xsize+1, wnd.ysize); resized = true; break;
                                case SDLK_F5: if(VidCellHeight > 5) --VidCellHeight; resized = true; break;
                                case SDLK_F6: if(VidCellHeight < 32) ++VidCellHeight; resized = true; break;
                                // Allow widths 6, 8 and 9
                                case SDLK_F7: if(VidCellWidth > 4)  --VidCellWidth; resized = true; break;
                                case SDLK_F8: if(VidCellWidth < 24) ++VidCellWidth; resized = true; break;
                                case SDLK_F9:
                                    if(ScaleY >= 2) --ScaleY;
                                    else             ScaleY = ScaleY/std::sqrt(2.f);
                                    if(ScaleY < 1.5) ScaleY = 1;
                                    resized = true;
                                    break;
                                case SDLK_F10:
                                    if(ScaleY < 2) ScaleY = ScaleY*std::sqrt(2.f);
                                    else           ++ScaleY;
                                    if(ScaleY >= 1.9) ScaleY = int(ScaleY+0.1);
                                    resized = true;
                                    break;
                                case SDLK_F11:
                                    if(ScaleX >= 2) --ScaleX;
                                    else            ScaleX = ScaleX/std::sqrt(2.f);
                                    if(ScaleX < 1.5) ScaleX = 1;
                                    resized = true;
                                    break;
                                case SDLK_F12:
                                    if(ScaleX < 2) ScaleX = ScaleX*std::sqrt(2.f);
                                    else           ++ScaleX;
                                    if(ScaleX >= 1.9) ScaleX = int(ScaleY+0.1);
                                    resized = true;
                                    break;
                            }
                        if(resized)
                        {
                            SDL_ReInitialize(wnd.xsize, wnd.ysize);
                            //tty.Kill(SIGWINCH);
                            tty.Resize(WindowWidth = wnd.xsize, WindowHeight = wnd.ysize);
                            wnd.Dirtify();
                            processed = true;
                        }
                        if(!processed)
                        {
                            auto str = InterpretInput(shift, alt, ctrl, ev.key.keysym.sym);
                            if(!str.empty())
                            {
                                // Put the input in "pending_input", so that it gets automatically
                                // cancelled if a textinput event is generated.
                                pending_input += str;
                            }
                        }
                    }
                    break;
                }
            }
        return outbuffer + pending_input;
    }

    std::string ProcessAutoInputs(Window& wnd, TerminalWindow& term, ForkPTY& tty)
    {
        std::string outbuffer;
        for(bool again=true; again; )
            again=false, std::visit([&](auto&& arg)
            {
                if constexpr(std::is_same_v<std::decay_t<decltype(arg)>, std::string>)
                {
                    if(!arg.empty())
                    {
                        outbuffer += std::move(arg);
                        again = true;
                        poll_counter = 0;

                        int r = tty.Send(outbuffer);
                        if(r > 0)
                            outbuffer.erase(0, r);
                    }
                }
                else if constexpr(std::is_same_v<std::decay_t<decltype(arg)>, std::array<unsigned,4>>)
                {
                    if(std::array{VidCellWidth,VidCellHeight,WindowWidth,WindowHeight} != arg)
                    {
                        VidCellWidth = arg[0];
                        VidCellHeight = arg[1];
                        term.Resize(arg[2], arg[3]);
                        SDL_ReInitialize(wnd.xsize, wnd.ysize);
                        tty.Resize(WindowWidth = wnd.xsize, WindowHeight = wnd.ysize);
                        wnd.Dirtify();
                        //again=true; // Don't set this flag. Render at least one frame before a new resize.
                        poll_counter = 0;
                        if(TimeFactor==0) std::this_thread::sleep_for(std::chrono::duration<float>(.1f));
                    }
                }
            }, GetAutoInput());

        return outbuffer;
    }

    std::pair<bool,bool> Poll(TerminalWindow& term, ForkPTY& tty, bool outgoing)
    {
        struct pollfd p[2] = { { tty.getfd(), POLLIN, 0 } };
        if(poll_counter != 0)
        {
            p[0].events = 0;
        }
        if(!term.OutBuffer.empty() || outgoing)
        {
            p[0].events |= POLLOUT;
        }
        if((p[0].events) && TimeFactor != 0.0)
        {
            int pollres = poll(p, 1, 1000/(SimulatedFrameRate*TimeFactor));
            if(pollres < 0) // If the poll failed, terminate
            {
                quit = true;
                return {false,false};
            }
        }
        if((p[0].revents & POLLIN) || (TimeFactor==0.0))
        {
            bool loop = true;
            while(loop)
            {
                auto input = tty.Recv();
                if(input.second == -1 && errno == EIO) quit = true;
                auto& str = input.first;
            #if 1
                term.Write(FromUTF8(str));
            #else
                term.Write(FromCP437(str));
            #endif
                if(input.first.empty() || TimeFactor != 0.0) loop = false;
            }
        }
        else
        {
            poll_counter = (poll_counter+1) % PollInterval;
        }

        /* Terminate if the subprocess terminated */
        if(p[0].revents & (POLLERR | POLLHUP))
        {
            quit = true;
        }

        return { bool(p[0].revents & POLLIN), bool(p[0].revents & POLLOUT) };
    }
}

int main(int argc, char** argv)
{
    /* Set up */
    SaveArg0(argv[0]);
    SetTimeFactor(TimeFactor);

    Window         wnd(WindowWidth, WindowHeight);
    ForkPTY        tty(wnd.xsize, wnd.ysize);
    TerminalWindow term(wnd);

    SDL_ReInitialize(wnd.xsize, wnd.ysize);

    if(!Headless)
    {
        SDL_StartTextInput();
    }

    if(AllowAutoInput)
    {
        AutoInputStart();
    }

    /* Main loop */
    std::string outbuffer;
    while(!quit)
    {
        auto [got_input, output_ok] = Poll(term, tty, !outbuffer.empty());

        /* If terminal itself generates input, add those to the output buffer */
        if(!term.OutBuffer.empty())
        {
            std::u32string str(term.OutBuffer.begin(), term.OutBuffer.end());
            outbuffer += ToUTF8(str);
            term.OutBuffer.clear();
        }
        /* If we have text to send to subprocess, do it at earliest opportunity */
        if(!outbuffer.empty() && (output_ok || !got_input))
        {
            int r = tty.Send(outbuffer);
            if(r > 0)
                outbuffer.erase(0, r);
            poll_counter = 0;
        }
        /* Process autoinputs unless we are busy reading from subprocess */
        if(!got_input)
        {
            outbuffer += ProcessAutoInputs(wnd, term, tty);
        }

        /* Process SDL events unless we are in headless mode */
        if(!Headless)
        {
            outbuffer += HandleSDLevents(wnd, term, tty);
        }

        /* Advance time */
        AdvanceTime(1 / SimulatedFrameRate);
        SDL_ReDraw(wnd);
        FontPlannerTick();
    }

    /* Cleanup */
    AutoInputEnd();
    TimeTerminate();
    tty.Kill(SIGHUP);
    tty.Close();
    SDL_Quit();
}
