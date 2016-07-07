#pragma once

#define NUM_ENTRIES_PER_QUEUE   256

typedef void(*job_callback)(void *);

struct job_entry
{
    job_callback Callback;
    void *Data;
    bool Executed;
};

struct parallel_job_queue
{
    volatile uint32_t CompletionGoal;
    volatile uint32_t CompletionCount;
    volatile uint32_t NextEntryWrite;
    volatile uint32_t NextEntryRead;

    job_entry JobEntries[NUM_ENTRIES_PER_QUEUE];
    void *SemaphoreHandle;
};

void CreateParallelJobQueue(parallel_job_queue *Queue, uint32_t NumThreads);
void SubmitJob(parallel_job_queue *Queue, job_callback Callback, void *Data);
void WaitForQueueToFinish(parallel_job_queue *Queue);

inline uint32_t GetThreadID(void)
{
    uint8_t *threadLocalStorage = (uint8_t *)__readgsqword(0x30);
    uint32_t threadID = *(uint32_t *)(threadLocalStorage + 0x48);

    return threadID;
}