#include "stdafx.h"
#include "InputLayout.h"

namespace renderer
{

uint32_t AlignedDataFormatSize(const DataFormat format)
{
    static const std::unordered_map<DataFormat, uint32_t> formatByteSize = {
        { Format_RGB_F32,     12 },
        { Format_RGBA_F32,    16 },
        { Format_RGBA_UI8,     4 },
        { Format_RGBA_UI8Norm, 4 }
    };

    return formatByteSize.at(format);
}

uint32_t InputLayout::GetStride() const
{
    uint32_t stride = 0;

    for (uint32_t i = 0; i < numElements; i++)
        stride += AlignedDataFormatSize(elements[i].format);

    return stride;
}

bool InputLayout::IsValid() const
{
    // validate element params
    if (!elements || !numElements)
        return false;

    // validate offsets
    uintptr_t previousOffset = 0;
    for (uint32_t i = 0; i < numElements; i++) {
        if (elements[i].alignedOffset < previousOffset)
            return false;

        previousOffset = elements[i].alignedOffset;
    }

    return true;
}

} // renderer