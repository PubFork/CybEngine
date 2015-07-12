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