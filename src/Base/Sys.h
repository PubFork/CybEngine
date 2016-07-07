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

void *Sys_Alloc(uint64_t size);
void Sys_Free(void *address);

std::vector<std::string> Sys_GetGraphicCardList();       // TODO: Make C-API-able-isch
void Sys_GetProcessorInfo(Sys_ProcessorInfo *info);

void Sys_VPrintf(const char *fmt, va_list args);
void Sys_Printf(const char *fmt, ...);
void Sys_ErrorPrintf(const char *fmt, ...);

uint64_t Sys_GetClockTicks();
uint64_t Sys_GetClockTicksPerSecond();
void Sys_Sleep(uint32_t milliseconds);

uint32_t AtomicCompareExchangeUInt32(uint32_t volatile *value, uint32_t exchange, uint32_t compareand);
uint64_t AtomicCompareExchangeUInt64(uint64_t volatile *value, uint64_t exchange, uint64_t compareand);
uint32_t AtomicAddUint32(uint32_t volatile *value, uint32_t addend);
uint64_t AtomicAddUint64(uint64_t volatile *value, uint64_t addend);