#include "stdafx.h"

#include <GL/glew.h>
#include <Windows.h>
#include "PipelineState.h"
#include "Base/Macros.h"
#include "Base/Debug.h"
#include "Base/FileUtils.h"

namespace renderer
{

//==============================
// Vertex Input Layout
//==============================

static const struct VertexElementInfo
{
    uint16_t alignedSize;
    GLint numComponents;
    GLenum type;
    GLboolean normalized;
} vertexlElementInfoTable[] =
{
    { 8,  2, GL_FLOAT,         GL_FALSE },      // VertexFormat_Float2
    { 12, 3, GL_FLOAT,         GL_FALSE },      // VertexFormat_Float3
    { 4,  4, GL_UNSIGNED_BYTE, GL_FALSE },      // VertexFormat_UByte4
    { 4,  4, GL_UNSIGNED_BYTE, GL_TRUE  }       // VertexFormat_UByte4N
};

static void SetupVertexLayout(VertexInputLayout *dest, const VertexInputLayout &source)
{
    assert(dest);
    assert(source.numElements > 0);
    assert(source.elements);

    memcpy(dest, &source, sizeof(source));

    uint16_t strideCount = 0;
    for (uint16_t i = 0; i < dest->numElements; i++)
    {
        VertexInputElement *element = &dest->elements[i];
        element->alignedOffset = strideCount;
        strideCount += vertexlElementInfoTable[element->format].alignedSize;
    }

    dest->stride = strideCount;
}

void BindVertexLayout(const VertexInputLayout &layout)
{
    for (uint32_t i = 0; i < layout.numElements; i++)
    {
        const VertexInputElement *element = &layout.elements[i];
        const VertexElementInfo *info = &vertexlElementInfoTable[element->format];
        glEnableVertexAttribArray(element->location);
        glVertexAttribPointer(element->location, info->numComponents, info->type, info->normalized, layout.stride, (void *)element->alignedOffset);
    }
}

void UnBindVertexLayout(const VertexInputLayout &layout)
{
    for (uint32_t i = 0; i < layout.numElements; i++)
    {
        const VertexInputElement *element = &layout.elements[i];
        glDisableVertexAttribArray(element->location);
    }
}

//==============================
// Shader Program
//==============================

ShaderProgram::ShaderProgram()
{
    memset(shaderStages, 0, sizeof(shaderStages));
    shaderProgram = 0;
}

void ShaderProgram::Create()
{
    // remove any previous data from the program
    Destroy();

    shaderProgram = glCreateProgram();
    THROW_FATAL_COND(shaderProgram == 0, "Failed to allocate shader program.");
}

void ShaderProgram::Destroy()
{
    for (auto shader : shaderStages)
    {
        if (shader > 0)
        {
            glDetachShader(shaderProgram, shader);
            glDeleteShader(shader);
        }
    }

    glDeleteProgram(shaderProgram);
    shaderProgram = 0;
}

ShaderProgram::~ShaderProgram()
{
    Destroy();
}

void ShaderProgram::AttachShaderFromString(ShaderStage stage, const char *source)
{
    assert(shaderProgram);
    assert(stage < Stage_Count);
    assert(source);

    static GLuint mapToGLStage[] = { GL_VERTEX_SHADER, GL_FRAGMENT_SHADER };

    GLuint &shaderId = shaderStages[stage];
    if (!shaderId)
    {
        shaderId = glCreateShader(mapToGLStage[stage]);
        glAttachShader(shaderProgram, shaderId);
    }

    glShaderSource(shaderId, 1, &source, 0);
    glCompileShader(shaderId);

    GLint compiled = 0;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compiled);
    if (!compiled)
    {
        GLchar infoLog[KILOBYTES(1)];
        glGetShaderInfoLog(shaderId, sizeof(infoLog), 0, infoLog);
        DEBUG_LOG_TEXT_COND(infoLog[0], "Compiling shader:\n\n%s\nFailed: %s", source, infoLog);
        throw FatalException("Failed to compile shader!");
    }

    if (shaderStages[Stage_Vertex] && shaderStages[Stage_Fragment])
    {
        Link();
    }
}

void ShaderProgram::AttachShaderFromFile(ShaderStage stage, const char *filename)
{
    assert(stage < Stage_Count);
    assert(filename);
    static const char *shaderStageString[] = { "vertex", "fragment" };

    FileReader sourceFile(filename, true);
    DEBUG_LOG_TEXT("Compiling %s shader stage from sourcefile %s", shaderStageString[stage], filename);
    AttachShaderFromString(stage, sourceFile.RawBuffer());
}

