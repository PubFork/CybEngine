#pragma once

#include "RenderDevice.h"

namespace renderer
{

class Buffer_GL : public Buffer
{
public:
    Buffer_GL();
    virtual ~Buffer_GL();
    virtual bool SetData(Type usage, const void *buffer, size_t bufSize);

    GLuint bufferId;
    GLenum target;
    GLsizeiptr size;
};

class Shader_GL : public Shader
{
public:
    Shader_GL(Type type, const char *source);
    virtual ~Shader_GL();

    GLenum GLStage() const;
    bool Compile(const char *source);

    GLuint shaderId;
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
    virtual void UnsetShader(Shader::Type stage);
    virtual bool SetUniformfv(const char *name, uint32_t numFloats, const float *v);

    bool Link();

    GLuint progId;
    std::shared_ptr<Shader> shaders[Shader::Count];
    std::list<Uniform> uniformInfo;
};

class RenderDevice_GL : public RenderDevice
{
public:
    RenderDevice_GL();
    virtual ~RenderDevice_GL();

    virtual std::shared_ptr<Buffer> CreateBuffer(Buffer::Type usage, const void *buf, size_t bufSize);
    virtual std::shared_ptr<ShaderSet> CreateShaderSet(std::initializer_list<std::shared_ptr<Shader>> shaderList = {});

    virtual void SetFillMode(FillMode mode);
    virtual void Clear(int32_t flags, uint32_t color);
    virtual void Render(const Surface *surf, const glm::mat4 &transform);

private:
    GLuint vaoId;
};

} // renderer