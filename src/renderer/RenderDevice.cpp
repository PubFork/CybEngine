#include "stdafx.h"
#include "RenderDevice.h"

namespace renderer
{

uint32_t AlignedDataFormatSize(const DataFormat format)
{
    static const uint32_t formatByteSize[] = {
        12,             // Format_RGB_F32
        16,             // Format_RGBA_F32
        4,              // Format_RGBA_UI8
        4               // Format_RGBA_UI8Norm
    };

    return formatByteSize[format];
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

bool ShaderSet::SetUniform1f(const char *name, float x)
{
    const float v[] = { x };
    return SetUniform(name, 1, v);
}

bool ShaderSet::SetUniform2f(const char *name, float x, float y)
{
    const float v[] = { x, y };
    return SetUniform(name, 2, v);
}

bool ShaderSet::SetUniform3f(const char *name, float x, float y, float z)
{
    const float v[] = { x, y, z };
    return SetUniform(name, 3, v);
}

bool ShaderSet::SetUniform4f(const char *name, float x, float y, float z, float w)
{
    const float v[] = { x, y, z, w };
    return SetUniform(name, 4, v);
}

bool ShaderSet::SetUniform4fv(const char *name, const float *v)
{
    return SetUniform(name, 4, v);
}

bool ShaderSet::SetUniform4x4f(const char *name, const glm::mat4 &m)
{
    return SetUniform(name, 16, glm::value_ptr(m));
}

std::shared_ptr<Shader> RenderDevice::LoadBuiltinShader(ShaderStage stage, BuiltinShaders shader)
{
    switch (stage) {
    case Shader_Vertex:   return vertexShaders[shader];
    case Shader_Fragment: return fragmentShaders[shader];
    default: break;
    }

    return nullptr;
}

void RenderDevice::SetProjection(const glm::mat4 &proj)
{
    projection = proj;
}


} // renderer