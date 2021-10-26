/* Adds "person" animation into the given pixels (bgcolor,fgcolor).
 * width = width of screen in pixels
 * x,y   = coordinates on screen of this pixel
 * action_type: 1 = Top of screen; 2 = Bottommost row of top of screen
 * 0 = anything else
 */
void PersonTransform(unsigned& bgcolor, unsigned& fgcolor,
                     unsigned width, unsigned x, unsigned y,
                     unsigned action_type);
