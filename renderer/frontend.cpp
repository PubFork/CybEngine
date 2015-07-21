#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include "core/memory.h"
#include "core/logger.h"
#include "frontend.h"

void viewDef_t::SetupViewMatrix() {
	// viewDef's world space is an identity matrix
	worldSpace.modelMatrix = glm::mat4( 1 );

	// setup view matrix looking down negative z, and positve y pointing upwards
	glm::vec3 up( 0.0f, 1.0f, 0.0f );
	worldSpace.modelViewMatrix = glm::lookAt( renderView.viewPosition, renderView.viewTarget, up );
}

void viewDef_t::SetupProjectionMatrix() {
	projectionMatrix = glm::perspective( renderView.fovY, (float)viewport.GetWidth() / (float)viewport.GetHeight(), viewport.zMin, viewport.zMax );
}

void viewDef_t::Render() {
	SetupViewMatrix();
	SetupProjectionMatrix();
	worldSpace.modelViewProjectionMatrix = projectionMatrix * worldSpace.modelViewMatrix;

	/* ... */
	// 
}

//=======================================================================

static const uint16_t NUM_FRAME_DATA = 2;
static const uint16_t FRAME_ALLOC_ALIGNMENT = 64U;
static const uint32_t MAX_FRAME_MEMORY = MEGABYTES( 32 );

/** Initialize and reset the frame data. */
void frameData_t::Create() {
	frameMemory = (uint8_t *)Mem_Alloc16( MAX_FRAME_MEMORY );
	Reset();
}

/** Destroy frame data, all used memory will be freed. */
void frameData_t::Destroy() {
	Mem_Free16( frameMemory );
	frameMemory = nullptr;
}

/**
 * Resets the frame allocator and clears the command chain,
 * leaving it with only a Nop command.
 */
void frameData_t::Reset() {
	// update highwater mark
	highWaterAllocated = std::max( highWaterAllocated, frameMemoryAllocated.load() );

	// reset memory allocations
	const uint32_t bytesNeededForAlignment = FRAME_ALLOC_ALIGNMENT - ( (uint32_t)frameMemory & ( FRAME_ALLOC_ALIGNMENT - 1 ) );
	frameMemoryAllocated = bytesNeededForAlignment;

	// clear the command chain and make a Nop command the only thing on the list
	cmdHead = (emptyCommand_t *)FrameAlloc( sizeof( *cmdHead ) );
	cmdHead->commandId = renderCommand_t::Nop;
	cmdHead->next = nullptr;
	cmdTail = cmdHead;
}

/**
 * Allocated memory will be automatically freed when the
 * current frame's back end completes.
 * This should only be used by the front end.
 * All memory is cache-line-cleared for the best performance.
 * @param	bytes	Number of bytes to allocate.
 */
void *frameData_t::FrameAlloc( uint32_t bytes ) {
	bytes = ( bytes + FRAME_ALLOC_ALIGNMENT - 1 ) &~( FRAME_ALLOC_ALIGNMENT - 1 );

	frameMemoryAllocated += bytes;
	CYB_FATAL( frameMemoryAllocated < MAX_FRAME_MEMORY, "Out of frame memory" );

	uint8_t *ptr = frameMemory + frameMemoryAllocated - bytes;

	// cache line clear the memory
	for ( uint32_t offset = 0; offset < bytes; offset += CACHE_LINE_SIZE ) {
		ZeroCacheLine( ptr, offset );
	}

	return ptr;
}

/**
 * Returns frame allocated memory for a command buffer, and
 * adds links it to the end of the current command chain.
 * @param	bytes	Size of the desired command buffer.
 */
void *frameData_t::GetCommandBuffer( uint32_t bytes ) {
	emptyCommand_t *cmd = (emptyCommand_t *)FrameAlloc( bytes );
	cmd->next = nullptr;
	cmdTail->next = &cmd->commandId;
	cmdTail = cmd;

	return (void *)cmd;
}

/** Add a draw view command to the command chain */
void frameData_t::AddDrawViewCmd( viewDef_t *params ) {
	drawSurfsCommand_t *cmd = (drawSurfsCommand_t *)GetCommandBuffer( sizeof( *cmd ) );
	cmd->commandId = renderCommand_t::DrawSurface;
	cmd->viewDef = params;
}
