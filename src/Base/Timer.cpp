#include "stdafx.h"
#include "Timer.h"

namespace core
{

double Timer::GetSeconds()
{
    using FpSeconds = std::chrono::duration<double, std::chrono::seconds::period>;

    auto now = std::chrono::high_resolution_clock::now();
    return FpSeconds(now.time_since_epoch()).count();
}

uint64_t Timer::GetTicksNanos()
{
    using Uint64Nanoseconds = std::chrono::duration<uint64_t, std::chrono::nanoseconds::period>;

    auto now = std::chrono::high_resolution_clock::now();
    return Uint64Nanoseconds(now.time_since_epoch()).count();
}

} // core