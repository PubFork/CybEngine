#include "Precompiled.h"
#include "Base/ParallelJobQueue.h"
#include <Windows.h>

static bool ProcessNextJob(ParallelJobQueue *queue)
{
    bool putThreadToSleep = true;

    /*
    uint32_t OriginalNextEntryToRead = queue->nextEntryToRead;
    uint32_t NewNextEntryToRead = (OriginalNextEntryToRead + 1) % _countof(queue->entries);

    if (OriginalNextEntryToRead != queue->nextEntryToWrite)
    {
        uint32_t Index = InterlockedCompareExchange((LONG volatile *)&queue->nextEntryToRead,
                                                    NewNextEntryToRead,
                                                    OriginalNextEntryToRead);
        if (Index == OriginalNextEntryToRead)
        {
            ThreadedWorkQueueEntry Entry = queue->entries[Index];
            Entry.callback(queue, Entry.userData);
            InterlockedIncrement((LONG volatile *)&queue->completionCount);
        }

        putThreadToSleep = false;
    } 
    */
    return putThreadToSleep;
}

static DWORD WINAPI ThreadEntryProc(LPVOID lpParameter)
{
    ParallelJobQueue *queue = (ParallelJobQueue *)lpParameter;

    for (;;)
    {
        bool putThreadToSleep = ProcessNextJob(queue);
        if (putThreadToSleep)
        {
            WaitForSingleObjectEx(queue->semaphoreHandle, INFINITE, FALSE);
        }
    }

//    return 0;
}

void CreateParallelJobQueue(ParallelJobQueue *queue, uint32_t numThreads)
{
    queue->currentJob = 0;
    queue->numThreadsExecuting = numThreads;

    queue->semaphoreHandle = CreateSemaphoreEx(0, 0, numThreads, 0, 0, SEMAPHORE_ALL_ACCESS);

    for (uint32_t threadIndex = 0; threadIndex < numThreads; ++threadIndex)
    {
        DWORD threadId = 0;
        HANDLE threadHandle = CreateThread(0, 0, ThreadEntryProc, queue, 0, &threadId);
        CloseHandle(threadHandle);
    }
}