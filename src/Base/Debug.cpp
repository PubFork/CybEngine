#include "Precompiled.h"
#include "Base/Debug.h"
#include "Base/Algorithm.h"
#include "Base/File.h"
#include "Base/Sys.h"

DebugPerformenceRecord *DebugPerformenceRecord::staticRecords = NULL;

std::ostrstream logStream;

FatalException::FatalException(const std::string &message) :
    errorMessage(message)
{
}

const char *FatalException::what() const
{
    return errorMessage.c_str();
}

void DebugAddMessageToBuffer(const char *msg)
{
    // TODO: Use something better than a stream buffer to save messages
    logStream << msg;
}

#define PRINT_BUFFER_SIZE       4096
void DebugPrintf(const char *fmt, ...)
{
    assert(fmt);
    static char msg[PRINT_BUFFER_SIZE];

    va_list args;
    va_start(args, fmt);
    _vsnprintf_s(msg, PRINT_BUFFER_SIZE, fmt, args);
    va_end(args);

    Sys_Printf(msg);
    DebugAddMessageToBuffer(msg);
}

void SaveDebugLogToFile(const char *filename)
{
    SysFile logFile(filename, FileOpen_WriteTruncate);
    logFile.Write((uint8_t *)logStream.str(), logStream.pcount());
    logStream.clear();
}

DebugPerformenceRecord::DebugPerformenceRecord(const char *inFunctionName, const char *inFileName, uint32_t inLineNumber) :
    functionName(inFunctionName),
    fileName(inFileName),
    lineNumber(inLineNumber),
    cycleCount(0),
    hitCount(0)
{
    next = DebugPerformenceRecord::staticRecords;
    DebugPerformenceRecord::staticRecords = this;
}

ScoopedTimedBlock::ScoopedTimedBlock(DebugPerformenceRecord *inRecord)
{
    record = inRecord;
    ++(record->hitCount);
    record->cycleCount -= Sys_GetClockTicks();
}

ScoopedTimedBlock::~ScoopedTimedBlock()
{
    record->cycleCount += Sys_GetClockTicks();
}

void DebugLogPerformanceCounters(const DebugPerformenceRecord *record)
{
    DebugPrintf("--------< Performance Counters >-----------------------------------------------------------\n");

    for (; record != NULL; record = record->next)
    {
        DebugPrintf("%s: CycleCount=%llu HitCount=%d AvgCycleCount=%d\n",
                    record->functionName,
                    record->cycleCount,
                    record->hitCount,
                    record->cycleCount / record->hitCount);
    }
}