#include <string>
#include <signal.h>

class ForkPTY
{
public:
    ForkPTY(std::size_t width, std::size_t height) { Open(width,height); }
    ~ForkPTY() { if(Active()) Close(); }
    void Open(std::size_t width, std::size_t height);
    void Close();
    inline bool Active() const { return fd >= 0; }
    int Send(std::string_view buffer);
    std::pair<std::string,int> Recv();
    int getfd() const { return fd; }
    void Kill(int signal);
    void Resize(unsigned xsize, unsigned ysize);
private:
    int fd, pid;
};
