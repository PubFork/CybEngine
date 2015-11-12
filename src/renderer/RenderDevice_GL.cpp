#include "stdafx.h"
#include "RenderDevice_GL.h"

#include "core/Log.h"
#include "InputLayout.h"
#include "Surface.h"

namespace renderer
{

static const char *vertexShaderMVSource =
"#version 430 core\n"
"uniform mat4 u_modelView;\n"
"in vec3 a_position;\n"
"in vec4 a_color0;\n"
"out vec4 color0;\n"
"void main()\n"
"{\n"
"   gl_Position = u_modelView * vec4(a_position, 1);\n"
"   color0 = a_color0;\n"
"}\n";

static const char *vertexShaderMVPSource =
"#version 430 core\n"
"uniform mat4 u_proj;\n"
"uniform mat4 u_modelView;\n"
"in vec3 a_position;\n"
"in vec4 a_color0;\n"
"out vec4 color0;\n"
"void main()\n"
"{\n"
"   gl_Position = u_proj * (u_modelView * vec4(a_position, 1));\n"
"   color0 = a_color0;\n"
"}\n";

static const char *fragmentShaderFlatSource =
"#version 430 core\n"
"uniform vec4 u_color;\n"
"out vec4 fragColor;\n"
"void main()\n"
"{\n"
"   fragColor = u_color;\n"
"}\n";

static const char *fragmentShaderGouraudSource =
"#version 430 core\n"
"in vec4 color0;\n"
"out vec4 fragColor;\n"
"void main()\n"
"{\n"
"   fragColor = color0;\n"
"}\n";

static const char *vertexShaderSources[VShader_Count] = {
    vertexShaderMVSource,
    vertexShaderMVPSource
};

static const char *fragmentShaderSources[FShader_Count] = {
    fragmentShaderFlatSource,
    fragmentShaderGouraudSource
};

static const char *attribName[Attrib_Count] = {
    "a_position",
    "a_normal",
    "a_color0",
    "a_color1",
    "a_texCoord0",
    "a_texCoord1",
    "a_texCoord2",
    "a_texCoord3",
};

struct DataFormatInfo
{
    GLint size;
    GLenum type;
    GLboolean normalized;
};

static const DataFormatInfo formatInfo[] = {
    { 3, GL_FLOAT,         GL_FALSE },      // Format_RGB_F32
    { 4, GL_FLOAT,         GL_FALSE },      // Format_RGBA_F32
    { 4, GL_UNSIGNED_BYTE, GL_FALSE },      // Format_RGBA_UI8
    { 4, GL_UNSIGNED_BYTE, GL_TRUE },      // Format_RGBA_UI8Norm
};

static const GLenum glPrimType[] = {
    GL_LINES,
    GL_TRIANGLES,
    GL_TRIANGLE_STRIP,
    GL_QUADS
};

void GLAPIENTRY DebugOutputCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei /*length*/, const GLchar *message, const void * /*userParam*/)
{
    static std::unordered_map<GLenum, const char *> toString = {
        { GL_DEBUG_SOURCE_API_ARB               , "OpenGL" },
        { GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB     , "Windows" },
        { GL_DEBUG_SOURCE_SHADER_COMPILER_ARB   , "Shader compiler" },
        { GL_DEBUG_SOURCE_THIRD_PARTY_ARB       , "Third party" },
        { GL_DEBUG_SOURCE_APPLICATION_ARB       , "Application" },
        { GL_DEBUG_SOURCE_OTHER_ARB             , "Other" },
        { GL_DEBUG_TYPE_ERROR_ARB               , "Error" },
        { GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB , "Deprecated behaviour" },
        { GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB  , "Undefined behaviour" },
        { GL_DEBUG_TYPE_PORTABILITY_ARB         , "Portability" },
        { GL_DEBUG_TYPE_PERFORMANCE_ARB         , "Performence" },
        { GL_DEBUG_TYPE_MARKER                  , "Marker" },
        { GL_DEBUG_TYPE_PUSH_GROUP              , "Push group" },
        { GL_DEBUG_TYPE_POP_GROUP               , "Pop group" },
        { GL_DEBUG_TYPE_OTHER_ARB               , "Other" },
        { GL_DEBUG_SEVERITY_HIGH_ARB            , "High" },
        { GL_DEBUG_SEVERITY_MEDIUM_ARB          , "Medium" },
        { GL_DEBUG_SEVERITY_LOW_ARB             , "Low" },
        { GL_DEBUG_SEVERITY_NOTIFICATION        , "Notification" }
    };

    DEBUG_LOG_TEXT("[GL DEBUG] %s %s %#x %s: %s", toString[source], toString[type], id, toString[severity], message);
}

