# That terminal.

It’s the terminal that was featured in a programming livestream series
on Bisqwit’s YouTube channel, at:
https://www.youtube.com/watch?v=9e38ORrkbtc&list=PLzLzYGEbdY5nKCfUgfk8tCO9veao_P1fV

![Screenshot showcasing the character set support](doc/snap.png)

## Features

Supported:

* Dozens of built-in fonts (covering the entire basic multilingual plane of Unicode and then some)
* Single-width and double-width characters (such as CJK + English text)
* Video recording at any frame rate, including at rates faster than your OS can handle
* Automated input
* Resizing of terminal dimensions / font size at any time
* Way more terminal escape codes than a typical user will ever depend on

Not supported (yet!)

* Copy and paste
* User-supplied fonts
* Configuration of key features without recompiling source code
* Bidirectional rendering (such as Arabic and Hebrew)
* Combining diacritics
* Letters that change shape depending on position in word (such as word-final glyphs in Hebrew)

This video is designed for programming video production purposes.

## What is a terminal emulator?

A terminal emulator is a program that emulates a *terminal*.

**A terminal** is a device or a program that connects two things:

1. user-interface (typically keyboard and screen)
1. a system (typically a local or remote [shell](https://en.wikipedia.org/wiki/Command-line_interface),
or a [BBS](https://en.wikipedia.org/wiki/Bulletin_board_system)) that reads input and writes output
([standard input and standard output](https://en.wikipedia.org/wiki/Standard_streams) respectively).

This communication is entirely text-based,
and includes
[in-band signaling](https://en.wikipedia.org/wiki/In-band_signaling)
like the *[ANSI escape codes](https://en.wikipedia.org/wiki/ANSI_escape_code)*.

A terminal emulator converts keyboard input (including arrow keys)
into a character stream, and reads character stream from the application
and converts that into symbols, cursor movements, and colors, on the screen.

### What is terminal emulator *not*?

A terminal emulator is *not a shell nor a command interpreter*.
It does not *understand* anything you type. It just converts those keys
into a character stream and passes them to another program that hopefully
does understand them.

### Why is it called an “emulator”?

Back in the early ages of computing, computers were extremely expensive.
The typical architecture was such that you had one central computer,
and a number of [*dumb terminals*](https://en.wikipedia.org/wiki/Computer_terminal)
(and later, smart terminals).

These terminals were devices
that… «drum roll»… translated between the user-interface
and the input & output of the central computer.
They had no computing capabilities of their own.

Over time as computers became cheaper, it became possible to run
these text-based systems locally. The local computer then ran
a terminal *emulator* to interface with these local systems,
that still communicated using text streams.

The emulator *emulates* the terminal such as the
[VT100](https://en.wikipedia.org/wiki/VT100).
The VT100 was a remarkable milestone in the
history of glass screen based terminal emulators,
started by the
[Datapoint 3300](https://en.wikipedia.org/wiki/Datapoint_3300)
and the
[DEC VT05](https://en.wikipedia.org/wiki/VT05).
These devices were a giant’s leap of improvement
over using a *line printer* for the display device.

Even in today’s time, many decades later, the system of combining
a text-based stream and a terminal emulator remains a useful and efficient
paradigm, and really, the *de-facto* way to maintain UNIX-style systems.
Some people, such as yours truly, even use it as their *primary* user-interface
for most tasks on the computer.

Even the Microsoft® Windows® Command Prompt utilizes a terminal emulator
([Win32 Console](https://en.wikipedia.org/wiki/Win32_console)),
even though the division between console programs and the terminal emulator
is not as obvious in Windows
as it is in UNIX-style systems like Linux.

Modern terminal emulators not only emulate historic terminals
like the VT100, VT220, VT240, VT340, VT420, and VT525,
but also support other [ANSI escape codes](https://en.wikipedia.org/wiki/ANSI_escape_code)
that have been created specifically for terminal emulators,
such as the 256-color and 16777216-color SGR parameters,
which are supported by my program as well.

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
* I also need to be able to change fonts and terminal sizes at whim with simple keyboard inputs (mouse input is too delicate to script).
* And I need to record video while doing that. This is a video production tool after all.

### Supported escape codes

You can find the list of escapes supported by this emulator
in [a separate document](doc/escapes.md).

### Supported fonts

You can find the list of fonts supported by this emulator
in [a separate document](doc/fonts.md).

This terminal emulator fully supports single-width
and double-width characters (such as CJK).
The best font sizes to try that are 4x8, 6x9, 8x16, and 9x18,
because they match perfectly with the CJK fonts at 8x8, 12x9, 16x16 and 18x18
at integer scaling.
The 8x16 (+16x16) font has best coverage for Unicode, thanks to Unifont.

### Supported keyboard inputs

In [a separate document](doc/inputs.md).

## Author

*That terminal* is written by Joel Yliluoma.
You can find my homepage at: https://iki.fi/bisqwit/
And you can contact me through e-mail: bisqwit@iki.fi


