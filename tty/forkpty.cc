#include <cstdlib>
#include <cerrno>

#include <unistd.h>
#include <sys/wait.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>

#ifdef __APPLE__
# include <util.h>
#else
# include <pty.h>
#endif

#include "forkpty.hh"

void ForkPTY_Init()
{
}

void ForkPTY::Open(std::size_t width, std::size_t height)
{
    struct winsize ws = {};
    ws.ws_col = width;
    ws.ws_row = height;
    pid = forkpty(&fd, nullptr, nullptr, &ws);
    if(!pid)
    {
        static char termstr[] = "TERM=xterm";
        putenv(termstr);
        execl(std::getenv("SHELL"), std::getenv("SHELL"), "-l", "-i", nullptr); // TODO: check return values
        // Note: getenv() is in C++ standard, but putenv() is not.
    }

    // Change the virtual terminal handle (file descriptor)
    // into non-blocking mode.
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}
void ForkPTY::Close()
{
    kill(pid, SIGTERM);
    close(fd);
    waitpid(pid, nullptr, 0);
}
int ForkPTY::Send(std::string_view buffer)
{
    return write(fd, buffer.data(), buffer.size());
}
std::pair<std::string,int> ForkPTY::Recv()
{
    char buffer[4096];
    std::pair<std::string,int> result;
    result.second = read(fd, buffer, sizeof(buffer));
    if(result.second > 0)
        result.first.assign(buffer, buffer+result.second);
    return result;
}
void ForkPTY::Kill(int signal)
{
    kill(pid, signal);
}
void ForkPTY::Resize(unsigned xsize, unsigned ysize)
{
    struct winsize ws = {};
    ws.ws_col = xsize;
    ws.ws_row = ysize;
    ioctl(fd, TIOCSWINSZ, &ws);
}
