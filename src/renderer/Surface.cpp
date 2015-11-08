#include "stdafx.h"
#include "Surface.h"

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

Buffer::Buffer() :
    bufferId(0),
    size(0),
    use(0)
{
}

Buffer::~Buffer()
{
    if (bufferId)
        glDeleteBuffers(1, &bufferId);
}

bool Buffer::Data(BufferUsage usage, const void *buffer, size_t bufSize)
{
    switch (usage) {
    case Buffer_Index: use = GL_ELEMENT_ARRAY_BUFFER; break;
    default:           use = GL_ARRAY_BUFFER; break;
    }

    if (!bufferId)
        glCreateBuffers(1, &bufferId);
    size = bufSize;

    glBindBuffer(use, bufferId);
    glBufferData(use, size, buffer, GL_STATIC_DRAW);
    return true;
}

} // renderer