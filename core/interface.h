#pragma once
#include "core/handles.h"
#include "core/memory.h"

namespace cyb {

//=========== Forward declarations...

struct VertexLayout;
struct CommandBuffer;

struct IRendererDriver {
	virtual void Init() = 0;
	virtual void Shutdown() = 0;

	virtual void CreateVertexBuffer( const VertexBufferHandle handle, const std::shared_ptr<memory_t> mem, const VertexLayoutHandle layoutHandle ) = 0;
	virtual void DestroyVertexBuffer( const VertexBufferHandle handle ) = 0;
	virtual void CreateIndexBuffer( const IndexBufferHandle handle, const std::shared_ptr<memory_t> mem ) = 0;
	virtual void DestroyIndexBuffer( const IndexBufferHandle handle ) = 0;
	virtual void CreateVertexLayout( const VertexLayoutHandle handle, const VertexLayout &layout ) = 0;
	virtual void DestroyVertexLayout( const VertexLayoutHandle handle ) = 0;
	virtual void CreateProgram( const ShaderProgramHandle handle, const std::shared_ptr<memory_t> vertexShaderMem, const std::shared_ptr<memory_t> fragmentShaderMem ) = 0;
	virtual void DestroyProgram( const ShaderProgramHandle handle ) = 0;

	virtual void Commit( const CommandBuffer *cbuf ) = 0;
};

};