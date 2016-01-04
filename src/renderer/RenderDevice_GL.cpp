#include "stdafx.h"
#include <GL/glew.h>
#include <Windows.h>
#include <GL/glew.h>
#include "RenderDevice_GL.h"

#include "core/Log.h"

namespace renderer
{

/*********************************************************************
 *
 * Default shader sources
 *
 *********************************************************************/

#define VERTEX_SHADER_COMMON                    \
"#version 420 core\n"                           \
"uniform mat4 u_proj;\n"                        \
"uniform mat4 u_modelView;\n"                   \
"layout(location = 1) in vec3 a_position;\n"    \
"layout(location = 2) in vec3 a_normal;\n"      \
"layout(location = 3) in vec2 a_texCoord0;\n"   \
"layout(location = 4) in vec2 a_texCoord1;\n"   \
"layout(location = 5) in vec2 a_texCoord2;\n"   \
"layout(location = 6) in vec2 a_texCoord3;\n"   \
"layout(location = 7) in vec4 a_color0;\n"      \
"layout(location = 8) in vec4 a_color1;\n"      \
"out vec4 o_color0;\n"                          \
"out vec3 o_normal;\n"                          \
"out vec2 o_texCoord0;\n"                       \
"out vec2 o_texCoord1;\n"

static const char *vertexShaderMVPSource =
VERTEX_SHADER_COMMON
"void main()\n"
"{\n"
"   gl_Position = u_proj * (u_modelView * vec4(a_position, 1));\n"
"   o_color0 = a_color0;\n"
"   o_normal = vec3(u_modelView * vec4(a_normal, 0));\n"
"   o_texCoord0 = a_texCoord0;\n"
"   o_texCoord1 = a_texCoord1;\n"
"}\n";

static const char *fsFlatSource =
"#version 420 core\n"
"uniform vec4 u_color;\n"
"out vec4 fragColor;\n"
"void main()\n"
"{\n"
"   fragColor = u_color;\n"
"}\n";

static const char *fsGouraudSource =
"#version 420 core\n"
"in vec4 o_color0;\n"
"out vec4 fragColor;\n"
"void main()\n"
"{\n"
"   fragColor = o_color0;\n"
"}\n";

static const char *fsLitGouraudSource =
"#version 420 core\n"
"in vec4 o_color0;\n"
"in vec3 o_normal;\n"
"in vec2 o_texCoord0;\n"    //!
"out vec4 fragColor;\n"

"uniform sampler2D tex0;\n" //!

"struct DirectionalLight\n"
"{\n"
"    vec3 color;\n"
"    vec3 direction;\n"
"    float ambientIntensity;\n"
"};\n"

"void main()\n"
"{\n"
"   DirectionalLight light;\n"
"   light.color = vec3(1.0, 1.0, 1.0);\n"
"   light.direction = vec3(-1.0, -1.0, -1.0);\n"
"   light.ambientIntensity = 0.2;\n"
"   float diffuseIntensity = max(0.0, dot(normalize(o_normal), -normalize(light.direction)));\n"
"   //fragColor = o_color0 * vec4(light.color * (light.ambientIntensity + diffuseIntensity), 1.0);\n"
"   fragColor = texture(tex0, o_texCoord0)  * vec4(light.color * (light.ambientIntensity + diffuseIntensity), 1.0);\n"
"}\n";

static const char *vertexShaderSources[VShader_Count] = {
    vertexShaderMVPSource
};

static const char *fragmentShaderSources[FShader_Count] = {
    fsFlatSource,
    fsGouraudSource,
    fsLitGouraudSource
};

/*********************************************************************
 *
 * VertexFormat declatations
 *
 *********************************************************************/

struct InputElement
{
    GLint location;
    const char *attribute;
    uintptr_t offset;
    GLenum type;
    GLint size;
    GLboolean normalized;
};

struct InputLayout
{
    GLint stride;       // size in bytes
    const InputElement *elements;
    uint32_t numElements;
};

static const InputElement defaultVertexDecl[] =
{
    { 1, "a_position",  0, GL_FLOAT,         3, GL_FALSE },
    { 2, "a_normal",   12, GL_FLOAT,         3, GL_FALSE },
    { 3, "a_texCoord0",     24, GL_FLOAT,         2, GL_FALSE },
    { 4, "a_texCoord1",     32, GL_FLOAT,         2, GL_FALSE },
    { 5, "a_texCoord2",     40, GL_FLOAT,         2, GL_FALSE },
    { 6, "a_texCoord3",     48, GL_FLOAT,         2, GL_FALSE },
    { 7, "a_color0",   56, GL_UNSIGNED_BYTE, 4, GL_TRUE  },
    { 8, "a_color1",   60, GL_UNSIGNED_BYTE, 4, GL_TRUE  }
};

static const InputElement shadedTexVertexDecl[] =
{
    { 1, "a_position",  0, GL_FLOAT,         3, GL_FALSE },
    { 2, "a_normal",   12, GL_UNSIGNED_BYTE, 4, GL_TRUE  },
    { 3, "a_texCoord0",     24, GL_FLOAT,         2, GL_FALSE },
};

static const InputElement doubleTexVertexDecl[] =
{
    { 1, "a_position",  0, GL_FLOAT,         3, GL_FALSE },
    { 3, "a_texCoord0",     12, GL_UNSIGNED_BYTE, 4, GL_TRUE  },
    { 4, "a_texCoord1",     20, GL_FLOAT,         2, GL_FALSE },
    { 7, "a_color0",   28, GL_UNSIGNED_BYTE, 4, GL_TRUE  }
};

static const InputElement compactVertexDecl[] =
{
    { 1, "a_position",  0, GL_FLOAT,         3, GL_FALSE },
    { 7, "a_color0",   12, GL_UNSIGNED_BYTE, 4, GL_TRUE  }
};

static const InputLayout vertexLayouts[] =
{
    { 0,  nullptr,             0                             },
    { 64, defaultVertexDecl,   _countof(defaultVertexDecl)   },
    { 32, shadedTexVertexDecl, _countof(shadedTexVertexDecl) },
    { 32, doubleTexVertexDecl, _countof(doubleTexVertexDecl) },
    { 16, compactVertexDecl,   _countof(compactVertexDecl)   }
};

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

