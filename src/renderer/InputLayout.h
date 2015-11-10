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
    uintptr_t alignedOffset;             // optional, set to 0 to auto calculate 
};

struct InputLayout
{
    const InputElement *elements;
    uint32_t numElements;

    uint32_t GetStride() const;
    bool IsValid() const;
};

} // renderer