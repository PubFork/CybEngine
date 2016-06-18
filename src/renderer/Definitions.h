#pragma once

enum BufferUsage
{
    Buffer_Vertex       = 0x00000001,
    Buffer_Index        = 0x00000002,
    Buffer_TypeMask     = 0x000000ff,
    Buffer_ReadOnly     = 0x00002000,           
    Buffer_32BitIndex   = 0x00004000,           // Only used with index buffers, if flag is not set, 16-bit indices is assumed
};

enum VertexElementUsage
{
    VertexElementUsage_Position,
    VertexElementUsage_Normal,
    VertexElementUsage_Tangent,
    VertexElementUsage_TexCoord0,
    VertexElementUsage_TexCoord1,
    VertexElementUsage_TexCoord2,
    VertexElementUsage_TexCoord3,
    VertexElementUsage_Color,
    VertexElementUsage_Count
};

enum VertexElementFormat
{
    VertexElementFormat_Float1,
    VertexElementFormat_Float2,
    VertexElementFormat_Float3,
    VertexElementFormat_Float4,
    VertexElementFormat_UByte4,
    VertexElementFormat_UByte4N,
    VertexElementFormat_Short2,
    VertexElementFormat_Short4,
    VertexElementFormat_Count
};

enum SamplerFilter
{
    SamplerFilter_Point,
    SamplerFilter_Bilinear,
    SamplerFilter_Trilinear,
    SamplerFilter_Anisotropic
};

enum SamplerWrapMode
{
    SamplerWrap_Repeat,
    SamplerWrap_RepeatMirror,
    SamplerWrap_Clamp,
    SamplerWrap_ClampToEdge
};

enum PixelFormat
{
    PixelFormat_Unknown,
    PixelFormat_R8G8B8A8,
    PixelFormat_R8,
    PixelFormat_R32G32B32A32F,
    PixelFormat_Depth24,
    PixelFormat_Count
};

enum ClearFlags
{
    Clear_None = 0x00,
    Clear_Color = 0x01,
    Clear_Depth = 0x02,
    Clear_Stencil = 0x04,
    Clear_All = Clear_Color | Clear_Depth | Clear_Stencil
};

enum DrawStateFlags
{
    DrawState_DepthTest_Less            = 0x00000010,
    DrawState_DepthTest_LessEqual       = 0x00000020,
    DrawState_DepthTest_Greater         = 0x00000030,
    DrawState_DepthTest_GreaterEqual    = 0x00000040,
    DrawState_DepthTest_Equal           = 0x00000050,
    DrawState_DepthTest_NotEqual        = 0x00000060,
    DrawState_DepthTest_Never           = 0x00000070,
    DrawState_DepthTest_Always          = 0x00000080,
    DrawState_DepthTest_Shift           = 4,
    DrawState_DepthTest_Mask            = 0x000000f0,

    DrawState_Cull_CW                   = 0x00000100,
    DrawState_Cull_CCW                  = 0x00000200,
    DrawState_Cull_Shift                = 8,
    DrawState_Cull_Mask                 = 0x00000f00,

    DrawState_Primitive_TriangleStrip   = 0x00001000,
    DrawState_Primitive_Lines           = 0x00002000,
    DrawState_Primitive_LineStrip       = 0x00003000,
    DrawState_Primitive_Points          = 0x00004000,
    DrawState_Primitive_Shift           = 12,
    DrawState_Primitive_Mask            = 0x0000f000,

    DrawState_Default = DrawState_DepthTest_Less | DrawState_Cull_CW
};

/*
inline void DebugStateFlag(uint32_t state)
{
    uint32_t depthTestMasked = state & DrawState_DepthTest_Mask;
    uint32_t depthTestShifted = depthTestMasked >> DrawState_DepthTest_Shift;
    uint32_t cullMasked = state & DrawState_Cull_Mask;
    uint32_t cullShifted = cullMasked >> DrawState_Cull_Shift;
    uint32_t primMasked = state & DrawState_Primitive_Mask;
    uint32_t primShifted = primMasked >> DrawState_Primitive_Shift;

    const char *format = "%s (state & mask) 0x%x ((state & mask) >> shift) 0x%x";
    DEBUG_LOG_TEXT(format, "Depth", depthTestMasked, depthTestShifted);
    DEBUG_LOG_TEXT(format, "Cull", cullMasked, cullShifted);
    DEBUG_LOG_TEXT(format, "Prim", primMasked, primShifted);
}
*/