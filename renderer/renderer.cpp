#include "core/logger.h"
#include "renderer.h"

namespace cyb {

IRendererDriver *CreateRendererDriverGL();
void DestroyRendererDriverGL();

void Renderer::Init() {
	m_driver = CreateRendererDriverGL();
	m_driver->Init();

	m_vertexLayoutCache.Init();
}

void Renderer::Shutdown() {
	// Destroy all command buffers still in list
	while ( m_commandBuffers.Next() != nullptr ) {
		DestroyCommandBuffer( m_commandBuffers.Next() );
	}

	// Shutdown driver
	m_driver->Shutdown();
	DestroyRendererDriverGL();
	m_driver = nullptr;
}

CommandBuffer *Renderer::CreateCommandBuffer( size_t size ) {
	CommandBuffer *cbuf = new CommandBuffer( size );
	cbuf->m_listNode.AddToEnd( m_commandBuffers );

	// Set default clear settings
	cbuf->m_clearFlags = ClearFlags::Color | ClearFlags::Depth;
	memset( cbuf->m_clearColor, 0, sizeof( float ) * 4 );
	cbuf->m_clearDepth = 1.0f;
	cbuf->m_clearStencil = 0;

	return cbuf;
}

void Renderer::DestroyCommandBuffer( CommandBuffer *cbuf ) {
	delete cbuf;
}

VertexBufferHandle Renderer::CreateVertexBuffer( const MemoryPtr mem, const VertexLayout &layout ) {
	VertexBufferHandle bufferHandle = m_vertexBuffers.Alloc();
	if ( IsValid( bufferHandle ) ) {
		// Create vertex layout
		VertexLayoutHandle layoutHandle = m_vertexLayoutCache.Find( layout.hash );
		if ( !IsValid( layoutHandle ) ) {
			layoutHandle = m_vertexLayouts.Alloc();
			m_driver->CreateVertexLayout( layoutHandle, layout );
		}

		// Add ref to layout cache and create vertex buffer
		m_vertexLayoutCache.Add( bufferHandle, layoutHandle, layout.hash );
		m_driver->CreateVertexBuffer( bufferHandle, mem, layoutHandle );
	}

	return bufferHandle;
}

void Renderer::DestroyVertexBuffer( const VertexBufferHandle bufferHandle ) {
	if ( IsValid( bufferHandle ) ) {
		VertexLayoutHandle layoutHandle = m_vertexLayoutCache.Release( bufferHandle );
		if ( IsValid( layoutHandle ) ) {
			m_driver->DestroyVertexLayout( layoutHandle );
		}

		m_driver->DestroyVertexBuffer( bufferHandle );
		m_vertexBuffers.Free( bufferHandle.index );
	}
}

IndexBufferHandle Renderer::CreateIndexBuffer( const MemoryPtr mem ) {
	IndexBufferHandle bufferHandle = m_indexBuffers.Alloc();
	if ( IsValid( bufferHandle ) ) {
		m_driver->CreateIndexBuffer( bufferHandle, mem );
	}

	return bufferHandle;
}

void Renderer::DestroyIndexBuffer( const IndexBufferHandle bufferHandle ) {
	if ( IsValid( bufferHandle ) ) {
		m_driver->DestroyIndexBuffer( bufferHandle );
		m_indexBuffers.Free( bufferHandle.index );
	}
}

ShaderProgramHandle Renderer::CreateProgram( const MemoryPtr vertexShaderMem, const MemoryPtr fragmentShaderMem ) {
	ShaderProgramHandle handle = m_shaderPrograms.Alloc();

	if ( IsValid( handle ) ) {
		m_driver->CreateProgram( handle, vertexShaderMem, fragmentShaderMem );
	}

	return handle;
}

void Renderer::DestroyProgram( const ShaderProgramHandle programHandle ) {
	if ( IsValid( programHandle ) ) {
		m_driver->DestroyProgram( programHandle );
		m_shaderPrograms.Free( programHandle.index );
	}
}

void Renderer::UpdateViewTransform( const glm::mat4 *view, const glm::mat4 *projection ) {
	if ( view != nullptr ) {
		m_viewMatrix = *view;
	}

	if ( projection != nullptr ) {
		m_projMatrix = *projection;
	}
}

void Renderer::Frame() {
	for ( auto cbuf = m_commandBuffers.Next(); cbuf != nullptr; cbuf = cbuf->m_listNode.Next() ) {
		cbuf->m_viewMatrix = m_viewMatrix;
		cbuf->m_projMatrix = m_projMatrix;

		m_driver->Commit( cbuf );
		cbuf->Reset();
	}
}

}	// namespace cyb