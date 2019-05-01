# Escape codes supported by *that terminal*

## Non-esc codes:

Note: Tables include also a few items marked “unsupported”.
These are documented for possible future use,
and are not supported by this terminal emulator.

All of these are recognized even in the middle of an escape code.
E.g. `printf("ABC\033[3\n4mDEF");` will print “ABC” on the current line with the current attribute and “DEF” on the next line with blue color (SGR 34).

* `<07>`  Beep
* `<08>`  Backspace: Moves cursor left by one column, no erase. Does nothing if cursor is already in the leftmost column.
* `<09>`  Tab: Moves cursor right to the next tab column.
* `<0D>`  Carriage return: Moves cursor to the leftmost column.
* `<0E>`  Selects G1 character set for GL
* `<0F>`  Selects G0 character set for GL
* `<7F>`  Del (ignored)
* `<18>`, `<1A>`, `<80>`, `<81>`, `<82>`, `<83>`, `<86>`, `<87>`, `<89>`,
`<8A>`, `<8B>`, `<8C>`, `<91>`, `<92>`, `<93>`, `<94>`, `<95>`, `<99>`
Ignored, terminates and cancels any ESC code (including an OSC/DCS string)
* `<0A>`, `<0B>`, `<0C>` ⁴Line feed: Moves cursor down within window; if at bottom
of the window, scrolls window up and inserts a blank line at bottom.
If the cursor is at the bottom of the screen
but the window bottom is somewhere above,
no scrolling or cursor movement happens.

Any other character is printed, unless an escape code is being parsed.
The cursor is moved to the next column after printing.
If the last character was printed to the last column of the screen,
the next character is printed to the first column of the next row
as if a linefeed was performed in the between. An automatic linefeed
does not happen until the new character is printed.

## Esc codes:

* `<1B>`      ESC.
* `<ESC> [`   CSI. An optional number of non-negative integer parameters may follow, separated by either `:` or `;`.
* `<CSI> "`   CSI".
* `<CSI> ?`   CSI?.
* `<ESC> c`   Resets console. Acts as if these commands were performed consecutively:
  * `<ESC> ( B` `<ESC> ) B` `<ESC> * B` `<ESC> + B` `<0F>` Resets G0,G1,G2,G3 to default and selects G0
  * `<CSI> * x` Resets the character attribute change extent
  * `<CSI> m` Sets attributes to default
  * `<CSI> r` Sets window to default (entire screen)
  * `<CSI> H` Puts cursor at top-left corner
  * `<CSI> 2 J` Clears screen
  * `<CSI> ? 5 l` Clears screen-inverse flag
  * `<CSI> ? 25 h` Shows cursor
* `<ESC> 7` , `<CSI> s` Saves current console state (cursor position, character attributes)
* `<ESC> 8` , `<CSI> u` Restores console state (cursor position, character attributes)
* `<ESC> M`   ⁴Reverse linefeed: Moves cursor up within window; if at top of
the window, scrolls window down and inserts a blank line at top.
If the cursor is at the topmost line of the screen,
but the top edge of the window is somewhere below,
no scrolling or cursor movement happens.
* `<ESC> E`   Acts like `<0D>` followed by `<0A>`.
* `<ESC> D`   Acts like `<0A>`. Can be used to produce LF behavior
without CR even when the TTY is automatically translating linefeeds into LF+CR.
* `<ESC> V`, `<CSI"> q` (If param is 1) Start protected writes
* `<ESC> W`, `<CSI"> q` (If param is 2, 0, or missing) End protected writes. Any characters written in “protected” mode
will be immune of erase operations. They can still be overwritten by
printing anything (including spaces) over them.
* `<CSI> ! p` Same as `<ESC> c`, except leaves cursor and screen contents untouched.
* `<CSI> A` ¹⁴Moves the cursor up by the specified number of rows.
* `<CSI> B`, `<CSI> e` ¹⁴Moves the cursor down by the specified number of rows.
* `<CSI> C`, `<CSI> a` ¹Moves the cursor right by the specified number of columns.
* `<CSI> D` ¹Moves the cursor left by the specified number of columns.
* `<CSI> E` Same as `<0D>` followed by `<CSI> B`.
* `<CSI> F` Same as `<0D>` followed by `<CSI> A`.
* `<CSI> G`, `<CSI> <60>` ¹²Absolute horizontal cursor positioning: Sets cursor to the specified column (X coordinate). Counting begins from 1.
* `<CSI> d` ¹²Absolute vertical cursor positioning: Sets cursor to the specified row (Y coordinate). Counting begins from 1.
* `<CSI> H`, `<CSI> f` ¹²Absolute cursor positioning: Row and column, in that order. Counting begins from 1.
* `<CSI> K` ²Clears on the current line depending on parameter.
  * 0 = Erase from cursor to end of line
  * 1 = Erase from start of line to cursor
  * 2 = Erase entire line
