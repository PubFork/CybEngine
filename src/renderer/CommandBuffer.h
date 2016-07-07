#pragma once
#include "Base/Math/Vector.h"

enum render_command_entry_type
{
    RenderCommandEntryType_command_entry_clear,
    RenderCommandEntryType_command_entry_test1,
    RenderCommandEntryType_command_entry_test2
};

struct sort_entry
{
    uint32_t SortKey;
    uint32_t Index;
};

struct render_command_buffer
{
    uint32_t MaxPushBufferSize;
    uint32_t PushBufferSize;
    uint8_t *PushBufferBase;

    uint32_t PushBufferElementCount;
    uint32_t SortEntryAt;                   // MaxPushBufferSize - (PushBufferElementCount * sizeof(sort_entry))
};
#define RenderCommandBufferStruct(MaxPushBufferSize, PushBuffer) \
    { MaxPushBufferSize, 0, (uint8_t *)PushBuffer, 0, MaxPushBufferSize }

struct render_command_header
{
    uint16_t Type;
};

struct command_entry_clear
{
    Vec4f ClearColor;
};

struct command_entry_test1
{
    uint32_t A, B, C, D;
};

struct command_entry_test2
{
    float A, B, C, D;
};

#define PushRenderCommand(Commands, Type) (Type *)PushRenderCommand_(Commands, sizeof(Type), RenderCommandEntryType_##Type)
void *PushRenderCommand_(render_command_buffer *Commands, uint32_t CommandSize, render_command_entry_type Type);
