double GetTime();
void   AdvanceTime(double seconds);  // Ignored if timefactor != 0
void   SleepFor(double seconds);     // Sleep this thread for given time. If timefactor=0, depends on AdvanceTime being used in another thread.
void   SetTimeFactor(double factor); // 0 = time advances only with AdvanceTime, 1 = normal time, >0 = relatively altered
void   TimeTerminate();
