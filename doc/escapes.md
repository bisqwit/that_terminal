# Escape codes supported by *that terminal*

## Non-esc codes:

All of these are recognized even in the middle of an escape code.
E.g. `printf("ABC\033[3\n4mDEF");` will print “ABC” on the current line with the current attribute and “DEF” on the next line with blue color (SGR 34).

* `<07>`  Beep (currently ignored)
* `<08>`  Backspace (move cursor left by one column, no erase)
* `<09>`  Tab (moves cursor right to the next tab column)
* `<0D>`  Carriage return: Moves cursor to column 1
* `<10>`  Select G1 character set
* `<11>`  Select G0 character set
* `<7F>`  Del (ignored)
* `<18>`  Ignored, terminates and cancels an ESC code
* `<1A>`  Ignored, terminates and cancels an ESC code
* `<0A>`, `<0B>`, `<0C>` ⁴Line feed: Moves cursor down within window; if at bottom, scrolls window up and inserts a blank line at bottom.

## Esc codes:

* `<1B>`      ESC.
* `<ESC> (`    SCS0.
* `<ESC> )`    SCS1.
* `<ESC> [`    CSI. An optional number of integer parameters may follow, separated by either `:` or `;`.
* `<SCS0> B ` Changes G0 set to 0
* `<SCS0> 0 ` Changes G0 set to 1
* `<SCS0> U ` Changes G0 set to 2
* `<SCS0> K ` Changes G0 set to 3
* `<SCS1> B ` Changes G1 set to 0
* `<SCS1> 0 ` Changes G1 set to 1
* `<SCS1> U ` Changes G1 set to 2
* `<SCS1> K ` Changes G1 set to 3
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
* `<ESC> M`   ⁴Reverse linefeed: Moves cursor up within window; if at top, scrolls window down and inserts a blank line at top.
* `<ESC> E`   Acts like `<0D>` followed by `<0A>`.
* `<ESC> # 8`   ²Clears screen with the letter “E” using the current attribute.
* `<ESC> % @`   Unsets UTF8 mode. (Ignored)
* `<ESC> % G`, `<ESC> % 8`   Sets UTF8 mode. (Ignored)
* `<CSI> g`    Set tab stops (UNIMPLEMENTED)
* `<CSI> q`    Set LED states (UNIMPLEMENTED)
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
* `<CSI> r`, `<CSI> ! p` ¹Sets the window top and bottom line numbers. Missing parameters are interpreted as the top and bottom of the screen respectively.
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
  * Mode 2: Keyboard locked. (Ignored)
  * Mode 4: Insert mode. (Ignored)
  * Mode 12: Local echo. (Ignored)
  * Mode 20: Auto linefeed. (Ignored)
* `<CSI> m` ³SGR attributes. If no parameters are given, a single zero-parameter is implied.
  * 0 = Sets default attributes (clears all modes listed below, and sets default foreground and default background color).
  * 1 = Sets bold.
  * 2 = Sets dim.
  * 3 = Sets italic.
  * 4 = Sets underline.
  * 5 = Sets blink.
  * 7 = Sets reverse.
  * 8 = Sets conceal. (Ignored)
  * 9 = Sets overstrike.
  * 20 = Sets fraktur. (Ignored)
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
  * 51 = Sets framed. (Ignored)
  * 52 = Sets encircled. (Ignored)
  * 53 = Sets overlined. (Ignored)
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
  * 30…37 = Sets foreground color \<n-30> using the 256-color lookup table.
  * 40…47 = Sets background color \<n-40> using the 256-color lookup table.
  * 90…97 = Sets foreground color \<n+8-90> using the 256-color lookup table.
  * 100…107 = Sets background color \<n+8-100> using the 256-color lookup table.
  
Blank = space character with current attributes.  
¹ = Missing or zero parameter is interpreted as 1.  
² = Ignores window boundaries.  
³ = Any number of parameters can be specified; all of them will be processed.  
⁴ = Does not allow the cursor to escape the window; however, if the cursor
  is already outside the window, allows moving to another row
  even if the target row is even farther outside the window.

None of the commands move the cursor unless explicitly specified.
This is important to note especially with the scrolling,
clearing/erasing, and writing commands.
