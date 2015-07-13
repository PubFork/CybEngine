#pragma once
#include <cstdint>

namespace cyb {

static const uint32_t INVALID_HANDLE = UINT32_MAX;

#define CREATE_HANDLE( name ) \
	struct name { \
		uint32_t index; \
		explicit name( uint32_t value = INVALID_HANDLE ) { index = value; } \
	}; \
	inline bool IsValid( const name handle ) { return handle.index != INVALID_HANDLE; }

CREATE_HANDLE( VertexBufferHandle );
CREATE_HANDLE( IndexBufferHandle );
CREATE_HANDLE( VertexLayoutHandle );
CREATE_HANDLE( ShaderProgramHandle );
CREATE_HANDLE( MatrixHandle );

#undef CREATE_HANDLE

template <uint32_t maxHandles>
class HandleAllocator {
public:
	explicit HandleAllocator() {
		m_numHandles = 0;

		for ( uint32_t i = 0; i < maxHandles; i++ ) {
			m_handles[i] = i;
		}
	}

	~HandleAllocator() = default;

	uint32_t Alloc() {
		if ( m_numHandles < maxHandles ) {
			uint32_t index = m_numHandles++;
			uint32_t handle = m_handles[index];
			uint32_t *sparse = &m_handles[maxHandles];
			sparse[handle] = index;

			return handle;
		}

		return INVALID_HANDLE;
	}

	void Free( uint32_t handle ) {
		uint32_t *sparse = &m_handles[maxHandles];
		uint32_t index = sparse[handle];
		--m_numHandles;
		uint32_t temp = m_handles[m_numHandles];
		m_handles[m_numHandles] = handle;
		sparse[temp] = index;
		m_handles[index] = temp;
	}

private:
	uint32_t m_handles[maxHandles * 2];
	uint32_t m_numHandles;
};

}	// namespace cyb