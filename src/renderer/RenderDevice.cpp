#include "stdafx.h"
#include "RenderDevice.h"

namespace renderer
{

SurfaceGeometry::SurfaceGeometry()
{
    vertexBuffer = nullptr;
    format = VertexFormat_Invalid;
    indexBuffer = nullptr;
    indexCount = 0;
    prim = Prim_Triangles;
    culling = Cull_Back;
    winding = Winding_CCW;
}

uint32_t PackRGBA(float r, float g, float b, float a)
{
    uint32_t color = ((uint32_t)(r * 255) << 24) |
                     ((uint32_t)(g * 255) << 16) |
                     ((uint32_t)(b * 255) << 8) |
                      (uint32_t)(a * 255);
    return color;
}

bool ShaderSet::SetUniform1f(const char *name, float x)
{
    const float v[] = { x };
    return SetUniformfv(name, 1, v);
}

bool ShaderSet::SetUniform2f(const char *name, float x, float y)
{
    const float v[] = { x, y };
    return SetUniformfv(name, 2, v);
}

bool ShaderSet::SetUniform3f(const char *name, float x, float y, float z)
{
    const float v[] = { x, y, z };
    return SetUniformfv(name, 3, v);
}

bool ShaderSet::SetUniform4f(const char *name, float x, float y, float z, float w)
{
    const float v[] = { x, y, z, w };
    return SetUniformfv(name, 4, v);
}

bool ShaderSet::SetUniform4fv(const char *name, const float *v)
{
    return SetUniformfv(name, 4, v);
}

bool ShaderSet::SetUniform4x4f(const char *name, const glm::mat4 &m)
{
    return SetUniformfv(name, 16, glm::value_ptr(m));
}

std::shared_ptr<Shader> RenderDevice::LoadBuiltinShader(Shader::Type stage, BuiltinShaders shader)
{
    switch (stage) {
    case Shader::Vertex:   return vertexShaders[shader];
    case Shader::Fragment: return fragmentShaders[shader];
    default: break;
    }

    return nullptr;
}

void RenderDevice::SetDefaultShader(std::shared_ptr<ShaderSet> shader)
{
    if (!shader)
        return;

    defaultShader = shader;
}

void RenderDevice::SetProjection(const glm::mat4 &proj)
{
    projection = proj;
}


} // renderer