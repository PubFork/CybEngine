#pragma once

#include <glm/mat4x4.hpp>

namespace renderer
{

struct Surface;

enum BufferUsage
{
    Buffer_Invalid,
    Buffer_Vertex,
    Buffer_Index,
    Buffer_Uniform,
    Buffer_Compute
};

class Buffer
{
public:
    virtual ~Buffer() = default;

    virtual bool SetData(BufferUsage usage, const void *buffer, size_t bufSize) = 0;
};

enum ShaderStage
{
    Shader_Vertex,
    Shader_Geometry,
    Shader_Fragment,
    Shader_Compute,
    Shader_Count
};

class Shader
{
public:
    Shader(ShaderStage s) : stage(s) {}
    virtual ~Shader() = default;

    ShaderStage GetStage() const { return stage; }
    virtual bool Compile(const char *source) = 0;

protected:
    ShaderStage stage;
};

class ShaderSet
{
public:
    ShaderSet() = default;
    virtual ~ShaderSet() = default;

    virtual void SetShader(std::shared_ptr<Shader> s) = 0;
    virtual void UnsetShader(ShaderStage stage) = 0;
    virtual bool SetUniform(const char *name, uint32_t numFloats, const float *v) = 0;
    
    bool SetUniform1f(const char *name, float x);
    bool SetUniform2f(const char *name, float x, float y);
    bool SetUniform3f(const char *name, float x, float y, float z);
    bool SetUniform4f(const char *name, float x, float y, float z, float w = 1.0f);
    bool SetUniform4fv(const char *name, const float *v);
    bool SetUniform4x4f(const char *name, const glm::mat4 &m);
};

enum BuiltinShaders
{
    VShader_MV = 0,
    VShader_MVP,
    VShader_Count,

    FShader_Solid = 0,
    FShader_Gouraud,
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

    RenderDevice() = default;
    virtual ~RenderDevice() = default;

    void SetProjection(const glm::mat4 &proj);
    void SetFillMode(FillMode mode);
    std::shared_ptr<Shader> LoadBuiltinShader(ShaderStage stage, BuiltinShaders shader);

    virtual std::shared_ptr<Buffer> CreateBuffer(BufferUsage usage, const void *buf, size_t bufSize) = 0;
    virtual std::shared_ptr<ShaderSet> CreateShaderSet(std::initializer_list<std::shared_ptr<Shader>> shaderList = {}) = 0;
    
    virtual void Clear(float r, float g, float b, float a, float depth, bool clearColor = true, bool clearDepth = true) = 0;
    virtual void Render(const Surface *surf, const glm::mat4 &transform) = 0;

protected:
    glm::mat4 projection;
    std::shared_ptr<Shader> vertexShaders[VShader_Count];
    std::shared_ptr<Shader> fragmentShaders[FShader_Count];
};

} // renderer