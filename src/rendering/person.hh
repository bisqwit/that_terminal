#ifndef bqtPersonHH
#define bqtPersonHH
/** @file rendering/person.hh
 * @brief Defines PersonTransform(), which renders a person animation on the screen.
 */

/** Adds "person" animation into the given pixels
 *
 * @param bgcolor background color in that area
 * @param fgcolor foreground color in that area
 * @param width   width of screen in pixels
 * @param x       x coordinate on screen of this pixel
 * @param y       y coordinate on screen of this pixel
 * @param action_type  1 = Top of screen; 2 = Bottommost row of top of screen
 *                     0 = anything else
 */
void PersonTransform(unsigned& bgcolor, unsigned& fgcolor,
                     unsigned width, unsigned x, unsigned y,
                     unsigned action_type);

/** Retrieves timer-based person starting X coordinates for a window of given width
 * @param window_width window width in pixels
 * @returns X coordinate where Person would be drawn at this time
 */
int PersonBaseX(unsigned window_width);


#endif
