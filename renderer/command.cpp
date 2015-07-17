#include "core/logger.h"
#include "utils/allocator.h"
#include "command.h"

namespace cyb {

DrawCommand::DrawCommand() {
	m_drawNode.SetOwner( this );
}

void DrawCommand::Clear() {
	numVertices         = UINT32_MAX;
	startVertex         = 0;
	numIndices          = INDEX_MAX;
	startIndex          = 0;
	vertexBuffer.index  = INVALID_HANDLE;
	indexBuffer.index   = INVALID_HANDLE;
	shaderProgram.index = INVALID_HANDLE;
}

CommandBuffer::CommandBuffer( const size_t cbufSize ) {
	m_cbufNode.SetOwner( this );
	m_allocator = std::make_unique<LinearAllocator>( cbufSize );
}

DrawCommand *CommandBuffer::AddDrawCommand() {
	DrawCommand *draw = m_allocator->AllocateT<DrawCommand>();
	CYB_CHECK( draw != nullptr, "Out of memory" );
	draw->Clear();
	m_drawList.PushBack( draw->m_drawNode );

	return draw;
}

void CommandBuffer::Reset() {
	m_drawList.Clear();
	m_allocator->Reset();
}

void CommandBuffer::SetClearFlags( uint32_t flags ) {
	m_clearFlags = flags;
}

void CommandBuffer::SetClearColor( float r, float g, float b, float a ) {
	m_clearColor[0] = r;
	m_clearColor[1] = g;
	m_clearColor[2] = b;
	m_clearColor[3] = a;
}

void CommandBuffer::SetClearDepth( float depth ) {
	m_clearDepth = depth;
}

}	// namespace cyb