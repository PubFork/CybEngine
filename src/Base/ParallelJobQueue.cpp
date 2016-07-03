#include "Precompiled.h"
#include "Base/ParallelJobQueue.h"
#include "Sys.h"
#include <Windows.h>

inline uint32_t GetThreadID(void)
{
    uint8_t *ThreadLocalStorage = (uint8_t *)__readgsqword(0x30);
    uint32_t ThreadID = *(uint32_t *)(ThreadLocalStorage + 0x48);

    return ThreadID;
}

static DWORD WINAPI ThreadEntryProc(LPVOID lpParameter)
{
    ParallelJobQueue *queue = (ParallelJobQueue *)lpParameter;
//    uint32_t threadIndex = GetThreadID();

    for (;;)
    {
        uint32_t originalNextEntryRead = queue->nextEntryRead;
        uint32_t newNextEntryRead = (originalNextEntryRead + 1) % _countof(queue->jobEntries);
        
        if (originalNextEntryRead != queue->nextEntryWrite)
        {
            uint32_t index = InterlockedCompareExchange((LONG volatile *)&queue->nextEntryRead,
                                                        newNextEntryRead,
                                                        originalNextEntryRead);
            if (index == originalNextEntryRead)
            {
                ParallelJobEntry entry = queue->jobEntries[index];
                entry.callback(entry.userData);
                InterlockedIncrement((LONG volatile *)&queue->completionCount);
            }
        } 
        else
        {
            WaitForSingleObjectEx(queue->semaphoreHandle, INFINITE, FALSE);
        }
    }

//    return 0;
}

void CreateParallelJobQueue(ParallelJobQueue *queue, uint32_t numThreads)
{
    queue->completionGoal = 0;
    queue->completionCount = 0;
    queue->nextEntryWrite = 0;
    queue->nextEntryRead = 0;

    queue->semaphoreHandle = CreateSemaphoreEx(0, 0, numThreads, 0, 0, SEMAPHORE_ALL_ACCESS);

    for (uint32_t threadIndex = 0; threadIndex < numThreads; ++threadIndex)
    {
        DWORD threadId = 0;
        HANDLE threadHandle = CreateThread(0, 0, ThreadEntryProc, queue, 0, &threadId);
        CloseHandle(threadHandle);
    }
}

void SubmitJob(ParallelJobQueue *queue, JobEntryCallback *callback, void *userData)
{
    uint32_t newNextEntryWrite = (queue->nextEntryWrite + 1) % _countof(queue->jobEntries);
    assert(newNextEntryWrite != queue->nextEntryRead);

    ParallelJobEntry *entry = queue->jobEntries + queue->nextEntryWrite;
    entry->callback = callback;
    entry->userData = userData;
    entry->executed = false;
    ++queue->completionGoal;

    _WriteBarrier();
    queue->nextEntryWrite = newNextEntryWrite;
    ReleaseSemaphore(queue->semaphoreHandle, 1, 0);
}

void WaitForQueueToFinish(ParallelJobQueue *queue)
{
    while (queue->completionGoal != queue->completionCount)
    {
    }

    queue->completionGoal = 0;
    queue->completionCount = 0;
}