void SetCullFlags(CullMode culling, WindingOrder winding)
{
    switch (winding) {
    case Winding_CCW: glFrontFace(GL_CCW); break;
    case Winding_CW:  glFrontFace(GL_CW); break;
    default: assert(0);
    }

    switch (culling) {
    case Cull_None:
        glDisable(GL_CULL_FACE);
        break;
    case Cull_Back:
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        break;
    case Cull_Front:
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        break;
    default:
        assert(0);
        break;
    }
}

Buffer_GL::Buffer_GL() :
    bufferId(0),
    target(GL_INVALID_ENUM),
    size(0)
{
}

Buffer_GL::~Buffer_GL()
{
    if (bufferId)
        glDeleteBuffers(1, &bufferId);
}

bool Buffer_GL::SetData(BufferUsage usage, const void *buffer, size_t bufSize)
{
    switch (usage) {
    case Buffer_Index: target = GL_ELEMENT_ARRAY_BUFFER; break;
    default:           target = GL_ARRAY_BUFFER; break;
    }

    if (!bufferId)
        glCreateBuffers(1, &bufferId);
    size = bufSize;

    glBindBuffer(target, bufferId);
    glBufferData(target, size, buffer, GL_STATIC_DRAW);
    return true;
}

Shader_GL::Shader_GL(ShaderStage type, const char *source) :
    Shader(type),
    shaderId(0)
{
    THROW_FATAL_COND(Compile(source) != true, "Failed to compile shader source");
}

Shader_GL::~Shader_GL()
{
    if (shaderId)
        glDeleteShader(shaderId);
}

GLenum Shader_GL::GLStage() const
{
    static GLenum glStages[] = {
        GL_VERTEX_SHADER,
        GL_GEOMETRY_SHADER,
        GL_FRAGMENT_SHADER,
        GL_COMPUTE_SHADER
    };

    return glStages[GetStage()];
}

bool Shader_GL::Compile(const char *source)
{
    if (!shaderId)
        shaderId = glCreateShader(GLStage());

    glShaderSource(shaderId, 1, &source, 0);
    glCompileShader(shaderId);

    GLint compiled = 0;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLchar infoLog[1024];
        glGetShaderInfoLog(shaderId, sizeof(infoLog), 0, infoLog);
        DEBUG_LOG_TEXT_COND(infoLog[0], "Compiling shader:\n%s\nFailed: %s", source, infoLog);
        return false;
    }

    return true;
}

ShaderSet_GL::ShaderSet_GL()
{
    progId = glCreateProgram();
}

ShaderSet_GL::~ShaderSet_GL()
{
    glDeleteProgram(progId);
}

void ShaderSet_GL::SetShader(std::shared_ptr<Shader> s)
{
    shaders[s->GetStage()] = s;

    std::shared_ptr<Shader_GL> gls = std::dynamic_pointer_cast<Shader_GL>(s);
    glAttachShader(progId, gls->shaderId);

    if (shaders[Shader_Vertex] && shaders[Shader_Fragment])
        THROW_FATAL_COND(Link() != true, "Failed to link shader program.");
}

void ShaderSet_GL::UnsetShader(ShaderStage stage)
{
    std::shared_ptr<Shader_GL> shader = std::dynamic_pointer_cast<Shader_GL>(shaders[stage]);

    if (shader) {
        glDetachShader(progId, shader->shaderId);
        shaders[stage] = nullptr;
    }
}

bool ShaderSet_GL::Link()
{
    glLinkProgram(progId);
    GLint linked = 0;
    glGetProgramiv(progId, GL_LINK_STATUS, &linked);
    if (!linked) {
        GLchar infoLog[1024];
        glGetProgramInfoLog(progId, sizeof(infoLog), 0, infoLog);
        DEBUG_LOG_TEXT_COND(infoLog[0], "Linking shaders failed: %s", infoLog);
        return false;
    }

    glUseProgram(progId);

    GLint uniformCount = 0;
    glGetProgramiv(progId, GL_ACTIVE_UNIFORMS, &uniformCount);
    for (GLint i = 0; i < uniformCount; i++) {
        GLint size = 0;
        GLenum type;
        GLchar name[32];
        glGetActiveUniform(progId, i, sizeof(name), 0, &size, &type, name);

        if (size) {
            Uniform u;
            u.name = name;
            u.location = glGetUniformLocation(progId, name);

            switch (type) {
            case GL_FLOAT:      u.numFloats = 1; break;
            case GL_FLOAT_VEC2: u.numFloats = 2; break;
            case GL_FLOAT_VEC3: u.numFloats = 3; break;
            case GL_FLOAT_VEC4: u.numFloats = 4; break;
                //case GL_FLOAT_MAT3: u.numFloats = 12; break;
            case GL_FLOAT_MAT4: u.numFloats = 16; break;
            default: continue;
            }

            uniformInfo.push_back(u);
        }
    }

    for (int i = 0; i < Attrib_Count; i++)
        attribLocations.push_back(glGetAttribLocation(progId, attribName[i]));

    return true;
}

