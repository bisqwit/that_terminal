#include <string>
#include <csignal>

class ForkPTY
{
public:
    ForkPTY(std::size_t width, std::size_t height) { Open(width,height); }
    ~ForkPTY() { if(Active()) Close(); }

    // Create a subprocess in a virtual terminal with given dimensions.
    void Open(std::size_t width, std::size_t height);
    // Terminate the subprocess.
    void Close();

    // Return true if the subprocess was launched successfully.
    inline bool Active() const { return fd >= 0; }
    int getfd() const { return fd; }

    // Try to send the string as input to the subprocess.
    // Returns the number of bytes written.
    int Send(std::string_view buffer);

    // Try to receive output from the subprocess.
    std::pair<std::string,int> Recv();

    // Send a signal to the subprocess.
    void Kill(int signal);

    // Inform the subprocess (or its virtual terminal) that
    // the terminal has been resized to the given dimensions.
    void Resize(unsigned xsize, unsigned ysize);

private:
    int fd, pid;
};
