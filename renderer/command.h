#pragma once
#include <memory>
#include <glm/matrix.hpp>
#include "core/config.h"
#include "core/interface.h"
#include "utils/intrusive_list.h"

namespace cyb {

struct ClearSettings {
	enum Flags {
		None = 0x00,
		ClearColor = 0x01,
		ClearDepth = 0x02,
		ClearStencil = 0x04
	};

	void SetColor( float r, float g, float b, float a );
	void SetDepth( float clearValue  );
	void SetStencil( uint8_t clearValue  );
	void SetFlags( uint16_t clearFlags );

	float m_color[4];
	float m_depth;
	uint8_t m_stencil;
	uint16_t m_flags;
};

struct DrawCommand {
	DrawCommand();
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

struct CommandBuffer {
	// Construct a command buffer with allocation storage of cbufSize bytes.
	CommandBuffer( const size_t cbufSize );

	// Allocate a new DrawCommand, the command will be valid until next Reset().
	// This may return nullptr if the allocation storage is full.
	DrawCommand *AllocateDrawCommand();

	// Submit a DrawCommand to the draw list.
	// The command must be valid for atleast one frame.
	void Submit( DrawCommand *draw );

	// Resets the draw list and the allocator.
	void Reset();
	
	glm::mat4 m_viewMatrix;
	glm::mat4 m_projMatrix;

	ClearSettings m_clear;

	LinkedList<CommandBuffer> m_cbufNode;
	LinkedList<DrawCommand> m_drawList;
	std::unique_ptr<IAllocator> m_allocator;
};

}	// namespace cyb