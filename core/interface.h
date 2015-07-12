#pragma once
#include "core/handles.h"
#include "core/memory.h"

namespace cyb {

//=========== Forward declarations...

struct VertexLayout;
class CommandBuffer;

//=========== Interface classes

class IAllocator {
public:
	virtual ~IAllocator() {}
	virtual void *Allocate( size_t size ) = 0;
	virtual void Free( void *ptr ) = 0;
	virtual void Reset() = 0; 

	// Allocates a T and invokes it's constructor
	template <class T, class... Args>
	T *AllocateT( Args &&...args ) {
		T *object = static_cast<T*>( Allocate( sizeof( T ) ) );
		if ( object != nullptr ) {
			new (object) T( std::forward<Args>( args )... );
		}

		return object;
	}

	// Invokes T's destructor and frees the object
	template <class T>
	void FreeT( T *object ) {
		if ( object != nullptr ) {
			object~( );
		}

		Free( object );
	}
};

struct IRendererDriver {
	virtual void Init() = 0;
	virtual void Shutdown() = 0;

	virtual void CreateVertexBuffer( const VertexBufferHandle handle, const MemoryPtr mem, const VertexLayoutHandle layoutHandle ) = 0;
	virtual void DestroyVertexBuffer( const VertexBufferHandle handle ) = 0;
	virtual void CreateIndexBuffer( const IndexBufferHandle handle, const MemoryPtr mem ) = 0;
	virtual void DestroyIndexBuffer( const IndexBufferHandle handle ) = 0;
	virtual void CreateVertexLayout( const VertexLayoutHandle handle, const VertexLayout &layout ) = 0;
	virtual void DestroyVertexLayout( const VertexLayoutHandle handle ) = 0;
	virtual void CreateProgram( const ShaderProgramHandle handle, const MemoryPtr vertexShaderMem, const MemoryPtr fragmentShaderMem ) = 0;
	virtual void DestroyProgram( const ShaderProgramHandle handle ) = 0;

	virtual void Commit( const CommandBuffer *cbuf ) = 0;
};

};