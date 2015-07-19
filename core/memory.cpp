//#include <Windows.h>	// needed for cache line stuff
#include <malloc.h>
#include "memory.h"

namespace cyb {

Memory::Memory() {
	buffer = nullptr;
	size = 0;
}

MemoryAutoDelete::~MemoryAutoDelete() {
	if ( buffer != nullptr ) {
		_aligned_free( buffer );
	}
}

MemoryPtr MemAlloc( size_t size ) {
	auto mem = std::make_shared<MemoryAutoDelete>();
	mem->buffer = _aligned_malloc( size, s_memoryAlignment );
	mem->size = size;
	return mem;
}

MemoryPtr MemCopy( const void *buffer, size_t size ) {
	auto mem = MemAlloc( size );
	memcpy( mem->buffer, buffer, size );
	return mem;
}

MemoryPtr MemMakeRef( const void *buffer, size_t size ) {
	auto mem = std::make_shared<Memory>();
	mem->buffer = const_cast<void *>( buffer );
	mem->size = size;
	return mem;
}

}	// namespace cyb

void *Mem_Alloc16( size_t bytes ) {
	size_t paddedSize = ( bytes + 15 ) &~ 15;
	return _aligned_malloc( paddedSize, 16 );
}

void Mem_Free16( void *ptr ) {
	if ( ptr == nullptr ) {
		return;
	}

	_aligned_free( ptr );
}

//=======================================================================

#define assert_64_byte_aligned( ptr )		assert( ( ((uint64_t)(ptr)) &  63 ) == 0 )
#define assert_128_byte_aligned( ptr )		assert( ( ((uint64_t)(ptr)) & 127 ) == 0 )

void ZeroCacheLine( void *ptr, uint32_t offset ) {
	assert_64_byte_aligned( ptr );
	char *bytePtr = ( (char *) ptr ) + offset;
	__m128i zero = _mm_setzero_si128();
	_mm_store_si128( ( __m128i * ) ( bytePtr + 0 ), zero );
	_mm_store_si128( ( __m128i * ) ( bytePtr + 16 ), zero );
	_mm_store_si128( ( __m128i * ) ( bytePtr + 32 ), zero );
	_mm_store_si128( ( __m128i * ) ( bytePtr + 48 ), zero );
}

void FlushCacheLine( const void *ptr, int offset ) {
	const char * bytePtr = ( (const char *) ptr ) + offset;
	_mm_clflush( bytePtr );
}