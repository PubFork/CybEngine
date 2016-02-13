#include "stdafx.h"
#include "Profiler.h"
#include "Timer.h"
#include "Debug.h"
#include "Macros.h"

static ProfilerDataCollector staticProfiler;
ProfilerDataCollector *globalProfiler = &staticProfiler;

//==============================
// Profile Entry
//==============================

ProfileEntry::ProfileEntry(const std::string entryName, ProfileEntry *entryParent) :
    name(entryName),
    totalTime(0),
    numCalls(1),
    parent(entryParent)
{
}

//==============================
// Profile Manager
//==============================

ProfilerDataCollector::ProfilerDataCollector() :
    currentBlockLevel(0),
    currentEvent(nullptr)
{
}

void ProfilerDataCollector::BeginEvent(const char *name)
{
    ProfileEntry::ChildMap &childMap = (!currentEvent) ? entries : currentEvent->childEntries;
    auto it = childMap.find(name);
    if (it != childMap.end())
    {
        ProfileEntry *entry = &it->second;
        entry->numCalls++;
        currentEvent = entry;
    }
    else
    {
        childMap[name] = ProfileEntry(name, currentEvent);
        currentEvent = &childMap[name];
    }

    beginEventTime[currentBlockLevel] = HiPerformanceTimer::GetTicksNanos();
    currentBlockLevel++;
    assert(currentBlockLevel < MaxBlockLevel);
}

void ProfilerDataCollector::EndEvent()
{
    assert(currentBlockLevel > 0);

    currentBlockLevel--;
    currentEvent->totalTime += HiPerformanceTimer::GetTicksNanos() - beginEventTime[currentBlockLevel];
    currentEvent = currentEvent->parent;
}

const ProfileEntry::ChildMap *ProfilerDataCollector::GetEvents() const
{
    return &entries;
}

void PrintProfileEntry(const ProfileEntry &entry, std::ostream &os, int indentLevel)
{
    std::string indentSpaces(indentLevel * ProfilerDataCollector::NumIndentationSpaces, ' ');

    os << indentSpaces << entry.name << ": ";
    os << "TotalRuntime " << TimeStringNano(entry.totalTime);
    os << ", AverageRuntime " << TimeStringNano(entry.totalTime / entry.numCalls);
    os << ", CallCount " << entry.numCalls;

    if (entry.parent)
    {
        double percentOfParentTime = (double)entry.totalTime / (double)entry.parent->totalTime * 100.0;
        os << ", " << percentOfParentTime << "% of parent runtime";
    } 
    os << "\n";

    // recursively print all the child events
    FOR_EACH(entry.childEntries, [&](const auto &it) { PrintProfileEntry(it.second, os, indentLevel + 1); });
}

std::string ProfilerDataCollector::InfoString() const
{
    std::ostrstream infoString;

    FOR_EACH(entries, [&](const auto &it) { PrintProfileEntry(it.second, infoString, 0); });
    infoString << '\0';     // HACK: Not sure why, but without this, junk is added at the end of the infoString
    return infoString.str();
}

void ProfilerDataCollector::PrintToDebug() const
{
    DEBUG_LOG_TEXT("-----------------------------------------------------------------------------");
    DEBUG_LOG_TEXT("%s-----------------------------------------------------------------------------", InfoString().c_str());
}

//==============================
// Scooped Profile Entry
//==============================

ScoopedProfileEntry::ScoopedProfileEntry(const char *name, ProfilerDataCollector *profilerDC) :
    profiler(profilerDC)
{
    assert(name);
    assert(profiler);
    profiler->BeginEvent(name);
}

ScoopedProfileEntry::~ScoopedProfileEntry()
{
    profiler->EndEvent();
}