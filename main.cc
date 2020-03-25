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
static double TimeFactor = 1.0; // You can simulate faster / slower system, 0 = as fast as possible
bool Headless       = false; // Disables window creation (useless without autoinput & video recording)
bool EnableTimeTemp = false; // Enables substitution of $H:$M:$S and $TEMP in rendering
bool AllowAutoInput = false; // If enabled, reads inputter.dat and streams that into console
bool DoVideoRecording = false; // Needs ffmpeg, creates files as .term_videos/frame*.mp4
static double AimedFrameRate = 30; // Defines also video recording framerate
static unsigned PollInterval = 64;


// Allow windows bigger than desktop? Setting this "true"
// also disables reacting to window resizes.
static bool Allow_Windows_Bigger_Than_Desktop = false;


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
//static float ScaleX = 2.f;
//static float ScaleY = 2.f;
static float ScaleX = 1.0f;
static float ScaleY = 1.0f;

/* End settings */

SDL_Window*  window   = nullptr; // external linkage, because needed by beeper.cc



namespace
{
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
        ScaleX = 1;
        ScaleY = 1;

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
        //std::fprintf(stderr, "\rFR at %10.6f", GetTime());

        wnd.Render(VidCellWidth,VidCellHeight, &pixbuf[0]);
        for(unsigned y=0; y<cells_vert*VidCellHeight; ++y)
            RenderAddLine(y);

        RenderFlushLines();
        if(!Headless)
        {
            if(rect.y) { SDL_RenderPresent(renderer); }
        }

        if(DoVideoRecording)
        {
            double fps = AimedFrameRate;
            static FILE* fp = nullptr;
            static unsigned pwidth=0, pheight=0;
            static auto begin = GetTime();
            static unsigned long frames_done = 0;
            static std::vector<unsigned char> buffer;

            static unsigned counter = 0;
            auto Open = [&](bool quick)
            {
                char fnbuf[256];
                mkdir(".term_videos", 0755);
                std::sprintf(fnbuf, ".term_videos/frames%04d.mp4", counter++);
                std::remove(fnbuf);
                std::fprintf(stderr, "Opening %s\n", fnbuf);

                std::string encoder =
                    " -c:v h264_nvenc -profile high444p -pixel_format yuv444p -preset losslesshp";
                if(quick || true)
                    encoder = " -c:v libx264 -crf 0 -threads 16 -preset veryslow -g 960"
                              " -x264-params 'subme=0' -me_method dia -b-pyramid none -rc-lookahead 1";
                std::string cmd = "ffmpeg -y -f rawvideo -pix_fmt bgra"
                    " -s:v "+std::to_string(pwidth)+"x"+std::to_string(pheight)+
                    " -r "+std::to_string(fps)+
                    " -i -"
                    " -aspect 16/9"// -sws_flags neighbor -vf scale=3840:2160"
                    " -loglevel error"
                    + encoder
                //    " -sws_flags neighbor"
                //    " -vf scale="+std::to_string(swidth)+":"+std::to_string(sheight)+
                    + " " + (fnbuf);
                cmd = "dd bs=8G iflag=fullblock 2>/dev/null | "+cmd;
                fp = popen(cmd.c_str(), "w");
                setbuffer(fp, nullptr, pixbuf.size()*sizeof(std::uint32_t) * 1);
            };
            auto SafeWrite = [&](const void* buffer, std::size_t elemsize, std::size_t nelems, std::FILE* fp)
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

            if(pwidth != bufpixels_width || pheight != bufpixels_height)
            {
                if(!fp && !buffer.empty()) Open(true);
                if(fp)
                {
                    std::atomic<bool> ent{};

                    std::thread closer([&ent,&SafeWrite, f = fp, buf = std::move(buffer)]()
                    {
                        ent = true;
                        SafeWrite(&buf[0], 1, buf.size(), fp);
                        pclose(f);
                    });
                    while(!ent) std::this_thread::sleep_for(std::chrono::duration<double>(0.1));
                    closer.detach();
                    fp    = nullptr;
                    begin = GetTime();
                    frames_done = 0;
                }

                pwidth  = bufpixels_width;
                pheight = bufpixels_height;

                buffer.clear();
            }

            if(true)
            {
                auto end = GetTime();
                double d = end - begin;
                unsigned long frames_should = d * fps;

                //bool flag = false;
                for(; frames_done < frames_should; ++frames_done)
                {
                    static unsigned prev = 0;
                    unsigned clock = 10 * (
                        //19*60+30+
                        frames_done / fps);
                    if(clock != prev)
                    {
                        std::fprintf(stderr, "%02d:%04.1f\r", clock/600, (clock%600)*0.1);
                        prev = clock;
                    }

                    if(!fp && buffer.size() >= pixbuf.size()*sizeof(std::uint32_t)*16)
                    {
                        //unsigned swidth  = pwidth  * std::max(1u, (3840+pwidth-1)/pwidth);
                        //unsigned sheight = pheight * std::max(1u, (2160+pheight-1)/pheight);

                        //unsigned swidth  = pwidth  * std::max(1u, (1920+pwidth-1)/pwidth);
                        //unsigned sheight = pheight * std::max(1u, (1080+pheight-1)/pheight);
                        Open(false);
                    }

                    if(fp)
                    {
                        if(!buffer.empty())
                        {
                            SafeWrite(&buffer[0], 1, buffer.size(), fp);
                            buffer.clear();
                        }
                        SafeWrite(&pixbuf[0], sizeof(std::uint32_t), pixbuf.size(), fp);
                    }
                    else
                        buffer.insert(buffer.end(), (char*)&pixbuf[0], (char*)(&pixbuf[0]+pixbuf.size()));
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
            int pollres = poll(p, 1, 1000/(AimedFrameRate*TimeFactor));
            if(pollres < 0) break;
        }
        if((p[0].revents & POLLIN) | (TimeFactor==0.0))
        {
            auto input = tty.Recv();
            if(input.second == -1 && errno == EIO) quit = true;
            auto& str = input.first;
            term.Write(FromUTF8(str));
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
                        ScaleX = 1.0f;
                        ScaleY = 1.0f;
                        VidCellWidth = arg[0];
                        VidCellHeight = arg[1];
                        term.Resize(arg[2], arg[3]);
                        SDL_ReInitialize(wnd.xsize, wnd.ysize);
                        tty.Resize(WindowWidth = wnd.xsize, WindowHeight = wnd.ysize);
                        wnd.Dirtify();
                        //again=true;
                        // Render at least one frame before a new resize
                        expect = 0;
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
                        pending_input.clear(); // Overrides any input events from SDL_KEYDOWN
                        outbuffer += ev.text.text;
                        break;
                    case SDL_KEYDOWN:
                    case SDL_KEYUP:
                    {
                        keys[ev.key.keysym.sym] = (ev.type == SDL_KEYDOWN);
                        if(ev.type == SDL_KEYDOWN)
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
                                    case SDLK_F5: if(VidCellHeight > 6) --VidCellHeight; resized = true; break;
                                    case SDLK_F6: if(VidCellHeight < 32) ++VidCellHeight; resized = true; break;
                                    // Allow widths 6, 8 and 9
                                    case SDLK_F7: if(VidCellWidth > 6) --VidCellWidth; resized = true; break;
                                    case SDLK_F8: if(VidCellWidth < 16) ++VidCellWidth; resized = true; break;
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

        AdvanceTime(1 / AimedFrameRate);
        SDL_ReDraw(wnd);
    }
    tty.Kill(SIGHUP);
    tty.Close();
    SDL_Quit();
}
