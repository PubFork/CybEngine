#include <assert.h>
#include <malloc.h>
#include "memory.h"

/**
 * Allocates 16-byte aligned memory of the heap.
 * @param	numBytes		Requested number of bytes for the allocation.
 * @return The aligned heap memory.
 */
void *Mem_Alloc16( size_t numBytes ) {
	assert( numBytes >= 0 );

	size_t paddedSize = (numBytes + 15) & ~15;
	return _aligned_malloc( paddedSize, 16 );
}

/**
 * Free 16-byte aligned memory, allocated with Mem_Alloc16().
 * @param	memory			Pointer to the memory to free.
 */
void Mem_Free16( void *memory ) {
	if ( memory == nullptr ) {
		return;
	}

	_aligned_free( memory );
}

/**
 * Standard allocation using system's _aligned_malloc().
 * @param	numBytes		Requested number of bytes for the allocation.
 * @param	alignment		Alignment for the allocated memory (default: IAllocator::DefaultAlignment)
 * @return The aligned heap memory.
 */
void *CrtAllocator::Alloc( size_t numBytes, uint32_t alignment ) {
	assert( numBytes >= 0 );
	assert( (alignment & (alignment - 1)) == 0 );

	size_t paddedSize = (numBytes + alignment - 1) & ~(alignment - 1);
	return _aligned_malloc( paddedSize, alignment );
}

/**
 * Standard memory freeing using system's _aligned_free().
 * @param	memory			Pointer to the memory to free.
 */
void CrtAllocator::Free( void *memory ) {
	assert( memory );
	_aligned_free( memory );
}

/** Constuct a new linear allocator.
 * @param	memoryPoolSize	Number of bytes in the allocators memory pool.
 */
LinearAllocator::LinearAllocator( size_t memoryPoolSize ) {
	assert( memoryPoolSize >= 0 );

	memoryPool = Mem_Alloc16( memoryPoolSize );
	top = (uint8_t *)memoryPool;
	end = (uint8_t *)memoryPool + memoryPoolSize;
}

/** Destruct the linear allocator and free it's memory pool */
LinearAllocator::~LinearAllocator() {
	Mem_Free16( memoryPool );
}

/**
 * Allocate numBytes from the linear allocators memory pool.
 * If the pool doesen't have enough memory, application will crash,
 * this will never return nullptr.
 * TODO: Throw exception instead of crashig?
 */
void *LinearAllocator::Alloc( size_t numBytes, uint32_t alignment ) {
	assert( numBytes >= 0 );
	assert( (alignment & (alignment - 1)) == 0 );

	uint8_t *memory = top;
	numBytes = (numBytes + alignment - 1) & ~(alignment - 1);
	top += numBytes;

	assert( top < end );
	return memory;
}

/** Reset the allocator. */
void LinearAllocator::Flush() {
	top = (uint8_t *)memoryPool;
}

/**
* Allocates a shared pointer memory_t of 16-byte aligned heap memory.
* Memory will be automatically freed when there's no references to it.
* @param	numBytes		Requested number of bytes for the allocation.
* @return A shared pointer memory_t with data pointing to new allocated memory.
*/
SharedRef<memory_t> SharedMem_Alloc( size_t numBytes ) {
	assert( numBytes >= 0 );

	auto mem = SharedRef<memory_t>( (memory_t *)Mem_Alloc16( sizeof( memory_t ) + numBytes ), Mem_Free16 );
	mem->buffer = (uint8_t *)&mem.Get() + sizeof( memory_t );
	mem->size = numBytes;
	return mem;
}

/**
* Initializes a shared pointer memory_t, but uses refMem as data.
* This allows for static data to be used with memory_t.
* NOTE: while refMem is const, memory_t::buffer isn't, so data CAN be changed.
* @param	refMem			Pointer to pre-allocated referenced memory.
* @param	size			Number of bytes in refMem,
* @return A shared pointer memory_t with data pointing to refMem.
*/
SharedRef<memory_t> SharedMem_MakeRef( const void *refMem, size_t size ) {
	auto mem = SharedRef<memory_t>( (memory_t *)Mem_Alloc16( sizeof( memory_t ) ), Mem_Free16 );
	mem->buffer = (uint8_t *)const_cast<void *>(refMem);
	mem->size = size;
	return mem;
}

/**
* Same as \see SharedMem_Alloc( size_t numBytes ), but with a custom allocator.
* @param	numBytes		Requested number of bytes for the allocation.
* @return A shared pointer memory_t with data pointing to new allocated memory.
*/
SharedRef<memory_t> SharedMem_Alloc( SharedRef<IAllocator> allocator, size_t numBytes ) {
	assert( numBytes >= 0 );

	auto mem = SharedRef<memory_t>( (memory_t *)allocator->Alloc( sizeof( memory_t ) + numBytes ), [=]( void *memory ) {
		allocator->Free( memory );
	} );
	mem->buffer = (uint8_t *)&mem.Get() + sizeof( memory_t );
	mem->size = numBytes;
	return mem;
}

/** Type-safe allocation with custom allocator using new. */
void *operator new(size_t size, SharedRef<IAllocator> allocator, uint32_t count, uint32_t alignment ) {
	return allocator->Alloc( size * count, alignment );
}

/** Type-safe allocation with custom allocator using new[]. */
void *operator new[]( size_t size, SharedRef<IAllocator> allocator, uint32_t count, uint32_t alignment ) {
	return allocator->Alloc( size * count, alignment );
}

/** Type-safe deallocation with custom allocator using delete. */
void operator delete(void *object, SharedRef<IAllocator> allocator, uint32_t, uint32_t) {
	allocator->Free( object );
}

/** Type-safe deallocation with custom allocator using delete[]. */
void operator delete[]( void *object, SharedRef<IAllocator> allocator, uint32_t, uint32_t ) {
	allocator->Free( object );
}