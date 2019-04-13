# Escape codes supported by *that terminal*

## Non-esc codes:

All of these are recognized even in the middle of an escape code.
E.g. `printf("ABC\033[3\n4mDEF");` will print “ABC” on the current line with the current attribute and “DEF” on the next line with blue color (SGR 34).

* `<07>`  Beep (currently ignored)
* `<08>`  Backspace (move cursor left by one column, no erase)
* `<09>`  Tab (moves cursor right to the next tab column)
* `<0D>`  Carriage return: Moves cursor to column 1
* `<0E>`  Select G1 character set for GL
* `<0F>`  Select G0 character set for GL
* `<7F>`  Del (ignored)
* `<18>`  Ignored, terminates and cancels an ESC code
* `<1A>`  Ignored, terminates and cancels an ESC code
* `<0A>`, `<0B>`, `<0C>` ⁴Line feed: Moves cursor down within window; if at bottom
of the window, scrolls window up and inserts a blank line at bottom.
If the cursor is at the bottom of the screen
but the window bottom is somewhere above,
no scrolling or cursor movement happens.

## Esc codes:

Note: Table includes also a few items marked “unsupported”.
These are documented for possible future use,
and are not supported by this terminal emulator.

* `<1B>`      ESC.
* `<ESC> [`   CSI. An optional number of integer parameters may follow, separated by either `:` or `;`.
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
* `<ESC> c`   Resets console. Acts as if these commands were performed consecutively:
  * `<SCS0> B` `<SCS1> B <11>` Resets G0 and G1 to default and selects G0
  * `<CSI> m` Sets attributes to default
  * `<CSI> r` Sets window to default (entire screen)
  * `<CSI> H` Puts cursor at top-left corner
  * `<CSI> 2 J` Clears screen
  * `<CSI> ? 5 l` Clears screen-reverse flag
  * `<CSI> ? 25 h` Shows cursor
* `<ESC> 7` , `<CSI> s` Saves current console state (cursor position, window boundaries, character attributes)
* `<ESC> 8` , `<CSI> u` Restores console state (cursor position, window boundaries, character attributes)
* `<ESC> M`   ⁴Reverse linefeed: Moves cursor up within window; if at top of
the window, scrolls window down and inserts a blank line at top.
If the cursor is at the topmost line of the screen,
but the top edge of the window is somewhere below,
no scrolling or cursor movement happens.
* `<ESC> E`   Acts like `<0D>` followed by `<0A>`.
* `<ESC> # 3`   Change current line to be rendered using top half of double-height letters.
* `<ESC> # 4`   Change current line to be rendered using bottom half of double-height letters.
* `<ESC> # 5`   Change current line to be rendered using single-width (regular) letters.
* `<ESC> # 6`   Change current line to be rendered using double-width letters. This halves the line length.
* `<ESC> # 8`   ²Clears screen with the letter “E” using the current attribute. This is for testing the double-width / double-height character modes. Cursor is moved to the top-left corner of the screen.
* `<ESC> % @`   Unsets UTF8 mode. (unsupported)
* `<ESC> % G`, `<ESC> % 8`   Sets UTF8 mode. (unsupported)
* `<CSI> g`    Set tab stops (unsupported)
* `<CSI> q`    Set LED states (unsupported)
* `<CSI> A` ¹⁴Move the cursor up by the specified number of rows.
* `<CSI> B`, `<CSI> e` ¹⁴Move the cursor down by the specified number of rows.
* `<CSI> C`, `<CSI> a` ¹Move the cursor right by the specified number of columns.
* `<CSI> D` ¹Move the cursor left by the specified number of columns.
* `<CSI> E` Same as `<0D>` followed by `<CSI B>`.
* `<CSI> F` Same as `<0D>` followed by `<CSI A>`.
* `<CSI> G`, `<CSI> <60>` ¹²Absolute horizontal cursor positioning: Set cursor to specified column (X coordinate).
* `<CSI> d` ¹²Absolute vertical cursor positioning: Set cursor to specified row (Y coordinate).
* `<CSI> H`, `<CSI> f` ¹²Absolute cursor positioning: Row and column, in that order.
* `<CSI> K` ²Clears on the current line depending on parameter.
  * 0 = Erase from cursor to end of line
  * 1 = Erase from start of line to cursor
  * 2 = Erase entire line
