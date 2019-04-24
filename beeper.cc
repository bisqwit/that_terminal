#include <cstdlib>
#include <cstdio>

#include "beeper.hh"

#include <SDL.h>
#include <SDL_syswm.h>

/* TODO: Remove X11 dependency (put it behind #ifdefs) */

#include <X11/Xlib.h>

extern SDL_Window* window;

void BeepOn()
{
  #ifdef _WIN32
    // TODO: Use some Microsoft thing
  #endif

    SDL_SysWMinfo info = {};
    SDL_VERSION(&info.version);
    if(!SDL_GetWindowWMInfo(window,&info))
        fprintf(stderr, "GetWindowWMInfo failed %s\n", SDL_GetError());

    auto display = info.info.x11.display;
    if(display)
    {
        XKeyboardControl ctrl = {};
        ctrl.bell_percent = 100;
        ctrl.bell_pitch   = 440;
        ctrl.bell_duration = 50;
        int ret1 = XChangeKeyboardControl(display, KBBellPitch|KBBellPercent|KBBellDuration, &ctrl);

        int ret = XBell(display, 100);
        std::fprintf(stderr, "belled? %d %d\n", ret1, ret);
    }
    else
    {
        std::fprintf(stderr, "display opening failed\n");
    }
}
