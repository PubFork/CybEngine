#pragma once
#include <cstdint>
#include <atomic>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include "screen_rect.h"
#include "buffer_object.h"

struct viewEntity_t {
	glm::mat4 modelMatrix;
	glm::mat4 modelViewMatrix;
	glm::mat4 modelViewProjectionMatrix;
};

struct drawSurf_t {
	void Create( const void *vertexData, uint32_t vertexDataSize, const void *indexData, uint32_t indexDataSize );
	void Destroy();

	vertexBuffer_t vertexBuffer;
	uint32_t numVertices;

	indexBuffer_t indexBuffer;
	uint32_t numIndices;

	viewEntity_t *space;
};

struct renderView_t {
	float fovX;
	float fovY;

	glm::vec3 viewPosition;
	glm::vec3 viewTarget;
};

/** 
 * viewDefs are allocated on the frame data memory stack.
 * This is the main wy of sending rendering information from
 * the front end to the back end.
 */
struct viewDef_t {
	void SetupViewMatrix();
	void SetupProjectionMatrix();

	void Render();

	renderView_t renderView;
	glm::mat4 projectionMatrix;
	viewEntity_t worldSpace;

	screenRect_t viewport;
	screenRect_t scissor;

	drawSurf_t **drawSurfs;
	uint32_t numDrawSurfs;
};


/* 
 * =================================================== 
 * Backend render command queue
 * =================================================== 
 */

enum class renderCommand_t {
	Nop,
	DrawSurface,
	CopyRender,
	PostProcess
};

struct emptyCommand_t {
	renderCommand_t commandId;
	renderCommand_t *next;
};

struct drawSurfsCommand_t : public emptyCommand_t {
	viewDef_t *viewDef;
};

struct copyRenderCommand_t : public emptyCommand_t {
	// image_t *image;
};

//=======================================================================

/**
 * All information needed by the back end must be contained in a frameData_t.
 */
struct frameData_t {
	void Create();
	void Destroy();
	void Reset();
	
	void *FrameAlloc( uint32_t bytes );
	void *GetCommandBuffer( uint32_t bytes );
	void AddDrawViewCmd( viewDef_t *params );

	std::atomic<uint32_t> frameMemoryAllocated;
	uint8_t *frameMemory;
	uint32_t highWaterAllocated;	// max number of bytes allocated on any frame.

	emptyCommand_t *cmdHead;
	emptyCommand_t *cmdTail;
};

