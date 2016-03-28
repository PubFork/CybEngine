#include "stdafx.h"
#include "Profiler.h"
#include "Timer.h"
#include "Debug.h"
#include "Macros.h"
#include <iomanip>

static EventProfiler staticEventProfiler;
EventProfiler *globalEventProfiler = &staticEventProfiler;

//==============================
// Event Profiler
//==============================

EventProfiler::EventProfiler()
{
    currentEvent = nullptr;
}

void EventProfiler::PushEvent(const std::string name)
{
    auto childMap = (currentEvent) ? &currentEvent->childMap : &rootEventNodes;
    ProfileEventNode *node = childMap->Find(name);
    if (!node)
    {
        node = childMap->Insert(name, ProfileEventNode());
        node->name = name;
        node->totalTime = 0;
        node->numCalls = 0;
        node->parent = currentEvent;
    }

    node->numCalls++;
    node->startTime = HiPerformanceTimer::GetTicksNanos();
    currentEvent = node;
}

void EventProfiler::PopEvent()
{
    assert(currentEvent);

    ProfileEventNode *node = currentEvent;
    node->totalTime += HiPerformanceTimer::GetTicksNanos() - node->startTime;
    node->startTime = 0;
    currentEvent = node->parent;
}

std::string EventProfiler::CreateInfoMessage() const
{
    std::ostrstream infoString;

    std::function<void(const ProfileEventNode *, int)> printProfileEvent = [&](const ProfileEventNode *node, int indentLevel)
    {
        std::string indentSpaces(indentLevel * NumIndentationSpaces, ' ');
        infoString << indentSpaces << node->name << ": ";
        infoString << "TotalRuntime " << TimeStringNano(node->totalTime);
        infoString << ", AverageRuntime " << TimeStringNano(node->totalTime / node->numCalls);
        infoString << ", Count " << node->numCalls;

        if (node->parent)
        {
            double percentOfParentTime = (double)node->totalTime / (double)node->parent->totalTime * 100.0;
            infoString << ", " << std::setprecision(2) << std::fixed << percentOfParentTime << "% of parent runtime";
        }
        infoString << "\n";

        FOR_EACH(node->childMap, [&](const auto &it) { printProfileEvent(it, indentLevel + 1); });
    };

    infoString << "-----------------------------------------------------------------------------\n";
    FOR_EACH(rootEventNodes, [&](const auto &it) { printProfileEvent(it, 0); });
    infoString << "-----------------------------------------------------------------------------";

    infoString << '\0';     // HACK: Not sure why, but without this, junk is added at the end of the infoString
    return infoString.str();
}

//==============================
// Scooped Profile Event
//==============================

ScoopedProfileEvent::ScoopedProfileEvent(const char *name, EventProfiler *dataCollector) :
    profiler(dataCollector)
{
    assert(name);
    assert(profiler);
    profiler->PushEvent(name);
}

ScoopedProfileEvent::~ScoopedProfileEvent()
{
    profiler->PopEvent();
}
