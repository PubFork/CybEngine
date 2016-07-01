#include "Precompiled.h"
#include "Base/Sys.h"
#include "Base/Debug.h"
#include <Windows.h>

#define PRINT_BUFFER_LENGTH     4096

std::vector<std::string> Sys_GetGraphicCardList()
{
    std::vector<std::string> gpus;
    DISPLAY_DEVICE dd;
    dd.cb = sizeof(dd);

    DWORD deviceNum = 0;
    while (EnumDisplayDevices(NULL, deviceNum, &dd, 0))
    {
        if (dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)
        {
            gpus.push_back(std::string(dd.DeviceString));
        }

        deviceNum++;
    }

    return gpus;
}

void Sys_GetProcessorInfo(Sys_ProcessorInfo *info)
{
    // Note: https://msdn.microsoft.com/en-us/library/hskdteyh(v=vs.100).aspx
    static char cpuBrandString[0x40] = {};
    int cpuInfo[4] = { -1 };

    for (int i = 0; i < 3; ++i)
    {
        __cpuid(cpuInfo, 0x80000002 + i);
        memcpy(cpuBrandString + i*16, cpuInfo, sizeof(cpuInfo));
    }

    // cpu brand string is sometimes prefixed with spaces, skip those
    intptr_t brandStringBeginIndex = 0;
    while (*(cpuBrandString + brandStringBeginIndex) == ' ') ++brandStringBeginIndex;
    info->cpuString = cpuBrandString + brandStringBeginIndex;

    __cpuid(cpuInfo, 0x80000006);
    info->cacheLineSize = cpuInfo[2] & 0xff;
    info->L2Associativity = (cpuInfo[2] >> 12) & 0xf;
    info->cacheSizeK = (cpuInfo[2] >> 16) & 0xffff;

    __cpuid(cpuInfo, 1);
    info->numLogicalProcessors = ((cpuInfo[1] >> 16) & 0xff);

    __cpuid(cpuInfo, 0x4);
    info->numCores = (cpuInfo[0] >> 26) + 1;
}

void Sys_VPrintf(const char *fmt, va_list args)
{
    char msg[PRINT_BUFFER_LENGTH];

    _vsnprintf_s(msg, PRINT_BUFFER_LENGTH - 1, fmt, args);
    msg[PRINT_BUFFER_LENGTH - 1] = '\0';
    OutputDebugString(msg);
}

void Sys_Printf(const char *fmt, ...)
{   
    va_list args;
    va_start(args, fmt);
    Sys_VPrintf(fmt, args);
    va_end(args);
}

void Sys_ErrorPrintf(const char *fmt, ...)
{
    char msg[PRINT_BUFFER_LENGTH];
    char msgWithPrefix[PRINT_BUFFER_LENGTH];

    va_list args;
    va_start(args, fmt);
    _vsnprintf_s(msg, PRINT_BUFFER_LENGTH - 1, fmt, args);
    msg[PRINT_BUFFER_LENGTH - 1] = '\0';
    va_end(args);

    _snprintf(msgWithPrefix, PRINT_BUFFER_LENGTH - 1, "*** Error: %s", msg);
    msgWithPrefix[PRINT_BUFFER_LENGTH - 1] = '\0';

    Sys_Printf(msgWithPrefix);
    DebugAddMessageToBuffer(msgWithPrefix);
    MessageBox(0, msg, 0, MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
}

uint64_t Sys_GetClockTicks()
{
    uint64_t clockCycles = __rdtsc();
    return clockCycles;
}

uint64_t Sys_GetClockTicksPerSecond()
{
    static uint64_t frequency = 0;

    if (!frequency)
    {
        LARGE_INTEGER li;
        QueryPerformanceFrequency(&li); 
        frequency = li.QuadPart * 1000;     // Ms -> Sec
        assert(frequency != 0);
    }

    return frequency;
}