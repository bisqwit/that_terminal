#include <map>
#include <unordered_map>
#include <SDL.h>

#include "terminal.hh"
#include "forkpty.hh"
#include "screen.hh"
#include "ctype.hh"

#include <poll.h>
#include <unistd.h>

unsigned VidCellWidth = 8, VidCellHeight = 16;
unsigned WindowWidth  = 80, WindowHeight = 25;

namespace
{
    SDL_Window*   window   = nullptr;
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
        fprintf(stderr, "Cells: %ux%u, pix sizes: %ux%u (%u), pixels: %ux%u, buf: %ux%u\n",
            cells_horiz,cells_vert,
            cell_width_pixels,cell_height_pixels, VidCellHeight,
            pixels_width,pixels_height,
            bufpixels_width,bufpixels_height);

        if(!window)
        {
            window = SDL_CreateWindow("terminal",
                SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                pixels_width*4, pixels_height*4,
                SDL_WINDOW_RESIZABLE);
        }
        else
        {
            SDL_SetWindowSize(window, pixels_width, pixels_height);
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

        wnd.Render(VidCellWidth,VidCellHeight, &pixbuf[0]);
        for(unsigned y=0; y<cells_vert*VidCellHeight; ++y)
            RenderAddLine(y);

        RenderFlushLines();
        if(rect.y) { SDL_RenderPresent(renderer); }
    }
}

int main()
{
    Window wnd(WindowWidth, WindowHeight);
    termwindow term(wnd);
    ForkPTY tty(wnd.xsize, wnd.ysize);
    std::string outbuffer;

    SDL_ReInitialize(wnd.xsize, wnd.ysize);
    SDL_StartTextInput();

    std::unordered_map<int, bool> keys;
    for(;;)
    {
        struct pollfd p[2] = { { tty.getfd(), POLLIN, 0 },
                               { 0, POLLIN, 0 } };
        if(!term.OutBuffer.empty() || !outbuffer.empty())
        {
            p[0].events |= POLLOUT;
        }
        int pollres = poll(p, 2, 30);
        if(pollres < 0) break;
        if(p[0].revents & POLLIN)
        {
            auto input = tty.Recv();
            auto& str = input.first;
            term.Write(FromUTF8(str));
        }
        if(p[1].revents & POLLIN)
        {
            char Buf[4096];
            int r = read(0, Buf, sizeof(Buf));
            if(r > 0)
                tty.Send(std::string_view(Buf, r));
        }
        if(!term.OutBuffer.empty())
        {
            std::u32string str(term.OutBuffer.begin(), term.OutBuffer.end());
            outbuffer += ToUTF8(str);
        }
        if(!outbuffer.empty())
        {
            int r = tty.Send(outbuffer);
            if(r > 0)
                outbuffer.erase(0, r);
        }

        for(SDL_Event ev; SDL_PollEvent(&ev); )
            switch(ev.type)
            {
                case SDL_WINDOWEVENT:
                    switch(ev.window.event)
                    {
                        case SDL_WINDOWEVENT_EXPOSED:
                        case SDL_WINDOWEVENT_RESIZED:
                        case SDL_WINDOWEVENT_SIZE_CHANGED:
                            break;
                        default:
                            break;
                    }
                    break;
                case SDL_QUIT:
                    break;
                case SDL_TEXTINPUT:
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
                        bool shift = keys[SDLK_LSHIFT] || keys[SDLK_RSHIFT];
                        bool alt   = keys[SDLK_LALT]   || keys[SDLK_RALT];
                        bool ctrl  = keys[SDLK_LCTRL]  || keys[SDLK_RCTRL];
                        if(auto i = lore.find(ev.key.keysym.sym); i != lore.end())
                        {
                            const auto& d = i->second;
                            unsigned delta = 1 + shift*1 + alt*2 + ctrl*4, len;
                            char bracket = '[', Buf[16];
                            if(d.second >= 'P' && d.second <= 'S') bracket = 'O';
                            if(delta != 1)
                                len = std::sprintf(Buf, "\33%c%d;%d%c", bracket, d.first, delta, d.second);
                            else if(d.first == 1)
                                len = std::sprintf(Buf, "\33%c%c", bracket, d.second);
                            else
                                len = std::sprintf(Buf, "\33%c%d%c", bracket, d.first, d.second);
                            tty.Send(std::string_view(Buf, len));
                        }
                        else if(ctrl && (ev.key.keysym.sym >= SDLK_a && ev.key.keysym.sym <= SDLK_z))
                        {
                            char c = ev.key.keysym.sym - SDLK_a + 1;
                            tty.Send(std::string_view(&c,1));
                        }
                        else if(ctrl && ev.key.keysym.sym == SDLK_2)
                        {
                            char c = 0;
                            tty.Send(std::string_view(&c,1));
                        }
                        else if(ctrl && ev.key.keysym.sym >= SDLK_3 && ev.key.keysym.sym <= SDLK_9)
                        {
                            char c = ev.key.keysym.sym - SDLK_3 + 27;
                            tty.Send(std::string_view(&c,1));
                        }
                        else switch(ev.key.keysym.sym)
                        {
                            case SDLK_RETURN:
                                tty.Send("\r");
                                break;
                            case SDLK_BACKSPACE:
                                if(ctrl) tty.Send("\10");
                                else     tty.Send("\b");
                                break;
                            case SDLK_TAB:
                                if(shift) tty.Send("\33[[Z");
                                else      tty.Send("\t");
                                break;
                        }
                    }
                    break;
                }
            }

        SDL_ReDraw(wnd);
    }
}