    DEBUG_LOG_TEXT("[glDebug] %s %s %#x %s: %s", toString[source], toString[type], id, toString[severity], message);
}

void SetCullFlags(CullMode culling, WindingOrder winding)
{
    switch (winding)
    {
    case Winding_CCW: glFrontFace(GL_CCW); break;
    case Winding_CW:  glFrontFace(GL_CW); break;
    default: assert(0);
    }

    switch (culling)
    {
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

Shader_GL::Shader_GL(Type type, const char *source) :
    Shader(type),
    shaderId(0)
{
    bool result = Compile(source);
    THROW_FATAL_COND(result != true, "Failed to compile shader source");
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
        GL_FRAGMENT_SHADER,
        GL_GEOMETRY_SHADER,
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
    if (!compiled)
    {
        GLchar infoLog[KILOBYTES(1)];
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

    if (shaders[Shader::Vertex] && shaders[Shader::Fragment])
    {
        bool result = Link();
        THROW_FATAL_COND(result != true, "Failed to link shader program.");
    }
}

void ShaderSet_GL::UnsetShader(Shader::Type stage)
{
    std::shared_ptr<Shader_GL> shader = std::dynamic_pointer_cast<Shader_GL>(shaders[stage]);

    if (shader)
    {
        glDetachShader(progId, shader->shaderId);
        shaders[stage] = nullptr;
    }
}

bool ShaderSet_GL::Link()
{
#ifdef _DEBUG
    DEBUG_LOG_TEXT("Linking shader program %d", progId);
#endif // _DEBUG

    glLinkProgram(progId);
    GLint linked = 0;
    glGetProgramiv(progId, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        GLchar infoLog[KILOBYTES(1)];
        glGetProgramInfoLog(progId, sizeof(infoLog), 0, infoLog);
        DEBUG_LOG_TEXT_COND(infoLog[0], "Linking shaders failed: %s", infoLog);
        return false;
    }

    glUseProgram(progId);

#ifdef _DEBUG
    GLint attribCount = 0;
    glGetProgramiv(progId, GL_ACTIVE_ATTRIBUTES, &attribCount);
    while (attribCount--)
    {
        GLint size = 0;
        GLenum type;
        GLchar name[32];
        glGetActiveAttrib(progId, attribCount, sizeof(name), 0, &size, &type, name);
        if (size)
            DEBUG_LOG_TEXT("layout(location = %d) in %s", glGetAttribLocation(progId, name), name);
    }
#endif // _DEBUG

    GLint uniformCount = 0;
    glGetProgramiv(progId, GL_ACTIVE_UNIFORMS, &uniformCount);
    for (GLint i = 0; i < uniformCount; i++)
    {
        GLint size = 0;
        GLenum type;
        GLchar name[32];
        glGetActiveUniform(progId, i, sizeof(name), 0, &size, &type, name);

        if (size)
        {
            Uniform u;
            u.name = name;
            u.location = glGetUniformLocation(progId, name);

            switch (type)
            {
            case GL_FLOAT:      u.numFloats = 1; break;
            case GL_FLOAT_VEC2: u.numFloats = 2; break;
            case GL_FLOAT_VEC3: u.numFloats = 3; break;
            case GL_FLOAT_VEC4: u.numFloats = 4; break;
                //case GL_FLOAT_MAT3: u.numFloats = 12; break;
            case GL_FLOAT_MAT4: u.numFloats = 16; break;
            default: continue;
            }

#ifdef _DEBUG
            DEBUG_LOG_TEXT("layout(location = %d) uniform %s", u.location, name);
#endif // _DEBUG

            uniformInfo.push_back(u);
        }
    }

    char textureName[16];
    for (int i = 0; i < MAX_TEXTURES; i++)
    {
        sprintf_s(textureName, "tex%d", i);
        textureLocation[i] = glGetUniformLocation(progId, textureName);
        glUniform1i(textureLocation[i], i);
    }

    return true;
}

bool ShaderSet_GL::SetUniformfv(const char *name, uint32_t numFloats, const float *v)
{
    for (const auto &uniform : uniformInfo)
    {
        if (!strcmp(uniform.name.c_str(), name))
        {
            glUseProgram(progId);

            switch (uniform.numFloats)
            {
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

GLTexture::GLTexture(uint32_t fmt, uint32_t w, uint32_t h) :
    format(fmt),
    width(w),
    height(h),
    textureId(0)
{
}

GLTexture::~GLTexture()
{
    if (textureId != 0)
        glDeleteTextures(1, &textureId);
}

void GLTexture::SetSampleMode(uint32_t sample)
{
    glBindTexture(GL_TEXTURE_2D, textureId);
    switch (sample & Sample_FilterMask)
    {
    case Sample_Linear:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1);
        break;

    case Sample_Anisotropic:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxTextureAnisotroy);
        break;

    case Sample_Nearest:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1);
        break;
    }

    switch (sample & Sample_AddressMask)
    {
    case Sample_Repeat:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        break;

    case Sample_Clamp:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        break;

    case Sample_ClampBorder:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        break;
    }
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

    // create default shader set
    SetDefaultShader(LoadBuiltinShaderSet(ShaderSet::Builtin_Color));

    // setup default states
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
}

RenderDevice_GL::~RenderDevice_GL()
{
    glDeleteVertexArrays(1, &vaoId);
}

std::shared_ptr<Shader> RenderDevice_GL::LoadBuiltinShader(Shader::Type stage, BuiltinShaders shader)
{
    if (stage == Shader::Vertex)
    {
        if (!vertexShaders[shader])
            vertexShaders[shader] = std::make_shared<Shader_GL>(Shader::Vertex, vertexShaderSources[shader]);
        return vertexShaders[shader];
    }

    if (stage == Shader::Fragment)
    {
        if (!fragmentShaders[shader])
            fragmentShaders[shader] = std::make_shared<Shader_GL>(Shader::Fragment, fragmentShaderSources[shader]);
        return fragmentShaders[shader];
    }

    return nullptr;
}

std::shared_ptr<Buffer> RenderDevice_GL::CreateBuffer(Buffer::Type usage, const void *buf, size_t bufSize)
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

std::shared_ptr<Texture> RenderDevice_GL::CreateTexture(uint32_t format, uint32_t width, uint32_t height, const void *data)
{
    GLenum glFormat = 0;
    switch (format)
    {
    case Texture_RGBA: glFormat = GL_RGBA; break;
    case Texture_BGRA: glFormat = GL_BGRA; break;
    default: assert(0); break; // unsupported format
    }

    auto texture = std::make_shared<GLTexture>(format, width, height);
    glGenTextures(1, &texture->textureId);
    glBindTexture(GL_TEXTURE_2D, texture->textureId);

    texture->SetSampleMode(Sample_Anisotropic | Sample_Repeat);

    glTexImage2D(GL_TEXTURE_2D, 0, glFormat, width, height, 0, glFormat, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    return texture;
}

void RenderDevice_GL::SetTexture(uint32_t slot, std::shared_ptr<Texture> tex)
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, std::dynamic_pointer_cast<GLTexture>(tex)->textureId);
}

void RenderDevice_GL::SetFillMode(FillMode mode)
{
    static const GLenum glFillMode[] = {
        GL_FILL,
        GL_LINE,
        GL_POINT
    };

    glPolygonMode(GL_FRONT_AND_BACK, glFillMode[mode]);
}

void RenderDevice_GL::Clear(int32_t flags, uint32_t color)
{
    float r, g, b, a;
    UnpackRGBA(color, r, g, b, a);
    glClearColor(r, g, b, a);

    GLbitfield mask = 0;
    if (flags & Clear_Color)
        mask |= GL_COLOR_BUFFER_BIT;
    if (flags & Clear_Depth)
        mask |= GL_DEPTH_BUFFER_BIT;
    if (flags & Clear_Stencil)
        mask |= GL_STENCIL_BUFFER_BIT;

    glClear(mask);
}

void RenderDevice_GL::Render(const Surface *surf, const glm::mat4 &transform)
{
    static const GLenum glPrimType[] = {
        GL_LINES,
        GL_TRIANGLES,
        GL_TRIANGLE_STRIP,
        GL_QUADS
    };

    assert(surf);

    glBindVertexArray(vaoId);

    // setup material
    const SurfaceMaterial *material = &surf->material;
    std::shared_ptr<ShaderSet_GL> shader = std::dynamic_pointer_cast<ShaderSet_GL>(material->shader);
    if (!shader)
        shader = std::dynamic_pointer_cast<ShaderSet_GL>(defaultShader);
    glUseProgram(shader->progId);

    shader->SetUniform4x4f("u_proj", projection);
    shader->SetUniform4x4f("u_modelView", transform);

    if (material->texture[0])
        SetTexture(0, material->texture[0]);

    // setup geometry
    const SurfaceGeometry *geo = &surf->geometry;
    SetCullFlags(geo->culling, geo->winding);

    std::shared_ptr<Buffer_GL> vbo = std::dynamic_pointer_cast<Buffer_GL>(geo->vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vbo->bufferId);

    const InputLayout *inputLayout = &vertexLayouts[geo->format];
    for (uint32_t i = 0; i < inputLayout->numElements; i++)
    {
        const InputElement *elem = &inputLayout->elements[i];

        glEnableVertexAttribArray(elem->location);
        glVertexAttribPointer(elem->location, elem->size, elem->type, elem->normalized, inputLayout->stride, (void *)elem->offset);
    }

    // draw
    std::shared_ptr<Buffer_GL> ibo = std::dynamic_pointer_cast<Buffer_GL>(geo->indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo->bufferId);
    glDrawElements(glPrimType[geo->prim], geo->indexCount, GL_UNSIGNED_SHORT, NULL);

    for (uint32_t i = 0; i < inputLayout->numElements; i++)
    {
        const InputElement *elem = &inputLayout->elements[i];
        glDisableVertexAttribArray(elem->location);
    }
}

std::shared_ptr<RenderDevice> CreateRenderDeviceGL()
{
    return std::make_shared<RenderDevice_GL>();
}

} // renderer