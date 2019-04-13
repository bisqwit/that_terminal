# That terminal.

It’s the terminal that was featured in a programming livestream series
on Bisqwit’s YouTube channel, at:
https://www.youtube.com/watch?v=9e38ORrkbtc&list=PLzLzYGEbdY5nKCfUgfk8tCO9veao_P1fV

## What is a terminal emulator?

A terminal emulator is a program that emulates a *terminal*.

**A terminal** is a device or a program that connects two things:

1. user-interface (typically keyboard and screen)
1. a system (typically a local or remote [shell](https://en.wikipedia.org/wiki/Command-line_interface),
or a [BBS](https://en.wikipedia.org/wiki/Bulletin_board_system)) that reads input and writes output
([standard input and standard output](https://en.wikipedia.org/wiki/Standard_streams) respectively).

This communication is entirely text-based, through protocols like VT100.
Many people know of *ANSI escape codes*. That’s another version of that
protocol.

A terminal emulator converts keyboard input (including arrow keys)
into a character stream, and reads character stream from the application
and converts that into symbols, cursor movements, and colors, on the screen.

### What is terminal emulator *not*?

A terminal emulator is *not a shell nor a command interpreter*.
It does not *understand* anything you type. It just converts those keys
into a character stream and passes them to another program that hopefully
does understand them.

## What is *this* terminal emulator used for?

It is a tool I use for creating videos.
It is supposed to replace the DOSBox-based toolchain I have used until now.
The [input daemon](https://bisqwit.iki.fi/source/inputter.html) is similarly being
replaced, as is the tool that I use for generating the input in the first
place.

It also seeks to make [that editor](https://github.com/bisqwit/that_editor)
obsolete, replacing it with [Joe](https://joe-editor.sourceforge.io).

### Why do you need a custom terminal? Why not Xterm?

* I need something that I can customize for artistic reasons, even if that breaks compatibility with other programs.
* I also need to be able to change fonts and terminal sizes at whim with simple keyboard inputs (mouse input is too precise and difficult to script).
* And I need to record video while doing that. This is a video production tool after all.

## Author

*That terminal* is written by Joel Yliluoma.
You can find my homepage at: https://iki.fi/bisqwit/
And you can contact me through e-mail: bisqwit@iki.fi


