#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include "Base/Macros.h"

#include "Image.h"
#include "PipelineState.h"

namespace renderer
{

struct VertexStandard
{
    float x, y, z;
    float nx, ny, nz;
    float u0, v0;
};

enum ClearFlags
{
    Clear_None = 0x00,
    Clear_Color = 0x01,
    Clear_Depth = 0x02,
    Clear_Stencil = 0x04,
    Clear_All = Clear_Color | Clear_Depth | Clear_Stencil
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
    SurfaceGeometry() : indexCount(0) {}
    uint32_t indexCount;
    std::shared_ptr<Buffer> vertexBuffer;
    std::shared_ptr<Buffer> indexBuffer;
};

struct SurfaceMaterial
{
    enum { MaxNumTextures = 4 };
    std::shared_ptr<Image> texture[MaxNumTextures];
};

struct Surface
{
    Surface() : name("<unknown>") {}
    std::string name;
    SurfaceGeometry geometry;
    SurfaceMaterial material;
    PipelineState *pipelineState;
};

enum BuiltinPipelineStateEnum
{
    BuiltintPipelineState_Default,
    BuiltintPipelineState_Particle,      // UNIMPLEMENTED
    BuiltintPipelineState_Skybox,        // UNIMPLEMENTED
    BuiltintPipelineState_Count
};

class RenderDevice : public ImageCache
{
public:
    RenderDevice() = default;
    virtual ~RenderDevice() = default;

    void Init();

    void SetProjection(const glm::mat4 &proj);

    PipelineState *BuiltintPipelineState(uint32_t pipelineStateEnum);

    virtual std::shared_ptr<Buffer> CreateBuffer(Buffer::Type usage, const void *buf, size_t bufSize) = 0;
    
    virtual void Clear(uint32_t targets, const glm::vec4 color, float depth = 1.0f) = 0;
    virtual void Render(const Surface *surf, const glm::mat4 &transform) = 0;

protected:
    glm::mat4 projection;
    PipelineState builtintPipelineStates[BuiltintPipelineState_Count];
};

std::shared_ptr<RenderDevice> CreateRenderDeviceGL();

} // renderer