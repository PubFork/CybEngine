#include "core/logger.h"
#include "utils/allocator.h"
#include "command.h"

namespace cyb {

DrawCommand::DrawCommand() {
	listNode.SetOwner( this );
}

void DrawCommand::Clear() {
	numVertices   = UINT32_MAX;
	startVertex   = 0;
	numIndices    = INDEX_MAX;
	startIndex    = 0;
	vertexBuffer  = INVALID_HANDLE;
	indexBuffer   = INVALID_HANDLE;
	shaderProgram = INVALID_HANDLE;
	listNode.Clear();
}

CommandBuffer::CommandBuffer( const size_t cbufSize ) {
	m_allocator = std::make_unique<LinearAllocator>( cbufSize );
	m_listNode.SetOwner( this );
}

DrawCommand *CommandBuffer::AddDrawCommand() {
	DrawCommand *draw = m_allocator->AllocateT<DrawCommand>();
	CYB_CHECK( draw != nullptr, "Out of memory" );
	draw->Clear();
	draw->listNode.AddToEnd( m_drawCommands );

	return draw;
}

void CommandBuffer::Reset() {
	m_drawCommands.Clear();
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