* `<CSI> J` ²Combines `<CSI> K` with the following behavior depending on parameter.
  * 0 = Erase from cursor to end of screen
  * 1 = Erase from start of screen to cursor
  * 2 = Erase entire screen
* `<CSI> ? J` (???) Seems to do the same as `<CSI> J`, expect changes all remaining characters to protected. (possibly wrong)
* `<CSI> ? K` (???) Seems to do the same as `<CSI> K`, expect changes all remaining characters to protected. (possibly wrong)
* `<CSI> M` ¹Scrolls the region between top of window and current line (inclusive) up by specified number of lines.
* `<CSI> L` ¹Scrolls the region between current line and end of window (inclusive) down by specified number of lines.
* `<CSI> S` ¹Scrolls the region between top of window and bottom of window (inclusive) up by specified number of lines.
* `<CSI> ^` ¹Scrolls the region between top of window and bottom of window (inclusive) down by specified number of lines.
* `<CSI> T` Interpreted as `<CSI> ^` if there is one nonzero parameter. Ignored otherwise.
* `<CSI> P` ¹²On the current line, scrolls the region between the current column and the right edge of screen left by the specified number of positions.
* `<CSI> @` ¹²On the current line, scrolls the region between the current column and the right edge of screen right by the specified number of positions.
* `<CSI> X` ¹²On the current line and current column, writes the specified number of blanks. The write will not wrap to the next line.
* `<CSI> b` ¹²At the current position, writes the specified number of duplicates of the last printed character. The cursor *is* moved, and the write may wrap to the next line.
* `<CSI> r` Sets the window top and bottom line numbers. Missing parameters are interpreted as the top and bottom of the screen respectively. Counting begins from 1.
* `<CSI> n` Reports depending on the first parameter. Unrecognized values are ignored.
  * Value 5: Responds with `<ESC> [ 0 n`.
  * Value 6: Responds with `<ESC> [ <row> ; <column> R` with the current cursor coordinates. Counting begins from 1.
* `<CSI> = c` Responds with tertiary device attributes. Ignored if nonzero parameters were found.
* `<CSI> > c` Responds with secondary device attributes. Ignored if nonzero parameters were found.
* `<CSI> c`, `<ESC> Z` Responds with primary device attributes. Ignored if nonzero parameters were found.
* `<CSI?> h` ³Set MISC modes.
* `<CSI?> l` ³Unset MISC modes.
* `<CSI?> s` ³Save MISC modes (xterm extension) (unsupported)
* `<CSI?> r` ³Restore MISC modes (xterm extension) (unsupported)
* `<CSI?> $ p` Responds with `<ESC> [ ? <modenumber> ; <value> $ y` with the current value of the MISC mode. Values: 0=unrecognized, 1=set, 2=unset, 3=permanently set, 4=permanently unset
  * Mode 6: Puts cursor to top-left corner of the window. (Both set & clear)
  * Mode 5: Sets or clears screen-wide inverse flag.
  * Mode 25: Shows/hides cursor.
  * Mode 3: Set width to 132 (enable) or 80 (disable) (only if mode 40 is enabled). (unsupported)
  * Mode 40: Enable/disable 80/132 mode.
  * Mode 1000: VT200 mouse mode (unsupported)
  * Mode 1001: VT200 mouse highlight mode (unsupported)
  * Mode 1002: Button events (unsupported)
  * Mode 1003: Any events (unsupported)
  * Mode 1004: Focus events (unsupported)
  * Mode 1005: Mouse support for wider terminals (unsupported)
  * Mode 1006: SGR ext mouse support (unsupported)
  * Mode 1015: URXVT ext mouse support (unsupported)
  * Mode 1007: Enable/disable mousewheel sending up/down key inputs (unsupported)
