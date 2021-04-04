#include <map>
#include <unordered_map>

#include <SDL.h>

#include "settings.hh"
#include "autoinput.hh"
#include "terminal.hh"
#include "forkpty.hh"
#include "screen.hh"
#include "ctype.hh"
#include "clock.hh"

#include <poll.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <thread>
#include <atomic>

/* Settings */
#if 0
// Settings for making autoinput videos...
static double TimeFactor = 0.0; // You can simulate faster / slower system, 0 = as fast as possible
bool Headless       = true; // Disables window creation (useless without autoinput & video recording)
bool EnableTimeTemp = true; // Enables substitution of $H:$M:$S and $TEMP in rendering
bool AllowAutoInput = true; // If enabled, reads inputter.dat and streams that into console
bool DoVideoRecording = true; // Needs ffmpeg, creates files as .term_videos/frame*.mp4
bool IgnoreScale    = true;
static double SimulatedFrameRate = 240; // Defines the time step for autoinput
static double VideoFrameRate = 240;     // Defines the video recording framerate
static unsigned PollInterval = 60; // If you use autoinput and nonzero timefactor,
                                  // you can increase this number to eliminate some syscalls.
                                  // Otherwise keep it as 1.
static unsigned VideoFrameRateDownSampleFactor = 4; // Use powers of 2 only
static bool Allow_Windows_Bigger_Than_Desktop = true;
#elif 0
// Settings for *testing* autoinput videos...
static double TimeFactor = 0.0; // You can simulate faster / slower system, 0 = as fast as possible
bool Headless       = false; // Disables window creation (useless without autoinput & video recording)
bool EnableTimeTemp = true; // Enables substitution of $H:$M:$S and $TEMP in rendering
bool AllowAutoInput = true; // If enabled, reads inputter.dat and streams that into console
bool DoVideoRecording = true; // Needs ffmpeg, creates files as .term_videos/frame*.mp4
bool IgnoreScale    = false;
static double SimulatedFrameRate = 60; // Defines the time step for autoinput
static double VideoFrameRate = 60;     // Defines the video recording framerate
static unsigned PollInterval = 1; // If you use autoinput and nonzero timefactor,
                                  // you can increase this number to eliminate some syscalls.
                                  // Otherwise keep it as 1.
static unsigned VideoFrameRateDownSampleFactor = 1; // Use powers of 2 only
static bool Allow_Windows_Bigger_Than_Desktop = true;
#elif 0
// Settings for *testing* autoinput videos... *quicker*
static double TimeFactor = 0.0; // You can simulate faster / slower system, 0 = as fast as possible
bool Headless       = false; // Disables window creation (useless without autoinput & video recording)
bool EnableTimeTemp = true; // Enables substitution of $H:$M:$S and $TEMP in rendering
bool AllowAutoInput = true; // If enabled, reads inputter.dat and streams that into console
bool DoVideoRecording = true; // Needs ffmpeg, creates files as .term_videos/frame*.mp4
bool IgnoreScale    = false;
static double SimulatedFrameRate = 60; // Defines the time step for autoinput
static double VideoFrameRate = 30;     // Defines the video recording framerate
static unsigned PollInterval = 1; // If you use autoinput and nonzero timefactor,
                                  // you can increase this number to eliminate some syscalls.
                                  // Otherwise keep it as 1.
static unsigned VideoFrameRateDownSampleFactor = 1; // Use powers of 2 only
static bool Allow_Windows_Bigger_Than_Desktop = true;
#elif 0
// Settings for create videos manually using the terminal
static double TimeFactor = 1.0; // You can simulate faster / slower system, 0 = as fast as possible
bool Headless       = false; // Disables window creation (useless without autoinput & video recording)
bool EnableTimeTemp = true; // Enables substitution of $H:$M:$S and $TEMP in rendering
bool AllowAutoInput = false; // If enabled, reads inputter.dat and streams that into console
bool DoVideoRecording = true; // Needs ffmpeg, creates files as .term_videos/frame*.mp4
bool IgnoreScale    = false;
static double SimulatedFrameRate = 60; // Defines the time step for autoinput
static double VideoFrameRate = 60;     // Defines the video recording framerate
static unsigned PollInterval = 1; // If you use autoinput and nonzero timefactor,
                                  // you can increase this number to eliminate some syscalls.
                                  // Otherwise keep it as 1.
