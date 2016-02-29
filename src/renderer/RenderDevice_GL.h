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

class RenderDevice_GL : public RenderDevice
{
public:
    RenderDevice_GL();
    virtual ~RenderDevice_GL();

    virtual std::shared_ptr<Buffer>    CreateBuffer(Buffer::Type usage, const void *buf, size_t bufSize);

    virtual void Clear(uint32_t targets, const glm::vec4 color, float depth = 1.0f);
    virtual void Render(const Surface *surf, const glm::mat4 &transform);

private:
    GLuint vaoId;
    GLint maxAnisotropy;
};

} // renderer