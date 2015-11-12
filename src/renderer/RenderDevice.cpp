#include "stdafx.h"
#include "RenderDevice.h"

namespace renderer
{

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