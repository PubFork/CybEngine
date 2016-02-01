#include "stdafx.h"
#include "RenderDevice.h"

#include "Base/Debug.h"
#include "Base/FileUtils.h"

namespace renderer
{

SurfaceGeometry::SurfaceGeometry()
{
    indexCount = 0;
    vertexBuffer = nullptr;
    indexBuffer = nullptr;
}

uint32_t PackRGBA(float r, float g, float b, float a)
{
    uint32_t color = ((uint32_t)(r * 255) << 24) |
                     ((uint32_t)(g * 255) << 16) |
                     ((uint32_t)(b * 255) << 8) |
                      (uint32_t)(a * 255);
    return color;
}

void UnpackRGBA(uint32_t color, float &r, float &g, float &b, float &a)
{
    r = ((color >> 24) & 255) * (1.f / 255.f);
    g = ((color >> 16) & 255) * (1.f / 255.f);
    b = ((color >> 8) & 255) * (1.f / 255.f);
    a = (color & 255) * (1.f / 255.f);
}

void RenderDevice::SetProjection(const glm::mat4 &proj)
{
    projection = proj;
}

} // renderer