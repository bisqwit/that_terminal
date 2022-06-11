#ifndef bqtThatTerminalClockHH
#define bqtThatTerminalClockHH
/**@file clock.hh
 * @brief Time handling interface
 */

/** @returns the number of seconds elapsed since the program started. */
double GetTime();

/** Advances time. Used when TimeFactor is set to 0. Ignored otherwise.
 * @param seconds Number of seconds to advance. */
void   AdvanceTime(double seconds);

/** Puts this thread into sleep for given time.
 * If timefactor=0, another thread must call AdvanceTime in order for this thread to wake.
 * @param seconds Time to sleep for, in seconds
 */
void   SleepFor(double seconds);

/** Changes the flow of time.
 * @param factor  Has the following meanings:<br>
 *                  0 = time advances only with AdvanceTime<br>
 *                  1 = normal time<br>
 *                  >0 = normal time, but scaled by the given factor (i.e. 2 = time goes twice as fast as realtime)
 */
void   SetTimeFactor(double factor);

/** Terminates any background process/thread started by the clock module. */
void   TimeTerminate();

#endif
