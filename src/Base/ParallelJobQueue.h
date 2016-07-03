#pragma once

#define JOB_ENTRY_CALLBACK(name) void name(void *userData)
typedef JOB_ENTRY_CALLBACK(JobEntryCallback);

struct ParallelJobEntry
{
    JobEntryCallback *callback;
    void *userData;
    bool executed;
};

struct ParallelJobQueue
{
    volatile uint32_t completionGoal;
    volatile uint32_t completionCount;
    volatile uint32_t nextEntryWrite;
    volatile uint32_t nextEntryRead;

    ParallelJobEntry jobEntries[256];
    void *semaphoreHandle;
};

void CreateParallelJobQueue(ParallelJobQueue *queue, uint32_t numThreads);
void SubmitJob(ParallelJobQueue *queue, JobEntryCallback *callback, void *userData);
void WaitForQueueToFinish(ParallelJobQueue *queue);