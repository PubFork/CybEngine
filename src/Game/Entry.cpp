#include "Precompiled.h"
#include "Entry.h"
#include "Base/Memory.h"
#include "Base/Sys.h"

#include "Renderer/CommandBuffer.h"
#include "Renderer/Backend.h"

int GameEntryFunction(const game_entry_params *Params)
{
    game_memory GameMemory = {};
    GameMemory.PermanentStorageSize = Params->PermanentStorageSize;
    GameMemory.TransientStorageSize = Params->TransientStorageSize;

    uint64_t GameMemoryBlockSize = GameMemory.PermanentStorageSize +
                                   GameMemory.TransientStorageSize;
    void *GameMemoryBlock = Sys_Alloc(GameMemoryBlockSize);
    
    GameMemory.PermanentStorage = GameMemoryBlock;
    GameMemory.TransientStorage = ((uint8_t *)GameMemory.PermanentStorage + GameMemory.PermanentStorageSize);

    uint32_t RenderCommandStorageSize = Params->RenderCommandStorageSize;
    void *RenderCommandStorage = Sys_Alloc(RenderCommandStorageSize);

    // Render
    if (Params->RenderCallback)
    {
        render_command_buffer RenderCommands = RenderCommandBufferStruct(RenderCommandStorageSize, RenderCommandStorage);
        Params->RenderCallback(&RenderCommands);

        ExecuteCommandBuffer(&RenderCommands);
        Sys_Sleep(1);
    }

    Sys_Free(RenderCommandStorage);
    Sys_Free(GameMemoryBlock);
    return EXIT_SUCCESS;
}