bool ShaderProgram::Link()
{
    assert(shaderProgram);
    glLinkProgram(shaderProgram);
    GLint linked = 0;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        GLchar infoLog[KILOBYTES(1)];
        glGetProgramInfoLog(shaderProgram, sizeof(infoLog), 0, infoLog);
        DEBUG_LOG_TEXT_COND(infoLog[0], "Linking shaders failed: %s", infoLog);
        return false;
    }

    glUseProgram(shaderProgram);
    return true;
}

bool ShaderProgram::IsLinked() const
{
    GLint linked = 0;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linked);
    return linked == GL_TRUE;
}

int32_t ShaderProgram::GetUniformLocation(const char *name)
{
    assert(IsLinked());
    return name == nullptr ? InvalidUniform : glGetUniformLocation(shaderProgram, name);
}

void ShaderProgram::SetUniformVec3(int32_t location, const float *value)
{
    assert(value);
    assert(location != InvalidUniform);
    glProgramUniform3fv(shaderProgram, location, 1, value);
}

void ShaderProgram::SetUniformMat4(int32_t location, const float *value)
{
    assert(value);
    assert(location != InvalidUniform);
    glProgramUniformMatrix4fv(shaderProgram, location, 1, GL_FALSE, value);
}

void ShaderProgram::Bind()
{
    assert(IsLinked());
    glUseProgram(shaderProgram);
}

//==============================
// Pipeline State
//==============================

static const char *paramNames[] =
{
    nullptr,
    "projectionMatrix",
    "modelViewMatrix",
    nullptr,

    nullptr,
    "vec3Param1",
    "vec3Param2",
    nullptr
};

PipelineState::PipelineState()
{
    FOR_EACH(paramLocations, [](int32_t &loc) { loc = ShaderProgram::InvalidUniform; });
    rasterizerFlags = Raster_DefaultState;
}

void PipelineState::Create(const CreatePipelineStateInfo &info)
{
    rasterizerFlags = info.rasterFlags;

    // setup vertex layout
    SetupVertexLayout(&inputLayout, info.inputLayout);

    // setup shader program
    program.Create();
    program.AttachShaderFromFile(ShaderProgram::Stage_Vertex, info.vertShaderFilename);
    program.AttachShaderFromFile(ShaderProgram::Stage_Fragment, info.fragShaderFilename);
    for (uint32_t i = 0; i < Param_Count; i++)
    {
        paramLocations[i] = program.GetUniformLocation(paramNames[i]);
    }
}

// this should probably do nothing, the pipeline state should be destroyed 
// by it's class destructor. Destroying would just make the pipeline state 
// invalid, wich seems like a pretty useless feature.
void PipelineState::Destroy()
{
}

void PipelineState::Bind()
{
    SubmitRasterizerFlags();
    program.Bind();
}

// Get the location of a param, show a warning if it's not present in the shader program.
int32_t PipelineState::ValidatedParamLocation(uint32_t param)
{
    DEBUG_LOG_TEXT_COND(paramLocations[param] == ShaderProgram::InvalidUniform, "Warning: Param %s is not present in the selcted shader program", paramNames[param]);
    return paramLocations[param];
}

void PipelineState::SubmitRasterizerFlags()
{
    switch (rasterizerFlags & Raster_CullMask)
    {
    case Raster_CullBack:
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        break;
    case Raster_CullFront:
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        break;
    case Raster_CullNone:
        glDisable(GL_CULL_FACE);
        break;
    default: assert(!"Invalid enum");
    }

    switch (rasterizerFlags & Raster_FrontMask)
    {
    case Raster_FrontCCW:
        glFrontFace(GL_CCW);
        break;
    case Raster_FrontCW:
        glFrontFace(GL_CW);
        break;
    default: assert(!"Invalid enum");
    }
}

void PipelineState::SetParamVec3(uint32_t param, const float *value)
{
    assert(param > Param_Vec3Begin && param < Param_Vec3End);
    program.SetUniformVec3(ValidatedParamLocation(param), value);
}

void PipelineState::SetParamMat4(uint32_t param, const float *value)
{
    assert(param > Param_Mat4Begin && param < Param_Mat4End);
    program.SetUniformMat4(ValidatedParamLocation(param), value);
}

const uint32_t PipelineState::GetGLPrimType() const
{
    switch (rasterizerFlags & Raster_PrimMask)
    {
    case Raster_PrimPoint:          return GL_POINTS;
    case Raster_PrimLine:           return GL_LINES;
    case Raster_PrimTriangle:       return GL_TRIANGLES;
    case Raster_PrimTriangleStrip:  return GL_TRIANGLE_STRIP;
    default: break;
    }

    assert(!"Invalid enum");
    return 0;
}

} // renderer