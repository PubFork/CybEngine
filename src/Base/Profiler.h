#pragma once

#define SCOOPED_PROFILE_EVENT(name) ScoopedProfileEntry ___scoopedProfiler(name, globalProfiler)

struct ProfileEntry
{
    typedef std::map<std::string, ProfileEntry> ChildMap;

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

    void BeginEvent(const char *name);
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
    ScoopedProfileEntry(const char *name, ProfilerDataCollector *profilerDC);
    ~ScoopedProfileEntry();

private:
    ProfilerDataCollector *profiler;
};

extern ProfilerDataCollector *globalProfiler;