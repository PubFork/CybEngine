#include "Precompiled.h"
#include "Base/Timer.h"
#include <iomanip>
#include <chrono>           // TODO: Replace with Sys_GetClockTicks* functions

double HiPerformanceTimer::GetSeconds()
{
    using FpSeconds = std::chrono::duration<double, std::chrono::seconds::period>;

    auto now = std::chrono::high_resolution_clock::now();
    return FpSeconds(now.time_since_epoch()).count();
}

uint64_t HiPerformanceTimer::GetTicksNanos()
{
    using Uint64Nanoseconds = std::chrono::duration<uint64_t, std::chrono::nanoseconds::period>;

    auto now = std::chrono::high_resolution_clock::now();
    return Uint64Nanoseconds(now.time_since_epoch()).count();
}

std::string TimeStringNano(uint64_t nanos)
{
    static const char *timePrefix[] = { "ns", "us", "ms", "s", "m", "h" };
    double time = (double)nanos;
    uint16_t divCount = 0;

    while (time > 100.0 && divCount < _countof(timePrefix))
    {
        time /= 1000.0;
        divCount++;
    }

    std::string str(32, '\0');
    snprintf(&str[0], 32, "%.3f%s", time, timePrefix[divCount]);
    str.resize(strlen(str.c_str()));
    str.shrink_to_fit();
    return str;
}