#pragma once

#include "InputLayout.h"

namespace renderer
{

class Buffer;
class ShaderSet;

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