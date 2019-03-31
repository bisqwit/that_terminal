#include <SDL.h>

#include "terminal.hh"
#include "forkpty.hh"
#include "screen.hh"
#include "ctype.hh"

#include <poll.h>


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
            texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32,
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
    for(;;)
    {
        struct pollfd p = { tty.getfd(), POLLIN, 0 };
        if(!term.OutBuffer.empty() || !outbuffer.empty())
        {
            p.events |= POLLOUT;
        }
        int pollres = poll(&p, 1, -1);
        if(pollres < 0) break;
        if(p.revents & POLLIN)
        {
            auto input = tty.Recv();
            term.Write(FromUTF8(input.first));
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
    }
}
