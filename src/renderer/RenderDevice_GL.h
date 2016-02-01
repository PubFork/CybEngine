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

    virtual void Clear(int32_t flags, uint32_t color);
    virtual void Render(const Surface *surf, const glm::mat4 &transform, PipelineState &pstate);

private:
    GLuint vaoId;
    GLint maxAnisotropy;
};

} // renderer