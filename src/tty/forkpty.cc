#ifdef RUN_TESTS
#include <gtest/gtest.h>
#include <chrono>
#endif
/** @file tty/forkpty.cc
 * @brief Physical frontend to the underlying process (shell).
 */

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

bool ForkPTY::Active() const
{
    return fd >= 0;
}

int ForkPTY::getfd() const
{
    return fd;
}

ForkPTY::~ForkPTY()
{
    if(Active())
    {
        Close();
    }
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
    //char buffer[16]; usleep(400);
    char buffer[512];
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

#ifdef RUN_TESTS
TEST(forkpty, opening_a_shell_works) // Test that opening a shell works
{
    ForkPTY_Init();

    ForkPTY pty(80,25);
    pty.Active();
    EXPECT_NE(pty.getfd(), -1);
    pty.Close();
    pty.Active();
}
TEST(forkpty, killing_a_shell_works) // Test that killing a shell works
{
    ForkPTY pty(80,25);
    // Disable non-blocking mode
    fcntl(pty.getfd(), F_SETFL, fcntl(pty.getfd(), F_GETFL) & ~O_NONBLOCK);

    EXPECT_NE(pty.Recv().second, -1);
    pty.Kill(SIGHUP);
    pty.Active();
    EXPECT_EQ(pty.Recv().second, -1);
    pty.Close();
    pty.Active();
}
// Test that running commands in shell works
// This test depends on the command `stat` existing.
TEST(forkpty, running_a_command_in_shell_works)
{
    ForkPTY pty(80,25);
    auto t = []{return std::chrono::system_clock::now(); };
    // Wait until we receive something
    auto start = t();
    pty.Send("\r");
    std::string in;
    while(in.empty())
    {
        auto k = pty.Recv();
        in += k.first;
        EXPECT_LE(std::chrono::duration<double>(t()-start).count(), 2);
        usleep(10000);
    }
    // Send a command
    pty.Send("stat /tmp\r");
    // Wait until we receive something
    for(;;)
    {
        auto k = pty.Recv();
        in += k.first;
        EXPECT_LE(std::chrono::duration<double>(t()-start).count(), 2);
        usleep(10000);
        if(in.find("Device:") != in.npos) break;
    }
    pty.Close();
}
// Test that the shell reacts to resizes.
TEST(forkpty, resizing_works)
{
    ForkPTY pty(80,25);
    auto t = []{return std::chrono::system_clock::now(); };
    // Wait until we receive something
    auto start = t();
    pty.Send("\r");
    pty.Active();
    pty.Resize(40,30);
    std::string in;
    while(in.empty())
    {
        auto k = pty.Recv();
        in += k.first;
        ASSERT_LE(std::chrono::duration<double>(t()-start).count(), 2);
        usleep(10000);
    }
    // Send a command
    pty.Send("echo $LINES $COLUMNS\r");
    // Wait until we receive something
    for(;;)
    {
        auto k = pty.Recv();
        in += k.first;
        ASSERT_LE(std::chrono::duration<double>(t()-start).count(), 2);
        usleep(10000);
        if(in.find("30 40") != in.npos) break;
    }
    pty.Close();
    pty.Active();
}
#endif
