#pragma once
#include <cstdint>
#include <atomic>

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
	// viewDef_t *viewDef;
};

struct copyRenderCommand_t : public emptyCommand_t {
	// image_t *image;
};

/**
 * All information needed by the back end must be contained in a frameData_t.
 */
struct frameData_t {
	void Init();
	void Shutdown();
	void Reset();
	
	void *Alloc( uint32_t bytes );
	void *GetCommandBuffer( uint32_t bytes );
	void AddDrawViewCmd();

	std::atomic<uint32_t> frameMemoryAllocated;
	uint8_t *frameMemory;
	uint32_t highWaterAllocated;	// max number of bytes allocated on any frame.

	emptyCommand_t *cmdHead;
	emptyCommand_t *cmdTail;
};

