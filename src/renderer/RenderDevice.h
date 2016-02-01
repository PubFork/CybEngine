#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include "Base/Macros.h"

#include "Image.h"
#include "PipelineState.h"

namespace renderer
{

enum ClearFlags
{
    Clear_None                  = 0x00000000,
    Clear_Color                 = 0x00000001,
    Clear_Depth                 = 0x00000002,
    Clear_Stencil               = 0x00000004,
    Clear_All                   = Clear_Color | Clear_Depth | Clear_Stencil
};

struct VertexStandard
{
    float x, y, z;
    float nx, ny, nz;
    float u0, v0;
};

class Buffer
{
public:
    enum Type
    {
        Invalid,
        Vertex,
        Index,
        Uniform
    };

    virtual ~Buffer() = default;
    virtual bool SetData(Type usage, const void *buffer, size_t bufSize) = 0;
};

struct SurfaceGeometry
{
    SurfaceGeometry();

    uint32_t indexCount;
    std::shared_ptr<Buffer> vertexBuffer;
    std::shared_ptr<Buffer> indexBuffer;
};

uint32_t PackRGBA(float r, float g, float b, float a);
void UnpackRGBA(uint32_t color, float &r, float &g, float &b, float &a);

struct SurfaceMaterial
{
    enum { MaxNumTextures = 4 };
    //std::shared_ptr<ShaderSet> shader;
    std::shared_ptr<Image> texture[MaxNumTextures];
};

struct Surface
{
    Surface()
    {
        name = "<unknown>";
    }

    std::string name;
    SurfaceGeometry geometry;
    SurfaceMaterial material;
};

class RenderDevice
{
public:
    RenderDevice() = default;
    virtual ~RenderDevice() = default;

    void SetProjection(const glm::mat4 &proj);

    virtual std::shared_ptr<Buffer>    CreateBuffer(Buffer::Type usage, const void *buf, size_t bufSize) = 0;
    
    virtual void Clear(int32_t flags, uint32_t color) = 0;
    virtual void Render(const Surface *surf, const glm::mat4 &transform, PipelineState &pstate) = 0;

protected:
    glm::mat4 projection;
};

std::shared_ptr<RenderDevice> CreateRenderDeviceGL();

} // renderer