#include "stdafx.h"
#include <GL/glew.h>
#include <Windows.h>
#include "RenderDevice_GL.h"

#include "Base/Debug.h"
#include "Base/Profiler.h"

namespace renderer
{

/*********************************************************************/

static GLint maxTextureAnisotroy = 1;

/*********************************************************************/

void GLAPIENTRY DebugOutputCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei /*length*/, const GLchar *message, const void * /*userParam*/)
{
    static std::unordered_map<GLenum, const char *> toString = {
        { GL_DEBUG_SOURCE_API_ARB,                  "OpenGL"                },
        { GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB,        "Windows"               },
        { GL_DEBUG_SOURCE_SHADER_COMPILER_ARB,      "Shader compiler"       },
        { GL_DEBUG_SOURCE_THIRD_PARTY_ARB,          "Third party"           },
        { GL_DEBUG_SOURCE_APPLICATION_ARB,          "Application"           },
        { GL_DEBUG_SOURCE_OTHER_ARB,                "Other"                 },
        { GL_DEBUG_TYPE_ERROR_ARB,                  "Error"                 },
        { GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB,    "Deprecated behaviour"  },
        { GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB,     "Undefined behaviour"   },
        { GL_DEBUG_TYPE_PORTABILITY_ARB,            "Portability"           },
        { GL_DEBUG_TYPE_PERFORMANCE_ARB,            "Performence"           },
        { GL_DEBUG_TYPE_MARKER,                     "Marker"                },
        { GL_DEBUG_TYPE_PUSH_GROUP,                 "Push group"            },
        { GL_DEBUG_TYPE_POP_GROUP,                  "Pop group"             },
        { GL_DEBUG_TYPE_OTHER_ARB,                  "Other"                 },
        { GL_DEBUG_SEVERITY_HIGH_ARB,               "High"                  },
        { GL_DEBUG_SEVERITY_MEDIUM_ARB,             "Medium"                },
        { GL_DEBUG_SEVERITY_LOW_ARB,                "Low"                   },
        { GL_DEBUG_SEVERITY_NOTIFICATION,           "Notification"          }
    };

    DEBUG_LOG_TEXT("[driver] %s %s %#x %s: %s", toString[source], toString[type], id, toString[severity], message);
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

bool Buffer_GL::SetData(Type usage, const void *buffer, size_t bufSize)
{
    switch (usage)
    {
    case Index: target = GL_ELEMENT_ARRAY_BUFFER; break;
    default:    target = GL_ARRAY_BUFFER; break;
    }

    if (!bufferId)
        glCreateBuffers(1, &bufferId);
    size = bufSize;

    glBindBuffer(target, bufferId);
    glBufferData(target, size, buffer, GL_STATIC_DRAW);
    return true;
}

RenderDevice_GL::RenderDevice_GL()
{
    glewExperimental = true;
    GLenum err = glewInit();
    THROW_FATAL_COND(err != GLEW_OK, std::string("glew init: ") + (char *)glewGetErrorString(err));
    DEBUG_LOG_TEXT("Using OpenGL version %s", glGetString(GL_VERSION));
    DEBUG_LOG_TEXT("Shader language %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
    DEBUG_LOG_TEXT("GLEW %s", glewGetString(GLEW_VERSION));

    glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
    maxTextureAnisotroy = maxAnisotropy;
    DEBUG_LOG_TEXT("Max texture anisotropy: %d", maxAnisotropy);

    glDebugMessageCallback(DebugOutputCallback, NULL);
    glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_OTHER, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

    glGenVertexArrays(1, &vaoId);

    // setup default states
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
}

RenderDevice_GL::~RenderDevice_GL()
{
    glDeleteVertexArrays(1, &vaoId);
}

std::shared_ptr<Buffer> RenderDevice_GL::CreateBuffer(Buffer::Type usage, const void *buf, size_t bufSize)
{
    auto buffer = std::make_shared<Buffer_GL>();
    buffer->SetData(usage, buf, bufSize);
    return buffer;
}

void RenderDevice_GL::Clear(uint32_t targets, const glm::vec4 color, float depth)
{
    GLbitfield mask = 0;

    if (targets & Clear_Color)
    {
        glClearColor(color.r, color.g, color.b, color.a );
        mask |= GL_COLOR_BUFFER_BIT;
    }

    if (targets & Clear_Depth)
    {
        glClearDepth(depth);
        mask |= GL_DEPTH_BUFFER_BIT;
    }

    if (targets & Clear_Stencil)
    {
        mask |= GL_STENCIL_BUFFER_BIT;
    }

    glClear(mask);
}

void RenderDevice_GL::Render(const Surface *surf, const glm::mat4 &transform)
{
    assert(surf);

    SCOOPED_PROFILE_EVENT("Render_Surface");

    glBindVertexArray(vaoId);

    // setup material & pipeline state
    const SurfaceMaterial *material = &surf->material;
    PipelineState *pipelineState = surf->pipelineState;
    pipelineState->Bind();
    pipelineState->SetParamMat4(renderer::Param_Proj, glm::value_ptr(projection));
    pipelineState->SetParamMat4(renderer::Param_ModelView, glm::value_ptr(transform));

    if (material->texture[0])
        material->texture[0]->Bind(0);

    // setup geometry
    const SurfaceGeometry *geo = &surf->geometry;
    
    // draw
    std::shared_ptr<Buffer_GL> vbo = std::dynamic_pointer_cast<Buffer_GL>(geo->vertexBuffer);
    std::shared_ptr<Buffer_GL> ibo = std::dynamic_pointer_cast<Buffer_GL>(geo->indexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vbo->bufferId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo->bufferId);

    BindVertexLayout(pipelineState->GetVertexLayout());
    glDrawElements(pipelineState->GetGLPrimType(), geo->indexCount, GL_UNSIGNED_SHORT, NULL);
    UnBindVertexLayout(pipelineState->GetVertexLayout());
}

std::shared_ptr<RenderDevice> CreateRenderDeviceGL()
{
    return std::make_shared<RenderDevice_GL>();
}

} // renderer