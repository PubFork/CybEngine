#pragma once

namespace renderer
{

enum DataFormat
{
    Format_RGB_F32,
    Format_RGBA_F32,
    Format_RGBA_UI8,
    Format_RGBA_UI8Norm
};

enum InputAttribute
{
    Attrib_Position,
    Attrib_Normal,
    Attrib_Color0,
    Attrib_Color1,
    Attrib_TexCoord0,
    Attrib_TexCoord1,
    Attrib_TexCoord2,
    Attrib_TexCoord3,
    Attrib_Count
};

struct InputElement
{
    InputAttribute attribute;
    DataFormat format;
    uintptr_t alignedOffset;
};

inline uint32_t AlignedDataFormatSize(const DataFormat format)
{
    static const uint32_t formatByteSize[] = {
        12,             // Format_RGB_F32
        16,             // Format_RGBA_F32
        4,              // Format_RGBA_UI8
        4               // Format_RGBA_UI8Norm
    };

    return formatByteSize[format];
}

struct InputLayout
{
    const InputElement *elements;
    uint32_t numElements;

    uint32_t GetStride() const
    {
        uint32_t stride = 0;

        for (uint32_t i = 0; i < numElements; i++)
            stride += AlignedDataFormatSize(elements[i].format);

        return stride;
    }

    bool IsValid() const
    {
        if (!elements || !numElements)
            return false;

        uintptr_t previousOffset = 0;
        for (uint32_t i = 0; i < numElements; i++) {
            if (elements[i].alignedOffset < previousOffset)
                return false;

            previousOffset = elements[i].alignedOffset;
        }

        return true;
    }
};

} // renderer