#pragma once

// use comple time generated hash values for quick map lookup
#include "StringUtils.h"
#define SCOOPED_PROFILE_EVENT(name) ScoopedProfileEntry ___scoopedProfiler(name, COMPILE_TIME_CRC32_STR(name), globalProfiler)

struct ProfileEntry
{
    typedef std::map<uint32_t /*hash*/, ProfileEntry> ChildMap;

    ProfileEntry(const std::string entryName = "", ProfileEntry *entryParent = nullptr);

    std::string name;
    uint64_t totalTime;
    uint64_t numCalls;

    ProfileEntry *parent;
    ChildMap childEntries;
};

// TODO: refactor out info/print output functions
class ProfilerDataCollector
{
public:
    enum 
    {
        NumIndentationSpaces = 4,
        MaxBlockLevel = 8 
    };

    ProfilerDataCollector();

    void BeginEvent(const char *name, uint32_t hash);
    void EndEvent();

    const ProfileEntry::ChildMap *GetEvents() const;

    std::string InfoString() const;
    void PrintToDebug() const;

private:
    uint64_t beginEventTime[MaxBlockLevel];
    uint32_t currentBlockLevel;
    ProfileEntry::ChildMap entries;
    ProfileEntry *currentEvent;
};

class ScoopedProfileEntry
{
public:
    ScoopedProfileEntry(const char *name, uint32_t hash, ProfilerDataCollector *profilerDC);
    ~ScoopedProfileEntry();

private:
    ProfilerDataCollector *profiler;
};

extern ProfilerDataCollector *globalProfiler;