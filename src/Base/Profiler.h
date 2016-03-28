#pragma once
#include "Base/Container/InsertionOrderedMap.h"

// a scooped profile event
#define SCOOPED_PROFILE_EVENT(name) ScoopedProfileEvent ___scoopedProfileEvent(name, globalEventProfiler)

//==============================
// Event Profiler
//==============================

class EventProfiler
{
public:
    enum { NumIndentationSpaces = 4 };

    EventProfiler();
    void PushEvent(const std::string name);
    void PopEvent();

    std::string CreateInfoMessage() const;

private:
    struct ProfileEventNode
    {
        std::string name;
        uint64_t startTime;
        uint64_t totalTime;
        uint64_t numCalls;

        ProfileEventNode *parent;
        InsertionOrderedMap<std::string, ProfileEventNode> childMap;
    };

    ProfileEventNode *currentEvent;
    InsertionOrderedMap<std::string, ProfileEventNode> rootEventNodes;
};

//==============================
// Scooped Profile Event
//==============================

class ScoopedProfileEvent
{
public:
    ScoopedProfileEvent(const char *name, EventProfiler *dataCollector);
    ~ScoopedProfileEvent();

private:
    EventProfiler *profiler;
};

extern EventProfiler *globalEventProfiler;
