#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include "Base/Macros.h"

#include "PipelineState.h"

namespace renderer
{

class IRenderDevice;

//==============================
// Buffer interface
//==============================

enum BufferType
{
    Buffer_Invalid,
    Buffer_Vertex,
    Buffer_Index,
    //Buffer_Uniform,
};

struct BufferCreateParams
{
    BufferType usage;
    const void *data;
    size_t dataLength;
};

class IBuffer
{
public:
    virtual ~IBuffer() {}

    virtual void Create(const BufferCreateParams *params) = 0;
    virtual void Bind() const = 0;
};

//==============================
// Image interface
//==============================

enum ImageFilterMode
{
    ImageFilter_Nearest,
    ImageFilter_Linear,
    ImageFilter_Anisotropic
};

enum ImageWrapMode
{
    ImageWrap_Repeat,
    ImageWrap_Clamp,
    ImageWrap_ClampBorder,
};

enum ImageFormat
{
    ImageFormat_None,
    ImageFormat_RGBA8,                   // 4 channel, 32 bpp (R8:G8:B8:A8)
    ImageFormat_Alpha,                   // 1 channel,  8 bpp (R8)              (NOTE: Unimplemented)
    ImageFormat_DXT5,                    // 4 channel, 24 bpp (R5:G6:B5:A8)     (NOTE: Unimplemented)
};

struct ImageCreateParams
{
    std::string name;
    uint32_t width;
    uint32_t height;
    ImageFormat format;
    const void *pixels;
    ImageFilterMode filterMode;
    ImageWrapMode wrapMode;
};

class IImage
{
public:
    virtual ~IImage() {}

    virtual void Create(const ImageCreateParams *params) = 0;
    virtual void Bind(uint8_t slot) = 0;
    virtual void UpdateFilterMode(ImageFilterMode filterMode) = 0;
};

//==============================
// Pipeline state interface
//==============================

class IVertexLayout
{
public:
    virtual void Create(const VertexInputElement *elements, uint8_t numElements) = 0;
    virtual void Bind() const = 0;
    virtual void UnBind() const = 0;
    virtual uint32_t Hash() const = 0;
};

enum PipelineRasterFlags
{
    Raster2_PrimPoint = 0x0000,
    Raster2_PrimLine = 0x0001,
    Raster2_PrimTriangle = 0x0002,
    Raster2_PrimTriangleStrip = 0x0003,
    Raster2_PrimMask = 0x000f,

    Raster2_CullBack = 0x0010,       // cull back facing faces
    Raster2_CullFront = 0x0020,       // cull front facing faces
    Raster2_CullNone = 0x0030,       // dont't cull and faces at rasterer level
    Raster2_CullMask = 0x00f0,

    Raster2_FrontCCW = 0x0100,       // counter-clockwise ordered face indices
    Raster2_FrontCW = 0x0200,       // clockwise ordered face indices
    Raster2_FrontMask = 0x0f00,

    Raster2_DefaultState = Raster2_PrimTriangle | Raster2_CullBack | Raster2_FrontCCW
};

enum PipelineParams
{
    Param2_Mat4Begin,
    Param2_Proj,
    Param2_ModelView,
    Param2_Mat4End,

    Param2_Vec3Begin,
    Param2_Vec3Param1,       // dummy
    Param2_Vec3Param2,       // dummy
    Param2_Vec3End,

    Param2_Count
};

struct PipelineStateCreateParams
{
    const char *VS;
    const char *FS;
    IVertexLayout *vertexLayout;
    uint32_t rasterFlags;
};

class IPipelineState
{
public:
    virtual void Create(const PipelineStateCreateParams *param) = 0;
    virtual void Bind() const = 0;

    virtual void SetParamMat4(PipelineParams param, const float *value) = 0;
    virtual void SetParamVec3(PipelineParams param, const float *value) = 0;
    virtual const IVertexLayout *VertexLayout() const = 0;
};

////////////////////////
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
    std::shared_ptr<IBuffer> VBO;
    std::shared_ptr<IBuffer> IBO;
};

struct SurfaceMaterial
{
    enum { MaxNumTextures = 4 };
    std::shared_ptr<IImage> texture[MaxNumTextures];
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

class IRenderDevice
{
public:
    virtual ~IRenderDevice() {}
    virtual bool IsInitialized() const = 0;
};

class RenderDevice : public IRenderDevice
{
public:
    RenderDevice() : isInititialized(false) {}
    virtual ~RenderDevice() = default;

    void Init();
    virtual bool IsInitialized() const { return isInititialized; }

    void SetProjection(const glm::mat4 &proj);

    PipelineState *BuiltintPipelineState(uint32_t pipelineStateEnum);

    std::shared_ptr<IBuffer> CreateBuffer(BufferType usage, const void *dataBuffer, size_t dataBufferSize);
    std::shared_ptr<IImage> ImageFromFile(const char *filename, uint32_t sampleFlags);
    std::shared_ptr<IImage> ImageFromMemory(const char *name, const void *data, uint32_t width, uint32_t height, uint32_t format, uint32_t sampleFlags);
    std::shared_ptr<IImage> FindImage(const char *name);
    
    virtual void Clear(uint32_t targets, const glm::vec4 color, float depth = 1.0f) = 0;
    virtual void Render(const Surface *surf, const glm::mat4 &transform) = 0;

protected:
    std::shared_ptr<IImage> ImageFromMemoryInternal(const char *name, const void *data, uint32_t width, uint32_t height, uint32_t format, uint32_t sampleFlags);

    glm::mat4 projection;
    PipelineState builtintPipelineStates[BuiltintPipelineState_Count];
    std::unordered_map<uint32_t, std::shared_ptr<IImage>> imageCache;
    bool isInititialized;
};

std::shared_ptr<RenderDevice> CreateRenderDeviceGL();

} // renderer