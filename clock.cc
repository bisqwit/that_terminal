#include <chrono>
#include <mutex>
#include <condition_variable>
#include <thread>

/** Gets real starting time of the program, effective when this function is first called. */
static auto& GetRealStartTime()
{
    static auto start = std::chrono::system_clock::now();
    return start;
}

/** Gets real elapsed time, without any scaling. */
static double GetRealElapsed()
{
    auto start = GetRealStartTime();
    auto now = std::chrono::system_clock::now();
    std::chrono::duration<double> dur = (now - start);
    return dur.count();
}

/** Current settings of this module. */
static struct
{
    double TimeFactor = 1.0;
    double FakeTime   = 0.0;
    std::mutex              lk;
    std::condition_variable updated;
    double Waiting    = 0;
    bool   Terminated = false;
} data;

/** Last time read in this thread. */
static thread_local double LastTime = 0.0;

double GetTime()
{
    if(data.TimeFactor != 0.0)
    {
        return GetRealElapsed() * data.TimeFactor;
    }
    return data.FakeTime;
}

void AdvanceTime(double seconds)
{
    data.FakeTime += seconds;
    if(data.TimeFactor == 0.0)
    {
        if(data.Waiting != 0 && data.Waiting <= data.FakeTime)
        {
            {std::unique_lock<std::mutex> lock(data.lk);}
            data.updated.notify_all();
        }
        LastTime = data.FakeTime;
    }
}

void SleepFor(double seconds)
{
    if(data.TimeFactor != 0.0)
    {
        std::this_thread::sleep_for(std::chrono::duration<double>(seconds / data.TimeFactor));
    }
    else
    {
        double until = LastTime+seconds;
        if(data.FakeTime < until)
        {
            std::unique_lock<std::mutex> lock(data.lk);
            data.Waiting = until;
            if(data.FakeTime < until)
            {
                data.updated.wait(lock, [&]{ return data.FakeTime >= until || data.Terminated; });
            }
            data.Waiting = 0;
        }
        LastTime = until;
    }
}

void SetTimeFactor(double factor)
{
    data.TimeFactor = factor;
}

void TimeTerminate()
{
    data.Terminated = true;
    {std::unique_lock<std::mutex> lock(data.lk);}
    data.updated.notify_all();
}