static unsigned VideoFrameRateDownSampleFactor = 1; // Use powers of 2 only
// Allow windows bigger than desktop? Setting this "true"
// also disables reacting to window resizes.
static bool Allow_Windows_Bigger_Than_Desktop = false;
#else
// Settings for using terminal normally
static double TimeFactor = 1.0; // You can simulate faster / slower system, 0 = as fast as possible
bool Headless       = false; // Disables window creation (useless without autoinput & video recording)
bool EnableTimeTemp = false; // Enables substitution of $H:$M:$S and $TEMP in rendering
bool AllowAutoInput = false; // If enabled, reads inputter.dat and streams that into console
bool DoVideoRecording = false; // Needs ffmpeg, creates files as .term_videos/frame*.mp4
bool IgnoreScale    = false;
static double SimulatedFrameRate = 30; // Defines the time step for autoinput
static double VideoFrameRate = 30;     // Defines the video recording framerate
static unsigned PollInterval = 1; // If you use autoinput and nonzero timefactor,
                                  // you can increase this number to eliminate some syscalls.
                                  // Otherwise keep it as 1.
static unsigned VideoFrameRateDownSampleFactor = 1; // Use powers of 2 only
// Allow windows bigger than desktop? Setting this "true"
// also disables reacting to window resizes.
static bool Allow_Windows_Bigger_Than_Desktop = false;
#endif



unsigned VidCellWidth = 8, VidCellHeight = 12, WindowWidth  =129, WindowHeight = 40;
//unsigned VidCellWidth = 9, VidCellHeight = 8, WindowWidth  =126, WindowHeight = 60;
//unsigned VidCellWidth = 9, VidCellHeight = 14, WindowWidth  =126, WindowHeight = 35;
//unsigned VidCellWidth = 9, VidCellHeight = 12, WindowWidth  =109, WindowHeight = 46;
//unsigned VidCellWidth = 9, VidCellHeight = 10, WindowWidth  =126, WindowHeight = 48;
//unsigned VidCellWidth = 8, VidCellHeight = 8, WindowWidth  =126, WindowHeight = 292;
//unsigned VidCellWidth = 8, VidCellHeight = 16, WindowWidth  = 106, WindowHeight = 30;
//unsigned VidCellWidth = 8, VidCellHeight = 14, WindowWidth  = 106, WindowHeight = 34;
//static unsigned WindowWidth  = 80, WindowHeight = 25;
//static float ScaleX = 3.f;
//static float ScaleY = 3.5f;
static float ScaleX = 2.f;
static float ScaleY = 2.f;
//static float ScaleX = 1.0f;
//static float ScaleY = 1.0f;

/* End settings */

SDL_Window*  window   = nullptr; // external linkage, because needed by beeper.cc



namespace
{
    SDL_Renderer* renderer = nullptr;
    SDL_Texture*  texture  = nullptr;
    unsigned cells_horiz, cell_width_pixels,  pixels_width,  bufpixels_width, texturewidth;
    unsigned cells_vert,  cell_height_pixels, pixels_height, bufpixels_height, textureheight;
    std::vector<std::uint32_t> pixbuf;

    std::atomic<unsigned long> frames_done     = 0;
    std::atomic<unsigned long> frames_thisfile = 0;

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
        if(IgnoreScale)
        {
            ScaleX = 1;
            ScaleY = 1;
        }

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
                    //fprintf(stderr, "updated %u-%u\n", rect.y, rect.y+rect.h);
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
        bool rendered = false;
        bool dummy_video = false;//GetTime() < 16*60;
        auto DoRendering = [&]()
        {
            if(dummy_video)
            {
            }
            else
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
            rendered = true;
        };
        //std::fprintf(stderr, "\rFR at %10.6f", GetTime());

