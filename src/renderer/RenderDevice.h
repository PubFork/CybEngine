#pragma once

#include <glm/mat4x4.hpp>

namespace renderer
{

enum DataFormat
{
    Format_RGB_F32,
    Format_RGBA_F32,
    Format_RGBA_UI8,
    Format_RGBA_UI8Norm
};

enum InputAttribute
{
    Attrib_Position,
    Attrib_Normal,
    Attrib_Color0,
    Attrib_Color1,
    Attrib_TexCoord0,
    Attrib_TexCoord1,
    Attrib_TexCoord2,
    Attrib_TexCoord3,
    Attrib_Count
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

struct InputElement
{
    InputAttribute attribute;
    DataFormat format;
    uintptr_t alignedOffset;
};

struct InputLayout
{
    const InputElement *elements;
    uint32_t numElements;

    uint32_t GetStride() const;
    bool IsValid() const;
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
    SurfaceGeometry()
    {
        vertexBuffer = nullptr;
        inputLayout = { nullptr, 0 };
        indexBuffer = nullptr;
        indexCount = 0;
        prim = Prim_Triangles;
        culling = Cull_Back;
        winding = Winding_CCW;
    }

    std::shared_ptr<Buffer> vertexBuffer;
    InputLayout inputLayout;
    std::shared_ptr<Buffer> indexBuffer;
    uint32_t indexCount;

    PrimitiveType prim;
    CullMode culling;
    WindingOrder winding;
};

struct Surface
{
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

    virtual std::shared_ptr<Buffer> CreateBuffer(BufferUsage usage, const void *buf, size_t bufSize) = 0;
    virtual std::shared_ptr<ShaderSet> CreateShaderSet(std::initializer_list<std::shared_ptr<Shader>> shaderList = {}) = 0;
    
    virtual void SetFillMode(FillMode mode) = 0;
    virtual void Clear(float r, float g, float b, float a, float depth, bool clearColor = true, bool clearDepth = true) = 0;
    virtual void Render(const Surface *surf, const glm::mat4 &transform) = 0;

protected:
    glm::mat4 projection;
    std::shared_ptr<Shader> vertexShaders[VShader_Count];
    std::shared_ptr<Shader> fragmentShaders[FShader_Count];
};

uint32_t AlignedDataFormatSize(const DataFormat format);
std::shared_ptr<RenderDevice> CreateRenderDeviceGL();

} // renderer