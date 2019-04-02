#include <SDL.h>

#include "terminal.hh"
#include "forkpty.hh"
#include "screen.hh"
#include "ctype.hh"

#include <poll.h>
#include <unistd.h>

namespace
{
    SDL_Window*   window   = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture*  texture  = nullptr;
    std::vector<std::uint32_t> pixbuf;
    std::size_t texturewidth=0, textureheight=0;

    void InitGFX(std::size_t pixels_width, std::size_t pixels_height)
    {
        std::size_t bufpixels_width  = pixels_width;
        std::size_t bufpixels_height = pixels_height;
        if(!window)
        {
            window = SDL_CreateWindow("editor",
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
}

int main()
{
    Window wnd(80, 25);
    termwindow term(wnd);
    ForkPTY tty(wnd.xsize, wnd.ysize);
    std::string outbuffer;

    InitGFX(80*8, 25*16);

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
        wnd.Render(8,16, &pixbuf[0]);

        SDL_UpdateTexture(texture, nullptr,
                          pixbuf.data(),
                          80*8*sizeof(pixbuf[0]));
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }
}
