#pragma once

#include "RenderDevice.h"

namespace renderer
{

class Buffer_GL : public Buffer
{
public:
    Buffer_GL();
    virtual ~Buffer_GL() final;
    virtual bool SetData(BufferUsage usage, const void *buffer, size_t bufSize);

    GLuint bufferId;
    GLenum target;
    GLsizeiptr size;
};

class Shader_GL : public Shader
{
public:
    Shader_GL(ShaderStage type, const char *source);
    virtual ~Shader_GL();

    GLenum GLStage() const;
    virtual bool Compile(const char *source);

    GLuint shaderId;
    ShaderStage stage;
};

class ShaderSet_GL : public ShaderSet
{
public:
    struct Uniform
    {
        std::string name;
        GLint location;
        uint32_t numFloats;
    };

    ShaderSet_GL();
    virtual ~ShaderSet_GL();

    virtual void SetShader(std::shared_ptr<Shader> s);
    virtual void UnsetShader(ShaderStage stage);
    virtual bool SetUniform(const char *name, uint32_t numFloats, const float *v);

    bool Link();

    GLuint progId;
    std::shared_ptr<Shader> shaders[Shader_Count];
    std::vector<Uniform> uniformInfo;
    std::vector<int32_t> attribLocations;
};

class RenderDevice_GL : public RenderDevice
{
public:
    RenderDevice_GL();
    virtual ~RenderDevice_GL();

    virtual std::shared_ptr<Buffer> CreateBuffer(BufferUsage usage, const void *buf, size_t bufSize);
    virtual std::shared_ptr<ShaderSet> CreateShaderSet(std::initializer_list<std::shared_ptr<Shader>> shaderList = {});

    virtual void Clear(float r, float g, float b, float a, float depth, bool clearColor = true, bool clearDepth = true);
    virtual void Render(const Surface *surf, const glm::mat4 &transform);

private:
    GLuint vaoId;
};

} // renderer