#pragma once
#include <cassert>
#include <malloc.h>	// _aligned_malloc & _aligned_free
#include "core/interface.h"

namespace cyb {

static const size_t MEMORY_ALIGNMENT = static_cast<size_t>( 2 * sizeof( uintptr_t ) );

class CrtAllocator : public IAllocator {
public:
	explicit CrtAllocator() = default;
	virtual ~CrtAllocator() = default;

	virtual void *Allocate( size_t size ) final {
		return _aligned_malloc( size, MEMORY_ALIGNMENT );
	}

	virtual void Free( void *ptr ) final {
		_aligned_free( ptr );
	}

	virtual void Reset() {}
};

class LinearAllocator : public IAllocator {
public:
	explicit LinearAllocator( size_t size ) {
		m_size = size;
		m_start = new uint8_t[m_size];
		m_usagePeak = 0;
		Reset();
	}

	virtual ~LinearAllocator() override {
		delete[] m_start;
	}

	virtual void *Allocate( size_t size ) final {
		assert( size != 0 );
		size_t adjustment = 0;	// CYB_ALIGN_OFFSET( m_current );
		if ( ( m_current + size + adjustment ) > ( m_start + m_size ) ) {
			return nullptr;		// Out of memory
		}

		uint8_t *alignedMemory = m_current + adjustment;
		m_current = alignedMemory + size;
		m_usedMemory += size + adjustment;
		m_usagePeak = ( m_usedMemory > m_usagePeak ) ? m_usedMemory : m_usagePeak;
		m_allocations++;

		return static_cast<void *>( alignedMemory );
	}

	virtual void Free( void *ptr ) final {
		// Don't free memory from linear allocator, use Reset() 
	}

	virtual void Reset() final {
		m_current = m_start;
		m_usedMemory = 0;
		m_allocations = 0;
	}

private:
	uint8_t *m_start;
	uint8_t *m_current;
	size_t m_size;
	size_t m_usedMemory;
	size_t m_usagePeak;
	uint32_t m_allocations;
};

template <size_t m_size>
class LinearAllocatorT : public LinearAllocator {
public:
	explicit LinearAllocatorT() : LinearAllocator( m_size ) {}
};


}	// namespace cyb