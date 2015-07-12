#pragma once
#include <memory>

#define CYB_BYTES( x )		( x )
#define CYB_KILOBYTES( x )	( ( x ) << 10 )
#define CYB_MEGABYTES( x )	( ( x ) << 20 )
#define CYB_GIGABYTES( x )	( ( x ) << 30 )

namespace cyb {

static const size_t s_memoryAlignment = static_cast<size_t>(2 * sizeof( uintptr_t ));

struct Memory {
	Memory();
	virtual ~Memory() = default;

	void *buffer;
	size_t size;
};

struct MemoryAutoDelete : public Memory {
	MemoryAutoDelete() : Memory() {}
	virtual ~MemoryAutoDelete() final;
};

using MemoryPtr = std::shared_ptr<Memory>;
MemoryPtr MemAlloc( size_t size );
MemoryPtr MemCopy( const void *buffer, size_t size );
MemoryPtr MemMakeRef( const void *buffer, size_t size );


}	// namespace cyb