* `<CSI> h` ³Set ANSI modes.
* `<CSI> l` ³Unset ANSI modes.
* `<CSI> $ p` Responds with `<ESC> [ <modenumber> ; <value> $ y` with the current value of the ANSI mode. Values: 0=unrecognized, 1=set, 2=unset, 3=permanently set, 4=permanently unset
  * Mode 2: Keyboard locked. (unsupported)
  * Mode 4: Insert mode. (unsupported)
  * Mode 12: No local echo. (unsupported)
  * Mode 20: Auto linefeed. (unsupported)
* `<CSI> m` ³SGR attributes. If no parameters are given, a single zero-parameter is implied.
  * 0 = Sets default attributes (clears all modes listed below, and sets default foreground and default background color).
  * 1 = Sets bold.
  * 2 = Sets dim.
  * 3 = Sets italic.
  * 4 = Sets underline.
  * 5 = Sets blink.
  * 6 = Sets rapid blink.
  * 7 = Sets inverse. (swaps foreground and background color while in effect)
  * 8 = Sets conceal. (unsupported)
  * 9 = Sets overstrike.
  * 20 = Sets fraktur. (unsupported)
  * 21 = Sets double underline.
  * 22 = Clears dim and bold.
  * 23 = Clears italic and fraktur.
  * 24 = Clears underline and double underline.
  * 25 = Clears blink and rapid blink.
  * 27 = Clears inverse.
  * 28 = Clears conceal.
  * 29 = Clears overstrike.
  * 39 = Sets default foreground color, and clears underline and double underline attributes.
  * 49 = Sets default background color.
  * 51 = Sets framed. (unsupported)
  * 52 = Sets encircled. (unsupported)
  * 53 = Sets overlined.
  * 54 = Clears framed and encircled.
  * 55 = Clears overlined.
  * 38 2 \<r> \<g> \<b> = Sets RGB24 foreground color.
  * 38 3 \<c> \<m> \<y> = Sets CMY foreground color (interpreted as RGB24 for now).
  * 38 4 \<c> \<m> \<y> \<k> = Sets CMYK foreground color (interpreted as RGB24 for now).
  * 38 5 \<n> = Sets foreground color \<n> using the 256-color lookup table.
  * 48 2 \<r> \<g> \<b> = Sets RGB24 background color.
  * 48 3 \<c> \<m> \<y> = Sets CMY background color (interpreted as RGB24 for now).
  * 48 4 \<c> \<m> \<y> \<k> = Sets CMYK background color (interpreted as RGB24 for now).
  * 48 5 \<n> = Sets background color \<n> using the 256-color lookup table.
  * 30…37 = Sets foreground color \<n−30> using the 256-color lookup table.
  * 40…47 = Sets background color \<n−40> using the 256-color lookup table.
  * 90…97 = Sets foreground color \<n+8−90> using the 256-color lookup table.
  * 100…107 = Sets background color \<n+8−100> using the 256-color lookup table.
