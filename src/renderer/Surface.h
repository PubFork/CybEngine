#pragma once

namespace renderer
{

class ShaderSet;

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

struct InputElement
{
    InputAttribute attribute;
    DataFormat format;
    uintptr_t alignedOffset;             // optional, set to 0 to auto calculate 
};

struct InputLayout
{
    const InputElement *elements;
    uint32_t numElements;

    uint32_t GetStride() const;
    bool IsValid() const;
};


enum BufferUsage
{
    Buffer_Invalid,
    Buffer_Vertex,
    Buffer_Index,
    Buffer_Uniform,
    Buffer_Compute
};

class Buffer
{
public:
    Buffer();
    ~Buffer();

    bool Data(BufferUsage use, const void *buffer, size_t size);
    GLuint Id() const { return bufferId; }

private:
    uint32_t bufferId;
    uint32_t use;
    size_t size;
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
    Cull_Back,
    Cull_Front,
    Cull_None
};

enum WindingOrder
{
    Winding_CW,     // clockwise
    Winding_CCW     // counter-clockwise
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

}   // renderer