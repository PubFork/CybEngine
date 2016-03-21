#pragma once

#include "RenderDevice.h"

namespace renderer
{

class RenderDevice_GL : public RenderDevice
{
public:
    RenderDevice_GL();
    virtual ~RenderDevice_GL();

    virtual void Clear(uint32_t targets, const glm::vec4 color, float depth = 1.0f);
    virtual void Render(const Surface *surf, const glm::mat4 &transform);

private:
    GLuint vaoId;
    GLint maxAnisotropy;
};

} // renderer