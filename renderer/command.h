#pragma once
#include <memory>
#include <glm/matrix.hpp>
#include "core/config.h"
#include "core/interface.h"
#include "utils/intrusive_list.h"

namespace cyb {

struct ClearFlags {
	enum Enum {
		None    = 0x00,
		Color   = 0x01,
		Depth   = 0x02,
		Stencil = 0x04
	};
};

struct DrawCommand {
	DrawCommand();
	virtual ~DrawCommand() = default;
	void Clear();

	uint32_t numVertices;
	uint32_t startVertex;
	index_t numIndices;
	index_t startIndex;

	VertexBufferHandle vertexBuffer;
	IndexBufferHandle indexBuffer;
	ShaderProgramHandle shaderProgram;
	glm::mat4 transform;

	LinkedList<DrawCommand> m_drawNode;
};

class CommandBuffer {
public:
	CommandBuffer( const size_t cbufSize );
	virtual ~CommandBuffer() = default;

	DrawCommand *AddDrawCommand();
	void Reset();

	const LinkedList<DrawCommand> &DrawCommands() const { return m_drawList; }

	void SetClearFlags( uint32_t flags );
	void SetClearColor( float r, float g, float b, float a = 0.0f );
	void SetClearDepth( float depth );
	
	glm::mat4 m_viewMatrix;
	glm::mat4 m_projMatrix;

	uint32_t m_clearFlags;
	float m_clearColor[4];
	float m_clearDepth;
	uint8_t m_clearStencil;

	LinkedList<CommandBuffer> m_cbufNode;

private:
	LinkedList<DrawCommand> m_drawList;
	std::unique_ptr<IAllocator> m_allocator;
};

}	// namespace cyb