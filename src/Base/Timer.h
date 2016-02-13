#pragma once

class HiPerformanceTimer
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

std::string TimeStringNano(uint64_t nanos);