* `<CSI> g`    Set tab stops (unsupported)
* `<CSI> q`    Set LED states (unsupported)
* `<CSI> $ r`  Change character attributes in rectangular area (unsupported).
First 4 params define the rectangle (top line,left column,bottom line,right column), the rest are the SGR sequence. Xterm only supports attributes 0/1/4/5/7/8 and 22/24/25/27/28.
* `<CSI> $ t`  Toggle character attributes in rectangular area (unsupported)
* `<CSI> $ v`  Copy rectangular area (unsupported). First 4 params define
the rectangle, next is page number, next two are the target coordinates, and
the final is the target page number. Xterm ignores the page numbers.
* `<CSI> $ z`  Fill rectangular area with space (unsupported). First 4 params define the rectangle. Attributes are not touched.
* `<CSI> $ {`  Fill rectangular area with space *selectively* (unsupported). Same as `<CSI> $ z` except leaves protected characters untouched.
* `<CSI> $ x`  Fill rectangular area with given character and current attribute (unsupported). First parameter is the character number code, next four define the rectangle.
* `<CSI> $ |`  Set screen width to the given value (only widths 80 or 132 allowed; default=80) (unsupported)
* `<CSI> * |`  Set screen height to the given value (valid range between 1 and 255, inclusive) (unsupported)
* `<CSI> * x`  Set character attribute change extent (unsupported). Value 2 means that attribute-changing rectangular operations will perform rectangularly. Any other value (mainly 0 or 1) means that attribute-changing rectangular operations will perform on a text stream delimited by the first and last character position indicated. This flag will not affect the other rectangular operations, such as fills.
* `<CSI> * y`  Request checksum of rectangular area (unsupported)
* `<ESC> # 3`   Change current line to be rendered using top half of double-height letters.
* `<ESC> # 4`   Change current line to be rendered using bottom half of double-height letters.
* `<ESC> # 5`   Change current line to be rendered using single-width (regular) letters.
* `<ESC> # 6`   Change current line to be rendered using double-width letters. This halves the
rendered row length, but logically the row is still same length as every
other row. The second half of the line is simply not displayed at all.
* `<ESC> # 8`   ²Clears screen with the letter “E” using the current attribute. This is
intended for testing the double-width / double-height character modes. Cursor is moved to the top-left corner of the screen.
* `<ESC> % @`   Unsets UTF8 mode. (unsupported)
* `<ESC> % G`, `<ESC> % 8`   Sets UTF8 mode. (unsupported)
* `<ESC> ( <char>`    Set G0 to character set <char>.
* `<ESC> ) <char>`    Set G1 to character set <char>.
* `<ESC> * <char>`    Set G2 to character set <char>.
* `<ESC> + <char>`    Set G3 to character set <char>.
* `<ESC> - <char>`    Set G1 to type 2 character set <char>.
* `<ESC> . <char>`    Set G2 to type 2 character set <char>.
* `<ESC> / <char>`    Set G3 to type 2 character set <char>.
* `<ESC> <7C>` Select G3 character set for GR (unsupported)
* `<ESC> <7D>` Select G2 character set for GR (unsupported)
* `<ESC> <7E>` Select G1 character set for GR (unsupported)
* `<ESC> N <char>`, `<8E> <char>`  Temporarily selects G2, and prints `<char>` (unsupported)
* `<ESC> O <char>`, `<8F> <char>`  Temporarily selects G3, and prints `<char>` (unsupported)
* `<9C>`, `<ESC> <5C>`, `<07>` ST.
* `<ESC> ^ <string> <ST>`, `<9E> <string> <ST>` PM, Privacy Message (unsupported) (ignored by Xterm; Screen uses this to display messages in status line. Screen also supports `ESC ! <string> <ST>` as an alias.)
* `<ESC> _ <string> <ST>`, `<9F> <string> <ST>` APC, Application Program Command (unsupported) (ignored by Xterm; Screen uses this for changing the window title)
* `<ESC> X <string> <ST>`, `<98> <string> <ST>` SOS, Start of String (unsupported) (ignored by Xterm)
* `<ESC> P <string> <ST>`, `<90> <string> <ST>` DCS, Device Control String
  * Depending on the value of `<string>`:
    * `$ q r` Responds with `<ESC> P 1 $ r <top> ; <bottom> q <ST>` where `<top>` and `<bottom>` are the scrolling region boundaries
    * `$ q m` Responds with `<ESC> P 1 $ r <sgr> q <ST>` where `<sgr>` expresses the SGR sequence representing the current foreground and background colors
    * `$ q t` Responds with `<ESC> P 1 $ r <n> t <ST>` where `<n>` is the screen height in rows minus 1, or 24, whichever is greater
    * `$ q <20> q` Responds with `<ESC> P 1 $ <c> <20> q <ST>` where `<c>` is 1 for a blocky cursor, 3 for an underline cursor, and 5 for a bar cursor, +1 if the cursor is not blinking
    * `$ q " q` Responds with `<ESC> P 1 $ r <p> " q <ST>` where `<p>` indicates whether protected mode is active (see `<ESC> V`)
    * `$ q " p` Responds with `<ESC> P 1 $ r <l> ; <p> " q <ST>` where `<l>` is vtXX_level + 60, `<p>` indicates whether 8-bit controls are *disabled* (see `<CSI"> p`)
    * `$ q $ |` Responds with `<ESC> P 1 $ r <n> $ | <ST>` where `<n>` is the screen width in columns (either 80 or 132). Not the actual width, but the state of the `<CSI?> h` mode 3.
    * `$ q * |` Responds with `<ESC> P 1 $ r <n> * | <ST>` where `<n>` is the screen height in rows
    * `$ q <...>` Responds with `<ESC> P 0 <ST>`
    * `$ <...>` Responds with `<18>`
    * `<params> q <string>` Sixel graphics. (unsupported)
      * Parameters:
        * Parameter 0 (libsixel): Sets pad: 9 = 1; 0/1/7/8/≥10 = 2; 5/6 = 3; 3/4 = 4; 2 = 5. Default: pad=1, pan=2.
        * Parameter 0 (xterm): Sets pan=1, and pad: 7/8/9 = 1; 1/5/6 = 2; 3/4 = 3; 2 = 5
        * Parameter 1 (xterm only): Sets background pixels transparent (1) or 0 (0).
        * Parameter 2 (libsixel only): Multiplies both pan and pad by (param/10), but sets them always to at least 1.
        * Parameter 3—7 (xterm only): pad, pan, ph, pv (as in `<34>` below).
      * String contents:
        * `<22> <pad> ; <pan> ; <ph> ; <pv>` Change raster attributes. Zero pad and pan are treated as 1. Zero ph and pv are ignored.
        * `<23> <color>` Selects color to be used for drawing the 1-bits
        * `<23> <color> ; 1 ; <h> ; <l> ; <s>` Assigns palette color (HLS, 0—360 for hue, 0—100 l & s)
        * `<23> <color> ; 2 ; <r> ; <g> ; <b>` Assigns palette color (RGB, 0—100 range)
        * `<24>` x ← 0              (Carriage return)
        * `<2D>` x ← 0, y ← y + 6   (Carriage return and line feed)
        * `<3F>`…`<7E>` SIXEL. Subtract `<3F>` from the byte, and you get 6
        vertical 1-bit pixels. 0 = don’t draw, 1 = draw using selected color.
        Increment x coordinate after the Sixel.
        * `<21> <count> <SIXEL>` A form of run length encoding.
        Draws the Sixel `<count>` times using the algorithm described above.
        * `<20>`, `<0D>`, `<0A>` ignored
    * `<params> p <string>` ReGIS graphics (unsupported)
      * Parameter 0 is the mode (defaults to 0). Mode 0 was the default and picked up
      drawing where it left off, 1 reset the system to a blank slate, and 2 and 3 were
      the same as 0 and 1, but left a single line of text at the bottom of the screen
      for entering commands.