bool ShaderSet_GL::SetUniform(const char *name, uint32_t numFloats, const float *v)
{
    for (const auto &uniform : uniformInfo) {
        if (!strcmp(uniform.name.c_str(), name)) {
            glUseProgram(progId);

            switch (uniform.numFloats) {
            case 1:   glUniform1fv(uniform.location, numFloats, v); break;
            case 2:   glUniform2fv(uniform.location, numFloats / 2, v); break;
            case 3:   glUniform3fv(uniform.location, numFloats / 3, v); break;
            case 4:   glUniform4fv(uniform.location, numFloats / 4, v); break;
                //case 12:  glUniformMatrix3fv(uniform.location, 1, 1, v); break;
            case 16:  glUniformMatrix4fv(uniform.location, 1, GL_FALSE, v); break;
            default: assert(0);
            }

            return true;
        }
    }

    DEBUG_LOG_TEXT("Warning: uniform %s not present in selected shader", name);
    return 0;
}

RenderDevice_GL::RenderDevice_GL()
{
    glDebugMessageCallback(DebugOutputCallback, NULL);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

    glGenVertexArrays(1, &vaoId);

    // compile builtin shaders
    for (uint32_t i = 0; i < VShader_Count; i++)
        vertexShaders[i] = std::make_shared<Shader_GL>(Shader_Vertex, vertexShaderSources[i]);

    for (uint32_t i = 0; i < FShader_Count; i++)
        fragmentShaders[i] = std::make_shared<Shader_GL>(Shader_Fragment, fragmentShaderSources[i]);
}

RenderDevice_GL::~RenderDevice_GL()
{
    glDeleteVertexArrays(1, &vaoId);
}

std::shared_ptr<Buffer> RenderDevice_GL::CreateBuffer(BufferUsage usage, const void *buf, size_t bufSize)
{
    auto buffer = std::make_shared<Buffer_GL>();
    buffer->SetData(usage, buf, bufSize);
    return buffer;
}

std::shared_ptr<ShaderSet> RenderDevice_GL::CreateShaderSet(std::initializer_list<std::shared_ptr<Shader>> shaderList)
{
    auto shaderSet = std::make_shared<ShaderSet_GL>();

    for (auto &shader : shaderList)
        shaderSet->SetShader(shader);

    return shaderSet;
}

void RenderDevice_GL::Clear(float r, float g, float b, float a, float depth, bool clearColor, bool clearDepth)
{
    glClearColor(r, g, b, a);
    glClearDepth(depth);
    glClear((clearColor ? GL_COLOR_BUFFER_BIT : 0) |
            (clearDepth ? GL_DEPTH_BUFFER_BIT : 0));
}

void RenderDevice_GL::Render(const Surface *surf, const glm::mat4 &transform)
{
    assert(surf);

    glBindVertexArray(vaoId);

    std::shared_ptr<ShaderSet_GL> shader = std::dynamic_pointer_cast<ShaderSet_GL>(surf->shader);
    glUseProgram(shader->progId);
    shader->SetUniform4x4f("u_proj", projection);
    shader->SetUniform4x4f("u_modelView", transform);

    const SurfaceGeometry *geo = &surf->geometry;
    SetCullFlags(geo->culling, geo->winding);

    std::shared_ptr<Buffer_GL> vbo = std::dynamic_pointer_cast<Buffer_GL>(geo->vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vbo->bufferId);

    const InputLayout *inputLayout = &geo->inputLayout;
    GLsizei layoutStride = inputLayout->GetStride();
    for (uint32_t i = 0; i < inputLayout->numElements; i++) {
        const InputElement *elem = &inputLayout->elements[i];
        const DataFormatInfo *fmt = &formatInfo[elem->format];

        GLint location = shader->attribLocations[elem->attribute];
        glEnableVertexAttribArray(location);
        glVertexAttribPointer(location, fmt->size, fmt->type, fmt->normalized, layoutStride, (void *)elem->alignedOffset);
    }

    std::shared_ptr<Buffer_GL> ibo = std::dynamic_pointer_cast<Buffer_GL>(geo->indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo->bufferId);
    glDrawElements(glPrimType[geo->prim], geo->indexCount, GL_UNSIGNED_SHORT, NULL);

    for (uint32_t i = 0; i < inputLayout->numElements; i++) {
        const InputElement *elem = &inputLayout->elements[i];
        GLint location = shader->attribLocations[elem->attribute];
        glDisableVertexAttribArray(location);
    }
}

} // renderer