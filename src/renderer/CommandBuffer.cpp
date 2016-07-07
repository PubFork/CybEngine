#include "Precompiled.h"
#include "Renderer/CommandBuffer.h"
#include "Base/Debug.h"

void *PushRenderCommand_(render_command_buffer *Commands, uint32_t Size, const render_command_entry_type Type)
{
    void *Result = NULL;
    Size += sizeof(render_command_header);
    
    if ((Commands->PushBufferSize + Size) < (Commands->SortEntryAt - sizeof(sort_entry)))
    {
        render_command_header *Header = (render_command_header *)(Commands->PushBufferBase + Commands->PushBufferSize);
        Header->Type = (uint16_t)Type;

        Result = (uint8_t *)Header + sizeof(*Header);
        DebugPrintf("Push: Header:%x Result:%x\n", Header, Result);

        Commands->SortEntryAt -= sizeof(sort_entry);
        sort_entry *Entry = (sort_entry *)(Commands->PushBufferBase + Commands->SortEntryAt);
        Entry->SortKey = 0;     // TODO: Add sort key handling
        Entry->Index = Commands->PushBufferSize;

        Commands->PushBufferSize += Size;
        ++Commands->PushBufferElementCount;
    }
    else
    {
        assert(!"Out of command buffer memory");
    }

    return Result;
}