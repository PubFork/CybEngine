#include "core/logger.h"
#include "core/memory.h"
#include "command.h"

namespace cyb {

void ClearSettings::SetColor( float r, float g, float b, float a ) {
	m_color[0] = r;
	m_color[1] = g;
	m_color[2] = b;
	m_color[3] = a;
}

void ClearSettings::SetDepth( float clearValue ) {
	m_depth = clearValue;
}

void ClearSettings::SetStencil( uint8_t clearValue ) {
	m_stencil = clearValue;
}

void ClearSettings::SetFlags( uint16_t clearFlags ) {
	m_flags = clearFlags;
}

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
	m_allocator = std::make_shared<LinearAllocator>( cbufSize );
}

DrawCommand *CommandBuffer::AllocateDrawCommand() {
	DrawCommand *draw = new (m_allocator.get()) DrawCommand();
	if ( draw != nullptr ) {
		draw->Clear();
	}

	return draw;
}

void CommandBuffer::Submit( DrawCommand *draw ) {
	if ( draw != nullptr ) {
		m_drawList.PushBack( draw->m_drawNode );
	}
}

void CommandBuffer::Reset() {
	m_drawList.Clear();
	m_allocator->Flush();
}

}	// namespace cyb