* `<ESC> ] <string> <ST>`, `<9D> <string> <ST>` OSC, Operating System Call
  * Depending on the value of `<string>`:
    * `0 ; <label>`, `L <label>` Changes icon name and window title to `<label>` (partially supported)
    * `1 ; <label>`, `l <label>` Changes icon name to `<label>` (unsupported)
    * `2 ; <label>`, `I <label>` Changes window title to `<label>`
    * `4 ; <integer> ; <color>` Changes color number `<integer>` to `<color>`, which is a string that is parsed by `XParseColor`. If the color is `?`, the terminal reports the current color instead.
    * `5 ; <integer> ; <color>` Same as above, except 256 is added to `<integer>` first.
    * `6` followed by any number of `; <set> ; <value>` Set depending on `<set>` off(0) or on(1):
      * 0 colorBDmode (unsupported)
      * 1 colorULmode (unsupported)
      * 2 colorBLmode (unsupported)
      * 3 colorRVmode (unsupported)
      * 4 colorITmode (unsupported)
    * `10 ; <color>` Changes the text foreground color to `<color>`.
    * `11 ; <color>` Changes the text background color to `<color>`.
    * `12 ; <color>` Changes the text *cursor* color to `<color>`.
    * `13 ; <color>` Changes the mouse foreground color (interior of the T-cursor) to `<color>`. (unsupported)
    * `14 ; <color>` Changes the mouse background color (edges of the T-cursor) to `<color>`. (unsupported)
    * `17 ; <color>` Changes the mouse select-text background color to `<color>`. (unsupported)
    * `50 ; <label>` Changes font to `<label>` (unsupported)
    * 100+n = reset color setting *n* to default
