#pragma once

namespace core
{

class Timer
{
public:
    enum 
    {
        MsPerSecond = 1000,                     // Milliseconds in one second.
        MksPerSecond = 1000 * 1000,             // Microseconds in one second.
        NanosPerSecond = 1000 * 1000 * 1000,    // Nanoseconds in one second.
    };

    static double GetSeconds();
    static uint64_t GetTicksNanos();
};

} // core