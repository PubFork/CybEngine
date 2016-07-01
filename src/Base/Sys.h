#pragma once

struct Sys_ProcessorInfo
{
    const char *cpuString;
    uint32_t numLogicalProcessors;
    uint32_t numCores;

    uint32_t cacheLineSize;
    uint32_t L2Associativity;
    uint32_t cacheSizeK;
};

std::vector<std::string> Sys_GetGraphicCardList();       // TODO: Make C-API-able-isch
void Sys_GetProcessorInfo(Sys_ProcessorInfo *info);

void Sys_VPrintf(const char *fmt, va_list args);
void Sys_Printf(const char *fmt, ...);
void Sys_ErrorPrintf(const char *fmt, ...);

uint64_t Sys_GetClockTicks();
uint64_t Sys_GetClockTicksPerSecond();