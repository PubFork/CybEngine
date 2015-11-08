#pragma once

#include "Shader.h"
#include "Surface.h"

namespace renderer
{

enum BuiltinShaders
{
    VShader_MV      = 0,
    VShader_MVP     ,
    VShader_Count   ,

    FShader_Solid   = 0,
    FShader_Gouraud ,
    FShader_Count
};

class RenderDevice
{
public:
    enum FillMode
    {
        Fill_Solid,
        Fill_Wire,
        Fill_Point
    };

    RenderDevice();
    ~RenderDevice();

    std::shared_ptr<Buffer> CreateBuffer(BufferUsage usage, const void *buffer, size_t bufSize);
    std::shared_ptr<ShaderSet> CreateShaderSet(std::initializer_list<std::shared_ptr<Shader>> shaderList = {});
    std::shared_ptr<Shader> LoadBuiltinShader(ShaderStage stage, BuiltinShaders shader);

    void SetProjection(const glm::mat4 &proj);
    void SetFillMode(FillMode mode);

    void Clear(float r, float g, float b, float a, float depth, bool clearColor = true, bool clearDepth = true);
    void Render(const Surface *surf, const glm::mat4 &transform);

private:
    GLuint vaoId;
    glm::mat4 projection;
    std::shared_ptr<Shader> vertexShaders[VShader_Count];
    std::shared_ptr<Shader> fragmentShaders[FShader_Count];
};

} // renderer