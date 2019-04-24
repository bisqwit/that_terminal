#include <map>
#include <unordered_map>
#include <SDL.h>

#include "terminal.hh"
#include "forkpty.hh"
#include "screen.hh"
#include "ctype.hh"

#include <poll.h>
#include <unistd.h>

unsigned VidCellWidth = 8, VidCellHeight = 12;
unsigned WindowWidth  =129, WindowHeight = 40;
//unsigned WindowWidth  = 80, WindowHeight = 25;
//float ScaleX = 3.f;
//float ScaleY = 3.5f;
float ScaleX = 1.f;
float ScaleY = 1.f;

SDL_Window*   window   = nullptr;

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

        if(!window)
        {
            window = SDL_CreateWindow("terminal",
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

        pixbuf.resize(bufpixels_width*bufpixels_height);

        int w,h;
        SDL_GetWindowSize(window, &w,&h);
        ScaleX = w/(float)pixels_width;
        ScaleY = h/(float)pixels_height;

        fprintf(stderr, "Cells: %ux%u, pix sizes: %ux%u (%u), pixels: %ux%u, buf: %ux%u scale:%gx%g\n",
            cells_horiz,cells_vert,
            cell_width_pixels,cell_height_pixels, VidCellHeight,
            pixels_width,pixels_height,
            bufpixels_width,bufpixels_height,
            ScaleX, ScaleY);
    }
    void SDL_ReDraw(Window& wnd, unsigned timer)
    {
        SDL_Rect rect;
        rect.x=0; rect.w=bufpixels_width;
        rect.y=0; rect.h=0;
        unsigned errors = 0;
        auto RenderFlushLines = [&]()
        {
            if(rect.h)
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
                rect.y += rect.h;
                rect.h = 0;
            }
        };
        auto RenderAddLine = [&](unsigned line)
        {
            if(line > rect.y+rect.h+15 || line < rect.y) RenderFlushLines();
            if(!rect.h) { rect.y = line; rect.h = 1; }
            else rect.h = line+1-rect.y;
        };

        wnd.Render(VidCellWidth,VidCellHeight, &pixbuf[0], timer);
        for(unsigned y=0; y<cells_vert*VidCellHeight; ++y)
            RenderAddLine(y);

        RenderFlushLines();
        if(rect.y) { SDL_RenderPresent(renderer); }
    }
}

int main()
{
    //SDL_Init(SDL_INIT_EVERYTHING);

    Window wnd(WindowWidth, WindowHeight);
    termwindow term(wnd);
    ForkPTY tty(wnd.xsize, wnd.ysize);
    std::string outbuffer;

    SDL_ReInitialize(wnd.xsize, wnd.ysize);
    SDL_StartTextInput();

    std::unordered_map<int, bool> keys;
    bool quit = false;
    unsigned timer = 0;
    while(!quit)
    {
        struct pollfd p[2] = { { tty.getfd(), POLLIN, 0 } };
        if(!term.OutBuffer.empty() || !outbuffer.empty())
        {
            p[0].events |= POLLOUT;
        }
        int pollres = poll(p, 1, 30);
        if(pollres < 0) break;
        if(p[0].revents & POLLIN)
        {
            auto input = tty.Recv();
            auto& str = input.first;
            term.Write(FromUTF8(str));
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
        if(!outbuffer.empty())
        {
            int r = tty.Send(outbuffer);
            if(r > 0)
                outbuffer.erase(0, r);
        }

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
                            int w,h;
                            SDL_GetWindowSize(window, &w,&h);
                            if(w != pixels_width*ScaleX
                            || h != pixels_height*ScaleY)
                            {
                                unsigned newxsize = (w/ScaleX) / VidCellWidth;
                                unsigned newysize = (h/ScaleY) / VidCellHeight;
                                term.Resize(newxsize, newysize);
                                SDL_ReInitialize(wnd.xsize, wnd.ysize);
                                tty.Resize(wnd.xsize, wnd.ysize);
                            }
                            wnd.Dirtify();
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
                    pending_input.clear();
                    tty.Send(ev.text.text);
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
                            { SDLK_8, '8' }, { SDLK_9, '9' }, { SDLK_PERIOD, '.' }, { SDLK_COMMA, '-' },
                            { SDLK_RETURN, '\r' }, { SDLK_BACKSPACE, '\177' }, { SDLK_TAB, '\t' },
                            { SDLK_SPACE, ' ' },
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
                                case SDLK_F7: if(VidCellWidth == 8) VidCellWidth = 6; resized = true; break;
                                case SDLK_F8: if(VidCellWidth == 6) VidCellWidth = 8; resized = true; break;
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
                            tty.Resize(wnd.xsize, wnd.ysize);
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
                                if(shift && cval == '\t') pending_input += "\33[Z";
                                else pending_input += ToUTF8(std::u32string_view(&cval,1));
                            }
                            // Put the input in "pending_input", so that it gets automatically
                            // cancelled if a textinput event is generated.
                        }
                    }
                    else
                    {
                        if(!pending_input.empty())
                        {
                            tty.Send(std::move(pending_input));
                            pending_input.clear();
                        }
                    }
                    break;
                }
            }
        if(!pending_input.empty())
            tty.Send(std::move(pending_input));

        SDL_ReDraw(wnd, timer);
        ++timer;
    }
    tty.Kill(SIGHUP);
    tty.Close();
    SDL_Quit();
}
