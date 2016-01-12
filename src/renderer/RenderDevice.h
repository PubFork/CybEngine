#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include "core/Macros.h"

#include "Image.h"

namespace renderer
{

enum VertexFormat
{
    VertexFormat_Invalid,
    VertexFormat_Standard,                          // 64 bytes
    VertexFormat_ShadedTex,                         // 32 bytes
    VertexFormat_DoubleTex,                         // 32 bytes
    VertexFormat_Compact                            // 16 bytes
};

enum BuiltinShaders
{
    VShader_MVP                 = 0,
    VShader_Count,

    FShader_Solid               = 0,
    FShader_Gouraud,
    FShader_LitGouraud,
    FShader_Count
};

enum RasterStateFlags
{
    Raster_DrawSolid            = 0x00000000,       // draw solid primitives
    Raster_DrawWire             = 0x00000001,       // draw the edges of the primitives
    Raster_DrawPoint            = 0x00000010,       // draw the dots of the primitive vertices
    Raster_DrawMask             = 0x00000011,

    Raster_PrimTriangle         = 0x00000000,       // draw triangle primitives
    Raster_PrimLine             = 0x00000100,       // draw line primitives
    Raster_PrimQuad             = 0x00001000,       // draw quad primitives
    Raster_PrimTriangleStrip    = 0x00010000,       // draw triangle-strip primitives
    Raster_PrimMask             = 0x00011100,

    Raster_CullBack             = 0x00000000,       // cull back facing faces
    Raster_CullFront            = 0x00100000,       // cull front facing faces
    Raster_CullNone             = 0x01000000,       // dont't cull and faces at rasterer level
    Raster_CullMask             = 0x01100000,

    Raster_OrderCCW             = 0x00000000,       // counter-clockwise ordered face indices
    Raster_OrderCW              = 0x10000000,       // clockwise ordered face indices
    Raster_OrderMask            = 0x10000000
};

enum TextureFormat
{
    Texture_RGBA                = 0x00001000,
    Texture_BGRA                = 0x00001001,
    Texture_DXT1                = 0x00001010,       // unsupported
    Texture_DXT3                = 0x00001011,       // unsupported
    Texture_DXT5                = 0x00001110        // unsupported
};

enum SampleMode
{
    Sample_Linear               = 0x00000000,
    Sample_Nearest              = 0x00000001,
    Sample_Anisotropic          = 0x00000010,
    Sample_FilterMask           = 0x00000011,

    Sample_Repeat               = 0x00000000,
    Sample_Clamp                = 0x00000100,
    Sample_ClampBorder          = 0x00001000,
    Sample_AddressMask          = 0x00001100
};

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
    float u1, v1;
    float u2, v2;
    float u3, v3;
    uint32_t color0;
    uint32_t color1;
};

struct VertexShadedTex
{
    float x, y, z;
    float nx, ny, nz;
    float u0, v0;
};

struct VertexDoubleTex
{
    float x, y, z;
    uint32_t color0;
    float u0, v0;
    float u1, v1;
};

struct VertexCompact
{
    float x, y, z;
    uint32_t color0;
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

class Shader
{
public:
    enum Type
    {
        Vertex,
        Fragment,
        Geometry,
        Compute,
        Count
    };

    Shader(Type s) : stage(s) {}
    virtual ~Shader() = default;
    Type GetStage() const { return stage; }

protected:
    Type stage;
};

class ShaderSet
{
public:
    enum { MAX_TEXTURES = 4 };
    
    enum Builtin
    {
        Builtin_Color,
        Builtin_Texture,
        Builtin_Max
    };

    ShaderSet() = default;
    virtual ~ShaderSet() = default;

    virtual void SetShader(std::shared_ptr<Shader> s) = 0;
    virtual void UnsetShader(Shader::Type stage) = 0;
    virtual bool SetUniformfv(const char *name, uint32_t numFloats, const float *v) = 0;
    
    bool SetUniform1f(const char *name, float x);
    bool SetUniform2f(const char *name, float x, float y);
    bool SetUniform3f(const char *name, float x, float y, float z);
    bool SetUniform4f(const char *name, float x, float y, float z, float w = 1.0f);
    bool SetUniform4fv(const char *name, const float *v);
    bool SetUniform4x4f(const char *name, const glm::mat4 &m);
};

struct SurfaceGeometry
{
    SurfaceGeometry();

    VertexFormat format;
    uint32_t indexCount;
    uint32_t rasterState;
    std::shared_ptr<Buffer> vertexBuffer;
    std::shared_ptr<Buffer> indexBuffer;
};

struct Color4f
{
    Color4f() : r(0.0f), g(0.0f), b(0.0f), a(1.0f) {}
    Color4f(const glm::vec3 &v) : r(v.x), g(v.y), b(v.z), a(1.0f) {}
    Color4f(float ir, float ig, float ib, float ia) : r(ir), g(ig), b(ib), a(ia) {}

    float r;
    float g;
    float b;
    float a;
};

uint32_t PackRGBA(float r, float g, float b, float a);
void UnpackRGBA(uint32_t color, float &r, float &g, float &b, float &a);

struct SurfaceMaterial
{
    std::shared_ptr<ShaderSet> shader;
    std::shared_ptr<Image> texture[ShaderSet::MAX_TEXTURES];
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
    virtual std::shared_ptr<Shader> LoadBuiltinShader(Shader::Type stage, BuiltinShaders shader) = 0;
    void SetDefaultShader(std::shared_ptr<ShaderSet> shader);

    virtual std::shared_ptr<Buffer>    CreateBuffer(Buffer::Type usage, const void *buf, size_t bufSize) = 0;
    virtual std::shared_ptr<ShaderSet> CreateShaderSet(std::initializer_list<std::shared_ptr<Shader>> shaderList = {}) = 0;
    
    std::shared_ptr<ShaderSet> LoadBuiltinShaderSet(ShaderSet::Builtin shader);

    virtual void Clear(int32_t flags, uint32_t color) = 0;
    virtual void Render(const Surface *surf, const glm::mat4 &transform) = 0;

protected:
    glm::mat4 projection;
    std::shared_ptr<Shader> vertexShaders[VShader_Count];
    std::shared_ptr<Shader> fragmentShaders[FShader_Count];
    std::shared_ptr<ShaderSet> builtinShaderSets[ShaderSet::Builtin_Max];
    std::shared_ptr<ShaderSet> defaultShader;
};

std::shared_ptr<RenderDevice> CreateRenderDeviceGL();

} // renderer