#pragma once
#include <glm/mat4x4.hpp>
#include "core/config.h"
#include "core/interface.h"
#include "utils/LinkedList.h"
#include "vertex_layout.h"
#include "command.h"

namespace cyb {

class Renderer {
public:
	Renderer() : m_driver( nullptr ) {}
	~Renderer() = default;

	void Init();
	void Shutdown();
	bool IsInitialized() const { return m_driver != nullptr; }

	CommandBuffer *CreateCommandBuffer( size_t size );
	void DestroyCommandBuffer( CommandBuffer *cbuf );

	VertexBufferHandle CreateVertexBuffer( const std::shared_ptr<memory_t> mem, const VertexLayout &layout );
	void DestroyVertexBuffer( const VertexBufferHandle bufferHandle );
	IndexBufferHandle CreateIndexBuffer( const std::shared_ptr<memory_t> mem );
	void DestroyIndexBuffer( const IndexBufferHandle bufferHandle );
	ShaderProgramHandle CreateProgram( const std::shared_ptr<memory_t> vertexShaderMem, const std::shared_ptr<memory_t> fragmentShaderMem );
	void DestroyProgram( const ShaderProgramHandle programHandle );

	void UpdateViewTransform( const glm::mat4 *view, const glm::mat4 *projection );
	void Frame();

private:
	LinkedList<CommandBuffer> m_cbufList;

	glm::mat4 m_viewMatrix;
	glm::mat4 m_projMatrix;

	HandleAllocator<MAX_VERTEX_BUFFERS> m_vertexBuffers;
	HandleAllocator<MAX_INDEX_BUFFERS> m_indexBuffers;
	HandleAllocator<MAX_VERTEX_LAYOUTS> m_vertexLayouts;
	HandleAllocator<MAX_SHADER_PROGRAMS> m_shaderPrograms;

	VertexLayoutCache m_vertexLayoutCache;
	
	IRendererDriver *m_driver;
};

}	// namespace cyb