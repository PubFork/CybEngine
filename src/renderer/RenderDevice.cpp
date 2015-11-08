#include "stdafx.h"
#include "RenderDevice.h"

#include "core/Log.h"

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
    { 4, GL_UNSIGNED_BYTE, GL_TRUE  },      // Format_RGBA_UI8Norm
};

static const GLenum glPrimType[] = {
    GL_LINES,
    GL_TRIANGLES,
    GL_TRIANGLE_STRIP,
    GL_QUADS
};

static const char *vertexShaderSources[VShader_Count] = {
    vertexShaderMVSource,
    vertexShaderMVPSource
};

static const char *fragmentShaderSources[FShader_Count] = {
    fragmentShaderFlatSource,
    fragmentShaderGouraudSource
};

void GLAPIENTRY DebugOutputCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei /*length*/, const GLchar *message, const void * /*userParam*/)
{
    static std::unordered_map<GLenum, const char *> toString = {
        { GL_DEBUG_SOURCE_API_ARB               , "OpenGL"               },
        { GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB     , "Windows"              },
        { GL_DEBUG_SOURCE_SHADER_COMPILER_ARB   , "Shader compiler"      },
        { GL_DEBUG_SOURCE_THIRD_PARTY_ARB       , "Third party"          },
        { GL_DEBUG_SOURCE_APPLICATION_ARB       , "Application"          },
        { GL_DEBUG_SOURCE_OTHER_ARB             , "Other"                },
        { GL_DEBUG_TYPE_ERROR_ARB               , "Error"                },
        { GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB , "Deprecated behaviour" },
        { GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB  , "Undefined behaviour"  },
        { GL_DEBUG_TYPE_PORTABILITY_ARB         , "Portability"          },
        { GL_DEBUG_TYPE_PERFORMANCE_ARB         , "Performence"          },
        { GL_DEBUG_TYPE_MARKER                  , "Marker"               },
        { GL_DEBUG_TYPE_PUSH_GROUP              , "Push group"           },
        { GL_DEBUG_TYPE_POP_GROUP               , "Pop group"            },
        { GL_DEBUG_TYPE_OTHER_ARB               , "Other"                },
        { GL_DEBUG_SEVERITY_HIGH_ARB            , "High"                 },
        { GL_DEBUG_SEVERITY_MEDIUM_ARB          , "Medium"               },
        { GL_DEBUG_SEVERITY_LOW_ARB             , "Low"                  },
        { GL_DEBUG_SEVERITY_NOTIFICATION        , "Notification"         }
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

RenderDevice::RenderDevice()
{
    glDebugMessageCallback(DebugOutputCallback, NULL);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

    glGenVertexArrays(1, &vaoId);

    // compile builtin vertex shaders
    for (uint32_t i = 0; i < VShader_Count; i++) 
        vertexShaders[i] = std::make_shared<Shader>(Shader_Vertex, vertexShaderSources[i]);

    // compile builtin fragment shaders
    for (uint32_t i = 0; i < FShader_Count; i++)
        fragmentShaders[i] = std::make_shared<Shader >(Shader_Fragment, fragmentShaderSources[i]);
}

RenderDevice::~RenderDevice()
{
    glDeleteVertexArrays(1, &vaoId); 
}

std::shared_ptr<Buffer> RenderDevice::CreateBuffer(BufferUsage usage, const void *buf , size_t bufSize)
{
    auto buffer = std::make_shared<Buffer>();
    buffer->Data(usage, buf, bufSize);
    return buffer;
}

std::shared_ptr<ShaderSet> RenderDevice::CreateShaderSet(std::initializer_list<std::shared_ptr<Shader>> shaderList)
{
    auto shaderSet = std::make_shared<ShaderSet>();
    for (auto &shader : shaderList)
        shaderSet->SetShader(shader);
    return shaderSet;
}

std::shared_ptr<Shader> RenderDevice::LoadBuiltinShader(ShaderStage stage, BuiltinShaders shader)
{
    switch (stage) {
    case Shader_Vertex:   return vertexShaders[shader];
    case Shader_Fragment: return fragmentShaders[shader];
    default: break;
    }

    return nullptr;
}

void RenderDevice::SetProjection(const glm::mat4 &proj)
{
    projection = proj;
}

void RenderDevice::SetFillMode(FillMode mode)
{
    switch( mode) {
    case Fill_Solid: glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); break;
    case Fill_Wire:  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); break;
    case Fill_Point: glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); break;
    default: assert(0);
    }
}

void RenderDevice::Clear(float r, float g, float b, float a, float depth, bool clearColor, bool clearDepth)
{
    glClearColor(r, g, b, a);
    glClearDepth(depth);

    GLenum clearFlags = 0;
    clearFlags |= clearColor ? GL_COLOR_BUFFER_BIT : 0;
    clearFlags |= clearDepth ? GL_DEPTH_BUFFER_BIT : 0;
    glClear(clearFlags);
}

void RenderDevice::Render(const Surface *surf, const glm::mat4 &transform)
{
    assert(surf);

    glBindVertexArray(vaoId);

    const std::shared_ptr<ShaderSet> shader = surf->shader;
    glUseProgram(shader->progId);
    shader->SetUniform4x4f("u_proj", projection);
    shader->SetUniform4x4f("u_modelView", transform);

    const SurfaceGeometry *geo = &surf->geometry;
    SetCullFlags(geo->culling, geo->winding);

    glBindBuffer(GL_ARRAY_BUFFER, geo->vertexBuffer->Id());

    const InputLayout *inputLayout = &geo->inputLayout;
    GLsizei layoutStride = inputLayout->GetStride();
    for (uint32_t i = 0; i < inputLayout->numElements; i++) {
        const InputElement *elem = &inputLayout->elements[i];
        const DataFormatInfo *fmt = &formatInfo[elem->format];

        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, fmt->size, fmt->type, fmt->normalized, layoutStride, (void *)elem->alignedOffset);

    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geo->indexBuffer->Id());
    glDrawElements(glPrimType[geo->prim], geo->indexCount, GL_UNSIGNED_SHORT, NULL);
}

} // renderer