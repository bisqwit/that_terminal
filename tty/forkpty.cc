#include <cstdlib>
#include <sys/fcntl.h>
#include <unistd.h>
#include <pty.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

#include "forkpty.hh"

void ForkPTY_Init()
{
}

void ForkPTY::Open(std::size_t width, std::size_t height)
{
    struct winsize ws = {};
    ws.ws_col = width;
    ws.ws_row = height;
    pid = forkpty(&fd, NULL, NULL, &ws);
    if(!pid)
    {
        static char termstr[] = "TERM=xterm";
        putenv(termstr);
        execl(getenv("SHELL"), getenv("SHELL"), "-i", "-l", nullptr); // TODO: check return values
    }
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
