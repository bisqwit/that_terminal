#include <map>
#include <unordered_map>
#include <thread>
#include <atomic>

#include "share.hh"
#include "autoinput.hh"
#include "font_planner.hh"
#include "terminal.hh"
#include "forkpty.hh"
#include "window.hh"
#include "ctype.hh"
#include "clock.hh"
#include "ui.hh"

#include <poll.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

/* Settings */
// Settings for using terminal normally
static double TimeFactor = 1.0;        // You can simulate faster / slower system, 0 = as fast as possible
static bool AllowAutoInput = false;    // If enabled, reads inputter.dat and streams that into console
static double SimulatedFrameRate = 30; // Defines the time step for autoinput
static unsigned PollInterval = 1;      // If you use autoinput and nonzero timefactor,
                                       // you can increase this number to eliminate some syscalls.
                                       // Otherwise keep it as 1.
/* End settings */
namespace
{
    bool quit = false;                          ///< If set to true, terminal should immediately exit
    unsigned poll_counter = 0;                  ///< A counter for PollInterval. epoll() is only done when 0.

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
                    if(std::pair(arg[0],arg[1]) != ui.GetCellSize()
                    || std::pair(arg[2],arg[3]) != ui.GetWindowSize())
                    {
                        term.Resize(arg[2], arg[3]);
                        ui.ResizeTo(arg[0], arg[1], wnd.xsize, wnd.ysize);
                        tty.Resize(wnd.xsize, wnd.ysize);
                        wnd.Dirtify();
                        //again=true; // Don't set this flag. Render at least one frame before a new resize.
                        poll_counter = 0;
                        if(TimeFactor==0) std::this_thread::sleep_for(std::chrono::duration<float>(.1f));
                    }
                }
            }, GetAutoInput());

        return outbuffer;
    }

    std::string ProcessUIevents(Window& wnd, TerminalWindow& term, ForkPTY& tty)
    {
        auto ev = ui.HandleEvents(!AllowAutoInput || !AutoInputActive());
        std::visit([&](auto&& arg)
        {
            if constexpr(std::is_same_v<std::decay_t<decltype(arg)>, bool>)
            {
                quit = arg;
            }
            else if constexpr(std::is_same_v<std::decay_t<decltype(arg)>, std::pair<int,int>>)
            {
                // Window resize
                auto [w,h] = arg;
                auto [cellx,celly] = ui.GetCellSize();
                unsigned newxsize = w / cellx;
                unsigned newysize = h / celly;
                term.Resize(newxsize, newysize);
                tty.Resize(wnd.xsize, wnd.ysize);
                ui.ResizeTo(cellx,celly, wnd.xsize, wnd.ysize);
                wnd.Dirtify();
            }
            else if constexpr(std::is_same_v<std::decay_t<decltype(arg)>, std::pair<int,UI::resizetype>>)
            {
                auto [cellx,celly] = ui.GetCellSize();
                auto [winx,winy] = ui.GetWindowSize();
                switch(arg.second)
                {
                    case UI::resizetype::cellx:
                        cellx = std::clamp(int(cellx + arg.first), 4, 24);
                        break;
                    case UI::resizetype::celly:
                        celly = std::clamp(int(celly + arg.first), 5, 32);
                        break;
                    case UI::resizetype::winx:
                        term.Resize(wnd.xsize + arg.first, wnd.ysize);
                        break;
                    case UI::resizetype::winy:
                        term.Resize(wnd.xsize, wnd.ysize + arg.first);
                        break;
                }
                ui.ResizeTo(cellx,celly, wnd.xsize,wnd.ysize);
                tty.Resize(wnd.xsize, wnd.ysize);
                wnd.Dirtify();
            }
        }, std::move(ev.second));
        return ev.first;
    }
}

int main(int argc, char** argv)
{
    /* Set up */
    SaveArg0(argv[0]);
    SetTimeFactor(TimeFactor);

    Window         wnd(ui.GetWindowSize().first, ui.GetWindowSize().second);
    ForkPTY        tty(wnd.xsize, wnd.ysize);
    TerminalWindow term(wnd);

    ui.ResizeTo(ui.GetCellSize().first,
                ui.GetCellSize().second,
                wnd.xsize, wnd.ysize);

    if(AllowAutoInput)
    {
        AutoInputStart("inputter.dat");
    }

    /* Main loop */
    std::string outbuffer;
    std::vector<std::uint32_t> pixels;
    while(!quit)
    {
        auto [got_input, output_ok] = Poll(term, tty, !outbuffer.empty());
        if(quit) break;

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

        /* Process events from UI */
        outbuffer += ProcessUIevents(wnd, term, tty);

        /* Advance time */
        AdvanceTime(1 / SimulatedFrameRate);

        /* Redraw screen */
        auto a = ui.GetCellSize(), b = ui.GetWindowSize();
        pixels.resize(a.first*a.second * b.first*b.second);
        wnd.Render(a.first, a.second, &pixels[0]);
        ui.PresentGraphics(&pixels[0]);

        FontPlannerTick();
    }

    /* Cleanup */
    AutoInputEnd();
    TimeTerminate();
    tty.Kill(SIGHUP);
    tty.Close();
}