* `CSI> J` ²Combines `<CSI> K` with the following behavior depending on parameter.
  * 0 = Erase from cursor to end of screen
  * 1 = Erase from start of screen to cursor
  * 2 = Erase entire screen
* `<CSI> M` ¹Scrolls the region between top of window and current line (inclusive) up by specified number of lines.
* `<CSI> L` ¹Scrolls the region between current line and end of window (inclusive) down by specified number of lines.
* `<CSI> S` ¹Scrolls the region between top of window and bottom of window (inclusive) up by specified number of lines.
* `<CSI> ^` ¹Scrolls the region between top of window and bottom of window (inclusive) down by specified number of lines.
* `<CSI> T` Interpreted as `<CSI> ^` if there is one nonzero parameter. Ignored otherwise.
* `<CSI> P` ¹²Writes (delete) the specified number of black holes at the cursor. The rest of the line is moved towards the left.
* `<CSI> X` ¹²Writes (overwrite) the specified number of blanks at the cursor.
* `<CSI> @` ¹²Writes (insert) the specified number of blanks at the cursor, moving the rest of the line towards the right.
* `<CSI> r`, `<CSI> ! p` Sets the window top and bottom line numbers. Missing parameters are interpreted as the top and bottom of the screen respectively.
* `<CSI> n` Reports depending on the first parameter. Unrecognized values are ignored.
  * Value 5: Reports `<CSI> 0 n`.
  * Value 6: Reports `<CSI> <x> ; <y> R` with the current cursor coordinates.
* `<CSI> = c` Reports tertiary device attributes. Ignored if nonzero parameters were found.
* `<CSI> > c` Reports secondary device attributes. Ignored if nonzero parameters were found.
* `<CSI> c`, `<ESC> Z` Reports primary device attributes. Ignored if nonzero parameters were found.
* `<CSI> ? h` ³Set MISC modes.
* `<CSI> ? l` ³Unset MISC modes.
  * Mode 6: Puts cursor to top-left corner of the window. (Both set & clear)
  * Mode 5: Sets or clears screen-wide reverse flag.
  * Mode 25: Shows/hides cursor.
* `<CSI> h` ³Set ANSI modes.
* `<CSI> l` ³Unset ANSI modes.
  * Mode 2: Keyboard locked. (unsupported)
  * Mode 4: Insert mode. (unsupported)
  * Mode 12: Local echo. (unsupported)
  * Mode 20: Auto linefeed. (unsupported)
* `<CSI> m` ³SGR attributes. If no parameters are given, a single zero-parameter is implied.
  * 0 = Sets default attributes (clears all modes listed below, and sets default foreground and default background color).
  * 1 = Sets bold.
  * 2 = Sets dim.
  * 3 = Sets italic.
  * 4 = Sets underline.
  * 5 = Sets blink.
  * 7 = Sets reverse.
  * 8 = Sets conceal. (unsupported)
  * 9 = Sets overstrike.
  * 20 = Sets fraktur. (unsupported)
  * 21 = Sets double underline.
  * 22 = Clears dim and bold.
  * 23 = Clears italic and fraktur.
  * 24 = Clears underline and double underline.
  * 25 = Clears blink.
  * 27 = Clears reverse.
  * 28 = Clears concecal.
  * 29 = Clears overstrike.
  * 39 = Clears underline and double underline and resets foreground color.
  * 49 = Resets background color.
  * 51 = Sets framed. (unsupported)
  * 52 = Sets encircled. (unsupported)
  * 53 = Sets overlined. (unsupported)
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

## Character sets

  * `<` DEC supplementary (unsupported)
  * `=` Swiss (unsupported)
  * `>` DEC Technical (unsupported)
  * `0` DEC graphics (VT100 line-drawing) (unsupported)
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
