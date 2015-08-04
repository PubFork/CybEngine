#pragma once
#include <stdint.h>
#include <memory>

#define BYTES( x )		( (x) )
#define KILOBYTES( x )	( (x) << 10 )
#define MEGABYTES( x )	( (x) << 20 )
#define GIGABYTES( x )	( (x) << 30 )

// 16-byte aligned heap allocations
void *Mem_Alloc16( size_t numBytes );
void Mem_Free16( void *memory );

/** Container for SharedMem_* functions. */
struct memory_t {
	void *buffer;
	size_t size;
};

std::shared_ptr<memory_t> SharedMem_Alloc( size_t numBytes );
std::shared_ptr<memory_t> SharedMem_MakeRef( const void *refMem, size_t size );

/** Memory allocator interface. */
class IAllocator {
public:
	enum { DefaultAlignment = 16 };

	virtual void *Alloc( size_t numBytes, uint32_t alignment = DefaultAlignment ) = 0;
	virtual void Free( void *memory ) = 0;
	virtual void Flush() = 0;
};

// Type-safe allocations using custom allocators
void *operator new  ( size_t size, IAllocator *allocator, uint32_t count = 1, uint32_t alignment = IAllocator::DefaultAlignment );
void *operator new[]( size_t size, IAllocator *allocator, uint32_t count = 1, uint32_t alignment = IAllocator::DefaultAlignment );
void operator delete  ( void *object, IAllocator *allocator, uint32_t, uint32_t);
void operator delete[]( void *object, IAllocator *allocator, uint32_t, uint32_t );

// When using this,ensure that the allocator lifespan exceeds 
// the life of the allocated memory
std::shared_ptr<memory_t> SharedMem_Alloc( IAllocator *allocator, size_t numBytes );

/** System malloc/free allocator. */
class CrtAllocator : public IAllocator {
public:
	virtual void *Alloc( size_t numBytes, uint32_t alignment = DefaultAlignment ) final;
	virtual void Free( void *memory ) final;
	virtual void Flush() final {}
};

/** 
 * Simple linear allocator. 
 * Free() does nothing, use Flush() to reset the allocator.
 */
class LinearAllocator : public IAllocator {
public:
	LinearAllocator( size_t memoryPoolSize );
	virtual ~LinearAllocator() final;
	virtual void *Alloc( size_t numBytes, uint32_t alignment = DefaultAlignment ) final;
	virtual void Free( void * ) final {}
	virtual void Flush() final;

private:
	uint8_t *top;
	uint8_t *end;
	void *memoryPool;
};