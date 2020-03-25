#include <cstdlib>
#include <cstdio>

#include "settings.hh"
#include "beeper.hh"

#include <SDL.h>
#include <SDL_syswm.h>

#ifdef SDL_VIDEO_DRIVER_X11
# include <X11/Xlib.h>
#endif
#ifdef SDL_VIDEO_DRIVER_WINDOWS
# include <Utilapiset.h>
#endif

extern SDL_Window* window;

void BeepOn()
{
    if(Headless) return;

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
