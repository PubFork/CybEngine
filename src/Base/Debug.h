#pragma once

#define TIMED_FUNCTION__(num, name)     static DebugPerformenceRecord perfRecord_##num(name, __FILE__, __LINE__); ScoopedTimedBlock scoopedTimedBlock_##num(&perfRecord_##num)
#define TIMED_FUNCTION_(num)            TIMED_FUNCTION__(num, __FUNCTION__)
#define TIMED_FUNCTION()                TIMED_FUNCTION_(__LINE__)
#define TIMED_NAMED_BLOCK(name)         TIMED_FUNCTION__(__LINE__, name)

class FatalException : public std::exception
{
public:
    FatalException(const std::string &message);
    virtual ~FatalException() = default;

    virtual const char *what() const final;

private:
    std::string errorMessage;
};

// Copy the message to a buffer, making it possible to later save the messages to a file.
void DebugAddMessageToBuffer(const char *msg);

// Print a formatted string using Sys_Printf and adds message to debug message buffer.
#define CondititionalDebugPrintf(expression, ...) if (expression) { DebugPrintf(__VA_ARGS__); }
void DebugPrintf(const char *fmt, ...);

// Save a messages passed though DebugPrintf to a file, this also empties the buffer
// containing the messages.
void SaveDebugLogToFile(const char *filename);

struct DebugPerformenceRecord
{
    uint64_t cycleCount;
    uint32_t hitCount;
    const char *functionName;
    const char *fileName;
    uint32_t lineNumber;

    DebugPerformenceRecord *next;
    static DebugPerformenceRecord *staticRecords;

    DebugPerformenceRecord(const char *inFunctionName, const char *inFileName, uint32_t inLineNumber);
};

struct ScoopedTimedBlock
{
    DebugPerformenceRecord *record;

    ScoopedTimedBlock(DebugPerformenceRecord *inRecord);
    ~ScoopedTimedBlock();
};

// Using DebugPrintf to print out information about all linked performance records.
void DebugLogPerformanceCounters(const DebugPerformenceRecord *record);
