#pragma once
#include <stdint.h>
#include <assert.h>

#define Kilobytes(value)                (value*UINT64_C(1024))
#define Megabytes(value)                (Kilobytes(value)*UINT64_C(1024))
#define Gigabytes(value)                (Megabytes(value)*UINT64_C(1024))
#define Terabytes(value)                (Gigabytes(value)*UINT64_C(1024))

enum MemoryAllocationFlags
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

typedef size_t MemoryIndex;

struct MemoryPool
{
    MemoryIndex size;
    uint8_t *base;
    MemoryIndex used;

    int32_t tempCount;
};

struct TemporaryMemory
{
    MemoryPool *pool;
    MemoryIndex used;
};

inline void ClearMemory(void *ptr, MemoryIndex size)
{
#if 0
    // TODO: Check performance
    uint8_t *byte = (uint8_t *)ptr;
    while (size--)
    {
        *byte++ = 0;
    }
#else
    memset(ptr, 0, size);
#endif
}

inline void InitializeMemoryPool(MemoryPool *pool, MemoryIndex size, void *base)
{
    assert(size > 0);
    assert(base > 0);
    
    pool->size = size;
    pool->base = (uint8_t *)base;
    pool->used = 0;
    pool->tempCount = 0;
}

inline MemoryIndex GetAlignmentFromFlags(uint32_t flag)
{
    uint32_t alignFlag = flag & MemoryAllocation_AlignMask;
    uint32_t alignment = 1 << (alignFlag+2);
    return alignment;
}

inline MemoryIndex GetAlignmentOffset(const MemoryPool *pool, const MemoryIndex alignment)
{
    MemoryIndex alignmentOffset = 0;
    MemoryIndex resultPointer = (MemoryIndex)pool->base + pool->used;
    MemoryIndex alignmentMask = alignment - 1;
    if (resultPointer & alignmentMask)
    {
        alignmentOffset = alignment - (resultPointer & alignmentMask);
    }

    return alignmentOffset;
}

inline MemoryIndex GetAlignedSizeFor(const MemoryPool *pool, const MemoryIndex size, const MemoryIndex alignment)
{
    MemoryIndex alignmentOffset = GetAlignmentOffset(pool, alignment);
    MemoryIndex alignedSize = size + alignmentOffset;

    return alignedSize;
}

#define PushStruct(pool, type, ...)          (type *)PushSize(pool, sizeof(type), ## __VA_ARGS__)
#define PushArray(pool, count, type, ...)    (type *)PushSize(pool, count*sizeof(type), ## __VA_ARGS__)
inline void *PushSize(MemoryPool *pool, MemoryIndex size, uint32_t flags = MemoryAllocation_Default)
{
    MemoryIndex alignment = GetAlignmentFromFlags(flags);
    MemoryIndex alignedSize = GetAlignedSizeFor(pool, size, alignment);
    assert((pool->used + alignedSize) <= pool->size);
    MemoryIndex alignmentOffset = GetAlignmentOffset(pool, alignment);
    void *result = pool->base + pool->used + alignmentOffset;
    pool->used += alignedSize;

    if (flags & MemoryAllocation_ClearMask)
    {
        ClearMemory(result, alignedSize);
    }

    return result;
}

inline TemporaryMemory *PushTemporaryMemory(MemoryPool *pool)
{
    TemporaryMemory result = {};

    result.pool = pool;
    result.used = pool->used;
    ++pool->tempCount;
}

inline void PopTemporaryMemory(TemporaryMemory *tempMem)
{
    MemoryPool *pool = tempMem->pool;
    assert(pool->used >= tempMem->used);
    pool->used = tempMem->used;
    assert(pool->tempCount > 0);
    --pool->tempCount;
}

inline void CheckMemoryPool(const MemoryPool *pool)
{
    assert(pool->tempCount == 0);
}