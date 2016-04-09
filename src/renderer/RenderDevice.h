#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include "Base/Macros.h"

#include "PipelineState.h"

namespace renderer
{

class IRenderDevice;

class IGPUResouce
{
public:
    virtual ~IGPUResouce() {}
};

//==============================
// Buffer interface
//==============================

class IVertexBuffer : public IGPUResouce {};
class IIndexBuffer : public IGPUResouce {};

//==============================
// Sampler interface
//==============================

enum ESamplerFilter
{
    SamplerFilter_Point,
    SamplerFilter_Bilinear,
    SamplerFilter_Trilinear,
    SamplerFilter_Anisotropic
};

enum ESamplerWrapMode
{
    SamplerWrap_Repeat,
    SamplerWrap_RepeatMirror,
    SamplerWrap_Clamp
};

struct SamplerStateInitializer
{
    SamplerStateInitializer() {}
    SamplerStateInitializer(ESamplerFilter inFilter,
                            ESamplerWrapMode inWrapU = SamplerWrap_Repeat,
                            ESamplerWrapMode inWrapV = SamplerWrap_Repeat,
                            uint32_t inMaxAnisotropy = 0,
                            int32_t inMipBias = 0,
                            uint32_t inMinMipLevel = 0,
                            uint32_t inMaxMipLevel = UINT32_MAX) :
        filter(inFilter),
        wrapU(inWrapU),
        wrapV(inWrapV),
        mipBias(inMipBias),
        minMipLevel(inMinMipLevel),
        maxMipLevel(inMaxMipLevel),
        maxAnisotropy(inMaxAnisotropy)
    {
    }

    bool operator==(const SamplerStateInitializer &initializer) const
    {
        return memcmp(this, &initializer, sizeof(initializer)) == 0;
    }

    ESamplerFilter filter;
    ESamplerWrapMode wrapU;
    ESamplerWrapMode wrapV;
    int32_t mipBias;
    uint32_t minMipLevel;
    uint32_t maxMipLevel;
    uint32_t maxAnisotropy;
};

class ISamplerState : public IGPUResouce {};

//==============================
// Texture interface
//==============================

enum EPixelFormat
{
    PixelFormat_Unknown,
    PixelFormat_R8G8B8A8,
    PixelFormat_R8,
    PixelFormat_R32G32B32A32F,
    PixelFormat_Depth24,
    PixelFormat_Count
};

class ITexture : public IGPUResouce
{
public:
    ITexture(uint32_t inNumMipMaps, EPixelFormat inFormat) :
        numMipMaps(inNumMipMaps),
        format(inFormat)
    {
    }

    virtual ~ITexture() {}

    uint32_t GetNumMipMaps() const { return numMipMaps; }
    EPixelFormat GetFormat() const { return format; }

private:
    uint32_t numMipMaps;
    EPixelFormat format;
};

class ITexture2D : public ITexture
{
public:
    ITexture2D(uint32_t inWidth, uint32_t inHeight, uint32_t inNumMipMaps, EPixelFormat inFormat) :
        ITexture(inNumMipMaps, inFormat),
        width(inWidth),
        height(inHeight)
    {
    }

    uint32_t GetWidth()  const { return width; }
    uint32_t GetHeight() const { return height; }

private:
    uint32_t width;
    uint32_t height;
};

//==============================
// Pipeline state interface
//==============================

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

struct SurfaceGeometry
{
    SurfaceGeometry() : indexCount(0) {}
    uint32_t indexCount;
    std::shared_ptr<IVertexBuffer> VBO;
    std::shared_ptr<IIndexBuffer> IBO;
};

struct SurfaceMaterial
{
    enum { MaxNumTextures = 4 };
    std::shared_ptr<ITexture2D> texture[MaxNumTextures];
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
    BuiltintPipelineState_Skydome,
    BuiltintPipelineState_Particle,        // UNIMPLEMENTED
    BuiltintPipelineState_Count
};

#define MAX_TEXTURE_UNITS 8
struct DrawCallBase
{
    uint32_t primitiveType;
    uint32_t numPrimitives;
    std::shared_ptr<IVertexBuffer> VBO;
    uint32_t numVertices;

    std::array<std::shared_ptr<ITexture2D>, MAX_TEXTURE_UNITS> textures;
    uint32_t numTextures;
};

class IRenderDevice
{
public:
    virtual ~IRenderDevice() {}

    virtual void Init() = 0;
    virtual void Shutdown() = 0;
    virtual bool IsInitialized() const = 0;

    virtual void SetProjection(const glm::mat4 &proj) = 0;

    virtual PipelineState *BuiltintPipelineState(uint32_t pipelineStateEnum) = 0;

    virtual std::shared_ptr<IVertexBuffer> CreateVertexBuffer(const void *data, size_t size) = 0;
    virtual std::shared_ptr<IIndexBuffer> CreateIndexBuffer(const void *data, size_t size) = 0;

    virtual std::shared_ptr<ITexture2D> CreateTexture2D(int32_t width, int32_t height, EPixelFormat format, int32_t numMipMaps, const void *data) = 0;
    virtual void SetTexture(uint32_t textureIndex, const std::shared_ptr<ITexture> texture) = 0;

    virtual std::shared_ptr<ISamplerState> CreateSamplerState(const SamplerStateInitializer &initializer) = 0;
    virtual void SetSamplerState(uint32_t textureIndex, const std::shared_ptr<ISamplerState> state) = 0;

    virtual void Clear(uint32_t targets, const glm::vec4 color, float depth = 1.0f) = 0;
    virtual void Render(const Surface *surf, const glm::mat4 &transform) = 0;
};

std::shared_ptr<IRenderDevice> CreateRenderDevice();

} // renderer