* `<CSI"> p` If param1 >= 62, param2=1 disables 8-bit controls (default) and value 0 or 2 enables them. (unsupported)

Blank = space character with current attributes.  
¹ = Missing or zero parameter is interpreted as 1.  
² = Ignores window boundaries.  
³ = Any number of parameters can be specified; all of them will be processed.  
⁴ = Does not allow the cursor to escape the window; however, if the cursor
  is already outside the window, allows moving to another row
  even if the target row is even farther outside the window.

No cursor movement command allows the cursor to leave the *screen*.
However, as an exception, after printing,
the cursor is allowed to temporarily sit
on the rightmost edge of the screen,
outside the visible area.
Printing anything there will invoke an automatic linefeed.
For the purposes of cursor movement commands,
the cursor is still inside the visible area.

None of the commands move the cursor unless explicitly specified.
This is important to note especially with the scrolling,
clearing/erasing, and writing commands.

When any invalid escape code is encountered,
the escape code until the invalid character is silently ignored,
and the next character is processed as if no escape code was actived.

None of these escape codes are custom-made for this terminal emulator.
All of these are supported by e.g. Xterm (although Xterm does not render italic).

## Character sets

  * `<` DEC supplementary (unsupported)
  * `=` Swiss (unsupported)
  * `>` DEC Technical (unsupported)
  * `0` DEC graphics (VT100 line-drawing)
  * `1` DEC alt characters (unsupported)
  * `2` DEC alt graphics (unsupported)
  * `<60>` Norwegian/Danish (unsupported)
  * `4` Dutch (unsupported)
  * `5` Finnish (unsupported)
  * `6` Norwegian/Danish3 (unsupported)
  * `7` Swedish (unsupported)
  * `9` French/Canadian2 (unsupported)
  * `A` British (unsupported) (in xterm, chooses U+0080..U+00FF)
  * `B` ASCII (default character set) (unsupported)
  * `C` Finnish2 (unsupported)
  * `E` Norwegian/Danish2 (unsupported)
  * `f` French2 (unsupported)
  * `H` Swedish2 or ISO Hebrew Supplemental (unsupported)
  * `K` Linux user map / German (unsupported)
  * `Q` French/Canadian (unsupported)
  * `R` French (unsupported)
  * `U` Linux IBM PC map (unsupported)
  * `Y` Italian (unsupported)
  * `Z` Spanish (unsupported)

Type 2 character sets:

  * `A`, `H` (same meanings as above)
  * `F` ISO Greek Supplemental (unsupported)
  * `L` ISO Latin Cyrillic (unsupported)
  * `M` ISO Latin5 Supplemental (unsupported)