        if(!DoVideoRecording || !Headless)
        {
            DoRendering();
        }
        if(DoVideoRecording)
        {
            double fps = VideoFrameRate;
            static FILE* fp = nullptr;
            static unsigned pwidth=0, pheight=0;
            static auto begin = GetTime();
            static std::vector<unsigned char> buffer;
            static std::vector<unsigned char> lastframe;

            static unsigned counter = 0;
            auto Open = [&](bool quick)
            {
                char fnbuf[256];
                mkdir(".term_videos", 0755);
                std::sprintf(fnbuf, ".term_videos/frames%04d.%s", counter++, quick?"mp4":"mkv");
                std::remove(fnbuf);
                std::fprintf(stderr, "Opening %s\n", fnbuf);

                std::string encoder =
                    " -c:v h264_nvenc -profile high444p -pixel_format yuv444p -preset losslesshp";
                if(quick || true)
                    encoder = " -c:v libx264 -crf 0 -threads 16 -preset faster -g 120"
                              " -x264-params 'subme=0' -me_method dia -b-pyramid none -rc-lookahead 1";
                std::string filter_complex;
                for(unsigned down=1; down<VideoFrameRateDownSampleFactor; down *= 2)
                {
                    if(!filter_complex.empty()) filter_complex += ',';
                    filter_complex += "tblend=average,fps=" + std::to_string(fps / (down*2));
                }
                if(!filter_complex.empty())
                {
                    filter_complex =
                        " -filter_complex \"" + filter_complex + "\""
                        " -r " + std::to_string(fps / VideoFrameRateDownSampleFactor);
                }
                std::string cmd = "ffmpeg -y -f rawvideo -pix_fmt bgra"
                    " -s:v "+std::to_string(pwidth)+"x"+std::to_string(pheight)+
                    " -r "+std::to_string(fps)+
                    " -i -"
                    " -aspect 16/9"
                    " -loglevel error"
                    + encoder
                    + filter_complex
                    + " " + (fnbuf);
                //cmd = "dd bs=8G iflag=fullblock 2>/dev/null | "+cmd;
                fp = popen(cmd.c_str(), "w");
                setbuffer(fp, nullptr, pixbuf.size()*sizeof(std::uint32_t) * 1);
                /*
                sudo sysctl fs.pipe-user-pages-soft=0
                sudo sysctl fs.pipe-max-size=$[1048576*512]
                sudo setcap 'CAP_SYS_RESOURCE=+ep' term
                */
                int prev_err=0;
                for(unsigned power=41; power>10; --power)
                {
                    int r = 0;
                    for(int tries=0; tries<4000; ++tries)
                    {
                        r = fcntl(fileno(fp), F_SETPIPE_SZ, 1ul<<power);
                        if(r >= 0) break;
                    }
                    if(r >= 0)
                    {
                        std::fprintf(stderr, "Pipe size successfully set to %lu (r=%d)\n", 1ul<<power, r);
                        break;
                    }
                    else
                    {
                        if(errno != prev_err)
                        {
                            std::fprintf(stderr, "Failed to set pipe size to %lu; ", 1ul<<power);
                            std::perror("fcntl");
                            prev_err=errno;
                        }
                    }
                }
                int s = fcntl(fileno(fp), F_GETPIPE_SZ);
                if(s > 0)
                    std::fprintf(stderr, "Pipe size is %d bytes\n", s);
                else
                    std::perror("fcntl");
            };
            auto SafeWrite = [](const void* buffer, std::size_t elemsize, std::size_t nelems, std::FILE* fp)
            {
                const char* source = static_cast<const char*>(buffer);;
                std::size_t p = 0, limit = elemsize * nelems;
                while(p < limit)
                {
                    int r = std::fwrite(source+p, 1, limit-p, fp);
                    if(r > 0) p += r;
                    if(r <= 0) break;
                }
            };

            unsigned targetwidth = bufpixels_width, targetheight = bufpixels_height;
            if(dummy_video)
            {
                targetwidth  = 256;
                targetheight = 144;
            }
            if(pwidth != targetwidth || pheight != targetheight)
            {
                if(!fp && !buffer.empty())
                {
                    Open(true);
                }
                if(fp)
                {
                    std::atomic<bool> ent{};
                    std::thread closer([&ent,&SafeWrite,//&frames_done,
                                       f = fp, fr = 0+frames_thisfile, buf = std::move(buffer), last = lastframe]()
                    {
                        ent = true;
                        SafeWrite(&buf[0], 1, buf.size(), f);
                        unsigned long frr = fr;
                        while(frr % (VideoFrameRateDownSampleFactor*2) != 0 || frr == 0)
                        {
                            // Write duplicates of the last frame
                            SafeWrite(&last[0], 1, last.size(), f);
                            ++frr;
                            ++frames_done;
                        }
                        std::fprintf(stderr, "\33[1mWrote %lu frames (%zd bytes)\33[m\n",
                            (unsigned long)frr, std::ftell(f));
                        pclose(f);
                    });
                    while(!ent) std::this_thread::sleep_for(std::chrono::duration<float>(.1f));
                    closer.detach();
                    fp    = nullptr;
                    //begin = GetTime();
                    //frames_done     = 0;
                    frames_thisfile = 0;
                }

                pwidth  = targetwidth;
                pheight = targetheight;

                buffer.clear();
            }

            if(true)
            {
                auto end = GetTime();
                double d = end - begin;
                unsigned long frames_should = d * fps;
                bool do_at_least_one = false;
                if(frames_thisfile == 0 && frames_should <= frames_done && SimulatedFrameRate == VideoFrameRate)
                {
                    do_at_least_one = true;
                }
                unsigned words = pwidth*pheight;
                unsigned bytes = words*sizeof(std::uint32_t);

                //bool flag = false;
                for(; frames_done < frames_should || do_at_least_one; ++frames_done, ++frames_thisfile)
                {
                    if(!rendered) DoRendering();

                    static unsigned prev = 0;
                    unsigned clock = 10 * (
                        //19*60+30+
                        frames_done / fps);
                    if(clock != prev)
                    {
                        std::fprintf(stderr, "%02d:%04.1f\r", clock/600, (clock%600)*0.1);
                        prev = clock;
                    }

                    if(!fp && buffer.size() >= bytes*16)
                    {
                        //unsigned swidth  = pwidth  * std::max(1u, (3840+pwidth-1)/pwidth);
                        //unsigned sheight = pheight * std::max(1u, (2160+pheight-1)/pheight);

                        //unsigned swidth  = pwidth  * std::max(1u, (1920+pwidth-1)/pwidth);
                        //unsigned sheight = pheight * std::max(1u, (1080+pheight-1)/pheight);
                        Open(false);
                    }

                    lastframe.assign((char*)&pixbuf[0], (char*)(&pixbuf[0]+words));
                    if(fp)
                    {
                        if(!buffer.empty())
                        {
                            SafeWrite(&buffer[0], 1, buffer.size(), fp);
                            buffer.clear();
                        }
                        SafeWrite(&pixbuf[0], sizeof(std::uint32_t), words, fp);
                    }
                    else
                    {
                        buffer.insert(buffer.end(), (char*)&pixbuf[0], (char*)(&pixbuf[0]+words));
                    }
                    do_at_least_one = false;
                }
            }
        }
    }
}

