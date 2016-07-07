#include "Precompiled.h"
#include "Renderer/Backend.h"
#include "Base/Sys.h"
#include "Base/Debug.h"

void ExecuteCommandBuffer(render_command_buffer *Commands)
{
    uint32_t SortEntryCount = Commands->PushBufferElementCount;
    sort_entry *SortEntry = (sort_entry *)(Commands->PushBufferBase + Commands->SortEntryAt);

    for (uint32_t SortEntryIndex = 0; SortEntryIndex < SortEntryCount; ++SortEntryIndex, ++SortEntry)
    {
        render_command_header *Header = (render_command_header *)(Commands->PushBufferBase + SortEntry->Index);
        void *Data = (uint8_t *)Header + sizeof(*Header);

        DebugPrintf("Poll: Header:%x Result:%x\n", Header, Data);

        switch (Header->Type)
        {
        case RenderCommandEntryType_command_entry_clear:
        {
            command_entry_clear *Entry = (command_entry_clear *)Data;
            Sys_Sleep(1);
        } break;

        case RenderCommandEntryType_command_entry_test1:
        {
            command_entry_test1 *Entry = (command_entry_test1 *)Data;
            Sys_Sleep(1);
        } break;
        
        case RenderCommandEntryType_command_entry_test2:
        {
            command_entry_test2 *Entry = (command_entry_test2 *)Data;
            Sys_Sleep(1);
        } break;

        default:
            assert(!"Invalid type");
        }
    }
}