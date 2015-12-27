#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include "core/Macros.h"

namespace renderer
{

enum VertexFormat
{
    VertexFormat_Invalid,
    VertexFormat_Standard,          // 64 bytes
    VertexFormat_ShadedTex,         // 32 bytes
    VertexFormat_DoubleTex,         // 32 bytes
    VertexFormat_Compact            // 16 bytes
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

enum BuiltinShaders
{
    VShader_MV = 0,
    VShader_MVP,
    VShader_Count,

    FShader_Solid = 0,
    FShader_Gouraud,
    FShader_LitGouraud,
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

    enum Builtin
    {
        Builtin_Color,
        Builtin_Texture,
        Builtin_Max
    };
};

struct SurfaceGeometry
{
    SurfaceGeometry();

    std::shared_ptr<Buffer> vertexBuffer;
    VertexFormat format;
    std::shared_ptr<Buffer> indexBuffer;
    uint32_t indexCount;

    PrimitiveType prim;
    CullMode culling;
    WindingOrder winding;
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
    Color4f color;
    std::shared_ptr<ShaderSet> shader;
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

    virtual std::shared_ptr<Buffer> CreateBuffer(Buffer::Type usage, const void *buf, size_t bufSize) = 0;
    virtual std::shared_ptr<ShaderSet> CreateShaderSet(std::initializer_list<std::shared_ptr<Shader>> shaderList = {}) = 0;
    std::shared_ptr<ShaderSet> LoadBuiltinShaderSet(ShaderSet::Builtin shader);

    virtual void SetFillMode(FillMode mode) = 0;
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