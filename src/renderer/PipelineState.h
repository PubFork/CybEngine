#pragma once

namespace renderer
{

enum RasterizerStateFlags
{
    Raster_PrimPoint            = 0x0000,
    Raster_PrimLine             = 0x0001,
    Raster_PrimTriangle         = 0x0002,
    Raster_PrimTriangleStrip    = 0x0003,
    Raster_PrimMask             = 0x000f,

    Raster_CullBack             = 0x0010,       // cull back facing faces
    Raster_CullFront            = 0x0020,       // cull front facing faces
    Raster_CullNone             = 0x0030,       // dont't cull and faces at rasterer level
    Raster_CullMask             = 0x00f0,

    Raster_FrontCCW             = 0x0100,       // counter-clockwise ordered face indices
    Raster_FrontCW              = 0x0200,       // clockwise ordered face indices
    Raster_FrontMask            = 0x0f00,

    Raster_DefaultState = Raster_PrimTriangle | Raster_CullBack | Raster_FrontCCW
};

//==============================
// Vertex Input Layout
//==============================

enum VertexFormat2
{
    VertexFormat_Float2,
    VertexFormat_Float3,
    VertexFormat_UByte4,
    VertexFormat_UByte4N
};

struct VertexInputElement
{
    const char *attribName;
    int16_t location;
    VertexFormat2 format;
    int16_t alignedOffset;  // 0 = auto-calculate (TODO: implement manual control)
};

struct VertexInputLayout
{
    VertexInputElement *elements;
    uint16_t numElements;
    uint16_t stride;       // auto-calculated, set to zero
};

void BindVertexLayout(const VertexInputLayout &layout);
void UnBindVertexLayout(const VertexInputLayout &layout);

//==============================
// Shader Program
//==============================

class ShaderProgram
{
public:
    enum { InvalidUniform = -1 };
    enum ShaderStage
    {
        Stage_Vertex,
        Stage_Fragment,
        Stage_Count
    };

    ShaderProgram();
    ~ShaderProgram();

    void AttachShaderFromString(ShaderStage stage, const char *source);
    void AttachShaderFromFile(ShaderStage stage, const char *filename);
    bool Link();
    bool IsLinked() const;

    int32_t GetUniformLocation(const char *name);
    void SetUniformVec3(int32_t location, const float *value);
    void SetUniformMat4(int32_t location, const float *value);

    void Bind();

private:
    // API specific members
    uint32_t shaderStages[Stage_Count];
    uint32_t shaderProgram;
};

//==============================
// Pipeline State
//==============================

enum ShaderParam
{
    Param_Mat4Begin,
    Param_Proj,
    Param_ModelView,
    Param_Mat4End,

    Param_Vec3Begin,
    Param_Vec3Param1,       // dummy
    Param_Vec3Param2,       // dummy
    Param_Vec3End,

    Param_Count
};

struct CreatePipelineStateInfo
{
    const char *vertShaderFilename;
    const char *fragShaderFilename;
    VertexInputLayout inputLayout;
    uint32_t rasterFlags;
};

class PipelineState
{
public:
    void Create(const CreatePipelineStateInfo &info);
    void Destroy();

    void Bind();

    void SetParamMat4(uint32_t param, const float *value);
    void SetParamVec3(uint32_t param, const float *value);
    const VertexInputLayout &GetVertexLayout() const { return inputLayout; }
    const uint32_t GetGLPrimType() const;   // For the renderer, use something better (more generic)?

private:
    int32_t ValidatedParamLocation(uint32_t param);
    void SetupRasterer();

    uint32_t rasterizerFlags;
    VertexInputLayout inputLayout;
    ShaderProgram program;
    int32_t paramLocations[Param_Count];
};

} // renderer