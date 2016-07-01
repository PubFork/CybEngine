#pragma once

#define JOB_ENTRY_CALLBACK(name) void name(void *userData)
typedef JOB_ENTRY_CALLBACK(JobEntryCallback);

struct ParallelJobEntry
{
    JobEntryCallback *callback;
    void *userData;
};

struct ParallelJobQueue
{
    volatile uint32_t currentJob;
    ParallelJobEntry jobQueue[256];
    uint32_t numThreadsExecuting;
    void *semaphoreHandle;
};

void CreateParallelJobQueue(ParallelJobQueue *queue, uint32_t numThreads);
void SubmitJob(ParallelJobQueue *queue, JobEntryCallback *callback, void *userData);
void WaitForQueueToFinish(ParallelJobQueue *queue);