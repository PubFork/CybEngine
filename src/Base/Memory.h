#pragma once
#include <stdint.h>
#include <assert.h>

#define Kilobytes(value)                (value*UINT64_C(1024))
#define Megabytes(value)                (Kilobytes(value)*UINT64_C(1024))
#define Gigabytes(value)                (Megabytes(value)*UINT64_C(1024))
#define Terabytes(value)                (Gigabytes(value)*UINT64_C(1024))

enum memory_allocation_flags
{
    MemoryAllocation_Align4     = 0x00,
    MemoryAllocation_Align8     = 0x01,
    MemoryAllocation_Align16    = 0x02,
    MemoryAllocation_Align32    = 0x03,
    MemoryAllocation_AlignMask  = 0x0f,
    MemoryAllocation_Clear      = 0x10,
    MemoryAllocation_ClearMask  = 0xf0,

    MemoryAllocation_Default    = MemoryAllocation_Align4 | MemoryAllocation_Clear
};

typedef size_t memory_index;

struct memory_pool
{
    memory_index Size;
    uint8_t *Base;
    memory_index Used;

    int32_t TempCount;
};

struct temporaty_memory
{
    memory_pool *Pool;
    memory_index Used;
};

inline void ClearMemory(void *Address, memory_index Length)
{
#if 0
    // TODO: Check performance
    uint8_t *byte = (uint8_t *)ptr;
    while (size--)
    {
        *byte++ = 0;
    }
#else
    memset(Address, 0, Length);
#endif
}

inline void InitializeMemoryPool(memory_pool *Pool, memory_index Size, void *Base)
{
    assert(Size > 0);
    assert(Base > 0);
    
    Pool->Size = Size;
    Pool->Base = (uint8_t *)Base;
    Pool->Used = 0;
    Pool->TempCount = 0;
}

inline memory_index GetAlignmentFromFlags(uint32_t AllocationFlags)
{
    uint32_t AlignFlag = AllocationFlags & MemoryAllocation_AlignMask;
    uint32_t Alignment = 1 << (AlignFlag+2);
    return Alignment;
}

inline memory_index GetAlignmentOffset(const memory_pool *Pool, const memory_index Alignment)
{
    memory_index AlignmentOffset = 0;
    memory_index ResultPointer = (memory_index)Pool->Base + Pool->Used;
    memory_index AlignmentMask = Alignment - 1;
    if (ResultPointer & AlignmentMask)
    {
        AlignmentOffset = Alignment - (ResultPointer & AlignmentMask);
    }

    return AlignmentOffset;
}

inline memory_index GetAlignedSizeFor(const memory_pool *Pool, const memory_index Size, const memory_index Alignment)
{
    memory_index AlignmentOffset = GetAlignmentOffset(Pool, Alignment);
    memory_index AlignedSize = Size + AlignmentOffset;

    return AlignedSize;
}

#define PushStruct(Pool, Type, ...)          (Type *)PushSize(Pool, sizeof(Type), ## __VA_ARGS__)
#define PushArray(Pool, Count, Type, ...)    (Type *)PushSize(Pool, Count*sizeof(Type), ## __VA_ARGS__)
inline void *PushSize(memory_pool *Pool, memory_index Size, uint32_t AllocationFlags = MemoryAllocation_Default)
{
    memory_index Alignment = GetAlignmentFromFlags(AllocationFlags);
    memory_index AlignedSize = GetAlignedSizeFor(Pool, Size, Alignment);
    assert((Pool->Used + AlignedSize) <= Pool->Size);
    memory_index AlignmentOffset = GetAlignmentOffset(Pool, Alignment);
    void *Result = Pool->Base + Pool->Used + AlignmentOffset;
    Pool->Used += AlignedSize;

    if (AllocationFlags & MemoryAllocation_ClearMask)
    {
        ClearMemory(Result, AlignedSize);
    }

    return Result;
}

inline temporaty_memory *PushTemporaryMemory(memory_pool *Pool)
{
    temporaty_memory Result = {};

    Result.Pool = Pool;
    Result.Used = Pool->Used;
    ++Pool->TempCount;
}

inline void PopTemporaryMemory(temporaty_memory *TemporaryMemory)
{
    memory_pool *Pool = TemporaryMemory->Pool;
    assert(Pool->Used >= TemporaryMemory->Used);
    Pool->Used = TemporaryMemory->Used;
    assert(Pool->TempCount > 0);
    --Pool->TempCount;
}

inline void CheckMemoryPool(const memory_pool *Pool)
{
    assert(Pool->TempCount == 0);
}