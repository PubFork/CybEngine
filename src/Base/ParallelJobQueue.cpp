#include "Precompiled.h"
#include "Base/ParallelJobQueue.h"
#include "Base/Sys.h"
#include <Windows.h>

static DWORD WINAPI ThreadEntryProc(LPVOID lpParameter)
{
    parallel_job_queue *Queue = (parallel_job_queue *)lpParameter;
//    uint32_t threadIndex = GetThreadID();

    for (;;)
    {
        uint32_t OriginalNextEntryRead = Queue->NextEntryRead;
        uint32_t NewNextEntryRead = (OriginalNextEntryRead + 1) % _countof(Queue->JobEntries);
        
        if (OriginalNextEntryRead != Queue->NextEntryWrite)
        {
            uint32_t Index = AtomicCompareExchangeUInt32(&Queue->NextEntryRead,
                                                         NewNextEntryRead,
                                                         OriginalNextEntryRead);
            if (Index == OriginalNextEntryRead)
            {
                job_entry Entry = Queue->JobEntries[Index];
                Entry.Callback(Entry.Data);
                AtomicAddUint32(&Queue->CompletionCount, 1);
            }
        } 
        else
        {
            WaitForSingleObjectEx(Queue->SemaphoreHandle, INFINITE, FALSE);
        }
    }

//    return 0;
}

void CreateParallelJobQueue(parallel_job_queue *Queue, uint32_t numThreads)
{
    Queue->CompletionGoal = 0;
    Queue->CompletionCount = 0;
    Queue->NextEntryWrite = 0;
    Queue->NextEntryRead = 0;
    for (uint32_t i = 0; i < _countof(Queue->JobEntries); ++i)
    {
        Queue->JobEntries[i].Executed = true;
    }


    Queue->SemaphoreHandle = CreateSemaphoreEx(0, 0, numThreads, 0, 0, SEMAPHORE_ALL_ACCESS);

    for (uint32_t threadIndex = 0; threadIndex < numThreads; ++threadIndex)
    {
        DWORD threadId = 0;
        HANDLE threadHandle = CreateThread(0, 0, ThreadEntryProc, Queue, 0, &threadId);
        CloseHandle(threadHandle);
    }
}

void SubmitJob(parallel_job_queue *Queue, job_callback Callback, void *Data)
{
    uint32_t NewNextEntryWrite = (Queue->NextEntryWrite + 1) % _countof(Queue->JobEntries);
    assert(NewNextEntryWrite != Queue->NextEntryRead);

    job_entry *Entry = Queue->JobEntries + Queue->NextEntryWrite;
    assert(Entry->Executed != false);       // Need bigger NUM_ENTRIES_PER_QUEUE if this failes

    Entry->Callback = Callback;
    Entry->Data = Data;
    Entry->Executed = false;
    ++Queue->CompletionGoal;

    _WriteBarrier();
    Queue->NextEntryWrite = NewNextEntryWrite;
    ReleaseSemaphore(Queue->SemaphoreHandle, 1, 0);
}

void WaitForQueueToFinish(parallel_job_queue *Queue)
{
    while (Queue->CompletionGoal != Queue->CompletionCount)
    {
    }

    Queue->CompletionGoal = 0;
    Queue->CompletionCount = 0;
}