int main()
{
    SetTimeFactor(TimeFactor);

    //SDL_Init(SDL_INIT_EVERYTHING);
    Window wnd(WindowWidth, WindowHeight);
    termwindow term(wnd);
    ForkPTY tty(wnd.xsize, wnd.ysize);
    std::string outbuffer;

    SDL_ReInitialize(wnd.xsize, wnd.ysize);
    if(!Headless)
    {
        SDL_StartTextInput();
    }

    if(AllowAutoInput)
    {
        AutoInputStart();
    }

    std::unordered_map<int, bool> keys;
    bool quit = false;
    unsigned expect = 0;
    while(!quit)
    {
        struct pollfd p[2] = { { tty.getfd(), POLLIN, 0 } };
        if(expect != 0) p[0].events = 0;
        if(!term.OutBuffer.empty() || !outbuffer.empty())
        {
            p[0].events |= POLLOUT;
        }
        if((p[0].events) && TimeFactor != 0.0)
        {
            int pollres = poll(p, 1, 1000/(SimulatedFrameRate*TimeFactor));
            if(pollres < 0) break;
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
            expect = (expect+1) % PollInterval;
        }
        if(p[0].revents & (POLLERR | POLLHUP))
        {
            quit = true;
        }

        if(!term.OutBuffer.empty())
        {
            std::u32string str(term.OutBuffer.begin(), term.OutBuffer.end());
            outbuffer += ToUTF8(str);
            term.OutBuffer.clear();
        }
        if(!outbuffer.empty() && ((p[0].revents & POLLOUT) || !(p[0].revents)))
        {
            int r = tty.Send(outbuffer);
            if(r > 0)
                outbuffer.erase(0, r);
            expect = 0;
        }

        if(!(p[0].revents & POLLIN))
            for(bool again=true; again; )
                again=false, std::visit([&](auto&& arg)
                {
                    if constexpr(std::is_same_v<std::decay_t<decltype(arg)>, std::string>)
                    {
                        outbuffer += std::move(arg);
                        again=true;
                        expect = 0;

                        int r = tty.Send(outbuffer);
                        if(r > 0)
                            outbuffer.erase(0, r);
                    }
                    else if constexpr(std::is_same_v<std::decay_t<decltype(arg)>, std::array<unsigned,4>>)
                    {
                        if(std::array{VidCellWidth,VidCellHeight,WindowWidth,WindowHeight} != arg)
                        {
                            if(DoVideoRecording && TimeFactor==0)
                            {
                                while(frames_thisfile < (VideoFrameRateDownSampleFactor*2))
                                {
                                    AdvanceTime(1 / SimulatedFrameRate);
                                    SDL_ReDraw(wnd);
                                }
                            }
                            if(IgnoreScale)
                            {
                                ScaleX = 1.0f;
                                ScaleY = 1.0f;
                            }
                            VidCellWidth = arg[0];
                            VidCellHeight = arg[1];
                            term.Resize(arg[2], arg[3]);
                            SDL_ReInitialize(wnd.xsize, wnd.ysize);
                            tty.Resize(WindowWidth = wnd.xsize, WindowHeight = wnd.ysize);
                            wnd.Dirtify();
                            //again=true; // Don't set this flag. Render at least one frame before a new resize.
                            expect = 0;
                            if(TimeFactor==0) std::this_thread::sleep_for(std::chrono::duration<float>(.1f));
                        }
                    }
                }, GetAutoInput());

        if(!Headless)
        {
            std::string pending_input;
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
                            static const std::unordered_map<int, std::pair<int/*num*/,char/*letter*/>> lore
                            {
                                { SDLK_F1,       {1,'P'} },  { SDLK_LEFT,     {1,'D'} },
                                { SDLK_F2,       {1,'Q'} },  { SDLK_RIGHT,    {1,'C'} },
                                { SDLK_F3,       {1,'R'} },  { SDLK_UP,       {1,'A'} },
                                { SDLK_F4,       {1,'S'} },  { SDLK_DOWN,     {1,'B'} },
                                { SDLK_F5,       {15,'~'} }, { SDLK_HOME,     {1,'H'} },
                                { SDLK_F6,       {17,'~'} }, { SDLK_END,      {1,'F'} },
                                { SDLK_F7,       {18,'~'} }, { SDLK_INSERT,   {2,'~'} },
                                { SDLK_F8,       {19,'~'} }, { SDLK_DELETE,   {3,'~'} },
                                { SDLK_F9,       {20,'~'} }, { SDLK_PAGEUP,   {5,'~'} },
                                { SDLK_F10,      {21,'~'} }, { SDLK_PAGEDOWN, {6,'~'} },
                                { SDLK_F11,      {23,'~'} },
                                { SDLK_F12,      {24,'~'} },
                            };
                            static const std::unordered_map<int, char> lore2
                            {
                                { SDLK_a, 'a' }, { SDLK_b, 'b' }, { SDLK_c, 'c' }, { SDLK_d, 'd' },
                                { SDLK_e, 'e' }, { SDLK_f, 'f' }, { SDLK_g, 'g' }, { SDLK_h, 'h' },
                                { SDLK_i, 'i' }, { SDLK_j, 'j' }, { SDLK_k, 'k' }, { SDLK_l, 'l' },
                                { SDLK_m, 'm' }, { SDLK_n, 'n' }, { SDLK_o, 'o' }, { SDLK_p, 'p' },
                                { SDLK_q, 'q' }, { SDLK_r, 'r' }, { SDLK_s, 's' }, { SDLK_t, 't' },
                                { SDLK_u, 'u' }, { SDLK_v, 'v' }, { SDLK_w, 'w' }, { SDLK_x, 'x' },
                                { SDLK_y, 'y' }, { SDLK_z, 'z' }, { SDLK_ESCAPE, '\33' },
                                { SDLK_0, '0' }, { SDLK_1, '1' }, { SDLK_2, '2' }, { SDLK_3, '3' },
                                { SDLK_4, '4' }, { SDLK_5, '5' }, { SDLK_6, '6' }, { SDLK_7, '7' },
                                { SDLK_8, '8' }, { SDLK_9, '9' }, { SDLK_PERIOD, '.' },
                                { SDLK_COMMA, ',' }, { SDLK_SLASH, '-' },
                                { SDLK_RETURN, '\r' }, { SDLK_BACKSPACE, '\177' }, { SDLK_TAB, '\t' },
                                { SDLK_SPACE, ' ' }, { SDLK_KP_ENTER, '\r' },
                            };
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
                            if(processed)
                            {}
                            else if(auto i = lore.find(ev.key.keysym.sym); i != lore.end())
                            {
                                const auto& d = i->second;
                                unsigned delta = 1 + shift*1 + alt*2 + ctrl*4, len;
                                char bracket = '[', Buf[16];
                                if(d.second >= 'P' && d.second <= 'S') bracket = 'O';
                                if(d.second >= 'A' && d.second <= 'D' && delta == 1) bracket = 'O'; // less requires this for up&down, alsamixer requires this for left&right
                                if(delta != 1)
                                    len = std::sprintf(Buf, "\33%c%d;%d%c", bracket, d.first, delta, d.second);
                                else if(d.first == 1)
                                    len = std::sprintf(Buf, "\33%c%c", bracket, d.second);
                                else
                                    len = std::sprintf(Buf, "\33%c%d%c", bracket, d.first, d.second);
                                pending_input.append(Buf,len);
                            }
                            else if(auto i = lore2.find(ev.key.keysym.sym); i != lore2.end())
                            {
                                char32_t cval = i->second;
                                bool digit = cval >= '0' && cval <= '9', alpha = cval >= 'a' && cval <= 'z';
                                if(shift && alpha) cval &= ~0x20; // Turn uppercase
                                if(ctrl && digit) cval = "01\0\33\34\35\36\37\1779"[cval-'0'];
                                if(ctrl && i->second=='\177') cval = '\b';
                                else if(ctrl && !digit) cval &= 0x1F; // Turn into a control character
                                // CTRL a..z becomes 01..1A
                                // CTRL 0..9 becomes 10..19, should become xx,xx,00,1B-1F,7F,xx
                                if(alt) cval |= 0x80;  // Add ALT
                                if((!alpha && !digit) || ctrl||alt)
                                {
                                    //std::fprintf(stderr, "lore input(%c)\n", char(cval));
                                    if(shift && cval == '\t') pending_input += "\33[Z";
                                    else pending_input += ToUTF8(std::u32string_view(&cval,1));
                                }
                                // Put the input in "pending_input", so that it gets automatically
                                // cancelled if a textinput event is generated.
                            }
                        }
                        break;
                    }
                }
            outbuffer += std::move(pending_input);
        }

        AdvanceTime(1 / SimulatedFrameRate);
        SDL_ReDraw(wnd);
    }
    AutoInputEnd();
    TimeTerminate();
    tty.Kill(SIGHUP);
    tty.Close();
    SDL_Quit();
}
