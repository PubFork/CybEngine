#pragma once

struct render_command_buffer;

struct game_memory
{
    uint64_t PermanentStorageSize;
    void *PermanentStorage;

    uint64_t TransientStorageSize;
    void *TransientStorage;
};

struct game_entry_params
{
    uint64_t PermanentStorageSize;
    uint64_t TransientStorageSize;
    uint32_t RenderCommandStorageSize;

    bool(*InitializeCallback)(game_memory *Memory);
    void(*UpdateCallback)(game_memory *Memory);                   // TODO: pass game input
    void(*RenderCallback)(render_command_buffer *RenderCommands);
};

int GameEntryFunction(const game_entry_params *Params);