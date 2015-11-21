#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include "core/Macros.h"

namespace renderer
{

namespace dev
{

enum GeometryFlags
{
    Geo_Point               = 0x01,     // draw points
    Geo_Line                = 0x02,     // draw lines
    Geo_Triangle            = 0x03,     // draw triangle
    Geo_LineStrip           = 0x04,     // draw lines
    Geo_TriangleStrip       = 0x05,     // draw triangles
    Geo_Quad                = 0x06,     // draw quads

    Geo_DynamicVertexBuffer = 0x00,
    Geo_DynamicIndexBuffer  = 0x00,
    Geo_StaticVertexBuffer  = 0x08,
    Geo_StaticIndexBuffer   = 0x10,

    Geo_Dynamic = Geo_DynamicVertexBuffer | Geo_DynamicIndexBuffer,
    Geo_Static = Geo_StaticVertexBuffer | Geo_StaticIndexBuffer
};

} // dev

enum VertexFormat
{
    VF_Invalid,
    VF_Standard,                           // pos, normal, tex0
    VF_Double,                             // pos, color, tex0, tex1
    VF_Compact                             // pos, color
};

struct VertexStandard   // 32 bytes
{
    float x, y, z;
    float nx, ny, nz;
    float u, v;
};

struct VertexDouble     // 32 bytes
{
    float x, y, z;
    uint32_t color;
    float u0, v0;
    float u1, v1;
};

struct VertexCompact    // 16 bytes
{
    float x, y, z;
    uint32_t color;
};

enum BufferUsage
{
    Buffer_Invalid,
    Buffer_Vertex,
    Buffer_Index,
    Buffer_Uniform,
    Buffer_Compute
};

enum ShaderStage
{
    Shader_Vertex,
    Shader_Geometry,
    Shader_Fragment,
    Shader_Compute,
    Shader_Count
};

enum BuiltinShaders
{
    VShader_MV = 0,
    VShader_MVP,
    VShader_Count,

    FShader_Solid = 0,
    FShader_Gouraud,
    FShader_Count
};

enum FillMode
{
    Fill_Solid,
    Fill_Wire,
    Fill_Point
};

enum PrimitiveType
{
    Prim_Lines,
    Prim_Triangles,
    Prim_TriangleStrip,
    Prim_Quads
};

enum CullMode
{
    Cull_Back,                      // cull back facing faces
    Cull_Front,                     // cull front facing faces
    Cull_None                       // don't cull any faces based on winding
};

enum WindingOrder
{
    Winding_CW,                     // clockwise
    Winding_CCW                     // counter-clockwise
};

enum ClearFlags
{
    Clear_None      = BIT(0),
    Clear_Color     = BIT(1),
    Clear_Depth     = BIT(2),
    Clear_Stencil   = BIT(3),
    Clear_All = Clear_Color | Clear_Depth | Clear_Stencil
};

class Buffer
{
public:
    virtual ~Buffer() = default;
    virtual bool SetData(BufferUsage usage, const void *buffer, size_t bufSize) = 0;
};

class Shader
{
public:
    Shader(ShaderStage s) : stage(s) {}
    virtual ~Shader() = default;
    ShaderStage GetStage() const { return stage; }

protected:
    ShaderStage stage;
};

class ShaderSet
{
public:
    ShaderSet() = default;
    virtual ~ShaderSet() = default;

    virtual void SetShader(std::shared_ptr<Shader> s) = 0;
    virtual void UnsetShader(ShaderStage stage) = 0;
    virtual bool SetUniform(const char *name, uint32_t numFloats, const float *v) = 0;
    
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

    std::shared_ptr<Buffer> vertexBuffer;
    VertexFormat vfmt;
    std::shared_ptr<Buffer> indexBuffer;
    uint32_t indexCount;

    PrimitiveType prim;
    CullMode culling;
    WindingOrder winding;
};

struct Surface
{
    Surface()
    {
        name = "<unknown>";
    }

    std::string name;
    SurfaceGeometry geometry;
    std::shared_ptr<ShaderSet> shader;
};

class RenderDevice
{
public:
    RenderDevice() = default;
    virtual ~RenderDevice() = default;

    void SetProjection(const glm::mat4 &proj);
    std::shared_ptr<Shader> LoadBuiltinShader(ShaderStage stage, BuiltinShaders shader);
    void SetDefaultShader(std::shared_ptr<ShaderSet> shader);

    virtual std::shared_ptr<Buffer> CreateBuffer(BufferUsage usage, const void *buf, size_t bufSize) = 0;
    virtual std::shared_ptr<ShaderSet> CreateShaderSet(std::initializer_list<std::shared_ptr<Shader>> shaderList = {}) = 0;
    
    virtual void SetFillMode(FillMode mode) = 0;
    virtual void Clear(int32_t flags, uint32_t color) = 0;
    virtual void Render(const Surface *surf, const glm::mat4 &transform) = 0;

protected:
    glm::mat4 projection;
    std::shared_ptr<Shader> vertexShaders[VShader_Count];
    std::shared_ptr<Shader> fragmentShaders[FShader_Count];
    std::shared_ptr<ShaderSet> defaultShader;
};

std::shared_ptr<RenderDevice> CreateRenderDeviceGL();

} // renderer