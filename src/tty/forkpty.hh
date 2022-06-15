#ifndef bqtForkPTYHH
#define bqtForkPTYHH
/** @file tty/forkpty.hh
 * @brief Physical frontend to the underlying process (shell).
 */

#include <string>
#include <csignal>

/** ForkPTY is a communications end point between the terminal emulator
 * and the underlying operating system.
 * As the name suggests, it is a wrapper over the forkpty() call.
 * It launches a shell in the host system.
 */
class ForkPTY
{
public:
    /** Constructor. Calls Open() with its parameters. @see Open() */
    ForkPTY(std::size_t width, std::size_t height) { Open(width,height); }

    /** Destructor. Terminates the subprocess if it is still active. */
    ~ForkPTY();

    /** Create a subprocess in a virtual terminal with given dimensions.
     * If the opening is successful, the file descriptor is set to non-blocking mode.
     */
    void Open(std::size_t width, std::size_t height);

    /** Terminate the subprocess. */
    void Close();

    /** @returns true if the subprocess was launched successfully. */
    bool Active() const;

    /** @returns the underlying file descriptor. */
    int getfd() const;

    /** Try to send the string as input to the subprocess in non-blocking mode.
     * This function is typically called when the user types something.
     * @param buffer The string to send to the subprocess; typically whatever the user typed.
     * @returns the number of bytes written.
     */
    int Send(std::string_view buffer);

    /** Try to receive output from the subprocess in non-blcoking mode.
     * @returns The string containing raw data that was read.
     * The second member contains the length of the string if successful,
     * or -1 if read was unsuccesful.
     */
    std::pair<std::string,int> Recv();

    /** Send a signal to the subprocess. */
    void Kill(int signal);

    /** Inform the subprocess (or its virtual terminal) that
     * the terminal has been resized to the given dimensions.
     */
    void Resize(unsigned xsize, unsigned ysize);

private:
    int fd, pid;
};

#endif
