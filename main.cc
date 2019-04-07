#include <SDL.h>

#include "terminal.hh"
#include "forkpty.hh"
#include "screen.hh"
#include "ctype.hh"

#include <poll.h>
#include <unistd.h>

unsigned VidCellWidth = 8, VidCellHeight = 8;
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
                pixels_width, pixels_height,
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
            }

        SDL_ReDraw(wnd);
    }
}
