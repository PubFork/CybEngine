#include "Precompiled.h"
#include "Definitions.h"
#include "RenderDevice.h"
#include "RenderDeviceOpenGL.h"
#include "Base/Debug.h"
#include "Base/Algorithm.h"

namespace renderer
{

static const OpenGLTextureFormatInfo pixelFormats[PixelFormat_Count] = 
{
//    internalFormat        format              type                compressed
    { GL_NONE,              GL_NONE,            GL_NONE,            GL_FALSE },     // PixelFormat_Unknown
    { GL_RGBA8,             GL_RGBA,            GL_UNSIGNED_BYTE,   GL_FALSE },     // PixelFormat_R8G8B8A8
    { GL_R8,                GL_RED,             GL_UNSIGNED_BYTE,   GL_FALSE },     // PixelFormat_R8
    { GL_RGBA32F,           GL_RGBA,            GL_FLOAT,           GL_FALSE },     // PixelFormat_R32G32B32A32F
    { GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_FLOAT,           GL_FALSE },     // PixelFormat_Depth24
};

static const OpenGLVertexElementUsageInfo vertexElementUsageInfo[VertexElementUsage_Count] =
{
//    attribLocation        attribName
    { 0,                    "a_position"  },                                        // VertexElementUsage_Position
    { 1,                    "a_normal"    },                                        // VertexElementUsage_Normal
    { 7,                    "a_tangent"   },                                        // VertexElementUsage_Tangent
    { 2,                    "a_texCoord0" },                                        // VertexElementUsage_TexCoord0
    { 3,                    "a_texCoord1" },                                        // VertexElementUsage_TexCoord1
    { 4,                    "a_texCoord2" },                                        // VertexElementUsage_TexCoord2
    { 5,                    "a_texCoord3" },                                        // VertexElementUsage_TexCoord3
    { 6,                    "a_color"     }                                         // VertexElementUsage_Color
};

static const OpenGLVertexElementFormatInfo vertexElementTypeInfo[VertexElementFormat_Count] =
{
//    elementType           numComponents       alignedSíze         normalized
    { GL_FLOAT,             1,                  4,                  GL_FALSE },     // VertexElementFormat_Float1
    { GL_FLOAT,             2,                  8,                  GL_FALSE },     // VertexElementFormat_Float2
    { GL_FLOAT,             3,                  12,                 GL_FALSE },     // VertexElementFormat_Float3
    { GL_FLOAT,             4,                  16,                 GL_FALSE },     // VertexElementFormat_Float4
    { GL_UNSIGNED_BYTE,     4,                  4,                  GL_FALSE },     // VertexElementFormat_UByte4
    { GL_UNSIGNED_BYTE,     4,                  4,                  GL_TRUE  },     // VertexElementFormat_UByte4N
    { GL_UNSIGNED_SHORT,    2,                  4,                  GL_FALSE },     // VertexElementFormat_Short2
    { GL_UNSIGNED_SHORT,    4,                  8,                  GL_FALSE },     // VertexElementFormat_Short4
};

static const GLenum compareFunctionTable[] =
{
    GL_INVALID_ENUM,
    GL_LESS,
    GL_LEQUAL,
    GL_GREATER,
    GL_GEQUAL,
    GL_EQUAL,
    GL_NOTEQUAL,
    GL_NEVER,
    GL_ALWAYS
};

static const GLenum cullFaceTable[] =
{
    GL_INVALID_ENUM,
    GL_BACK,                                                                        // DrawState_Cull_CW
    GL_FRONT,                                                                       // DrawState_Cull_CCW
};

static const OpenGLPrimitiveInfo primitiveTable[] =
{
//    type                  div     sub
    { GL_TRIANGLES,         3,      0 },                                            // Default
    { GL_TRIANGLE_STRIP,    1,      2 },                                            // DrawState_Primitive_TriangleStrip
    { GL_LINES,             2,      0 },                                            // DrawState_Primitive_Lines
    { GL_LINE_STRIP,        1,      1 },                                            // DrawState_Primitive_LineStrip
    { GL_POINTS,            1,      0 },                                            // DrawState_Primitive_Points
};

//
// OpenGL Buffer
//
OpenGLBuffer::~OpenGLBuffer()
{
    glDeleteBuffers(1, &resource);
}

void *OpenGLBuffer::Map()
{
    assert(usage == GL_DYNAMIC_DRAW);
    glBindBuffer(target, resource);
    return glMapBuffer(target, GL_WRITE_ONLY);
}

void OpenGLBuffer::Unmap()
{
    glBindBuffer(target, resource);
    glUnmapBuffer(target);
}

//
// OpenGL shader compiler
//
OpenGLShaderCompiler::~OpenGLShaderCompiler()
{
    std::for_each(std::begin(compiledShaderStages), std::end(compiledShaderStages), [&](auto &shader) { glDeleteShader(shader); });
}

bool OpenGLShaderCompiler::CompileShaderStage(GLenum stage, const ShaderBytecode &bytecode)
{
    if (compileErrorFlag)
    {
        return false;
    }

    GLuint shader = glCreateShader(stage);
    glShaderSource(shader, 1, (const GLchar **)&bytecode.source, (const GLint *)&bytecode.length);
    glCompileShader(shader);

    GLint compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled)
    {
        GLchar infoLog[InfoLogSize];
        glGetShaderInfoLog(shader, (GLsizei)InfoLogSize, 0, infoLog);
        DEBUG_LOG_TEXT_COND(infoLog[0], "Compiling shader:\n\n%s\nFailed: %s", bytecode.source, infoLog);
        compileErrorFlag = true;
        return false;
    }

    compiledShaderStages.push_back(shader);
    return true;
}

bool OpenGLShaderCompiler::LinkAndClearShaderStages(GLuint &outProgram)
{
    if (compileErrorFlag || compiledShaderStages.empty())
    {
        return false;
    }

    GLuint program = glCreateProgram();
    std::for_each(std::begin(compiledShaderStages), std::end(compiledShaderStages), [&](auto &shader) { glAttachShader(program, shader); });

    for (uint32_t i = 0; i < VertexElementUsage_Count; i++)
    {
        const OpenGLVertexElementUsageInfo *usageInfo = &vertexElementUsageInfo[i];
        glBindAttribLocation(program, usageInfo->attribLocation, usageInfo->attribName);
    }

    glLinkProgram(program);
    std::for_each(std::begin(compiledShaderStages), std::end(compiledShaderStages), [&](auto &shader) { glDetachShader(program, shader); glDeleteShader(shader); });
    compiledShaderStages.clear();

    GLint linked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        GLchar infoLog[InfoLogSize];
        glGetProgramInfoLog(program, InfoLogSize, 0, infoLog);
        DEBUG_LOG_TEXT_COND(infoLog[0], "Linking shaders failed: %s", infoLog);
        return false;
    }

    outProgram = program;
    return true;
}

//
// OpenGL Shader Program
//
OpenGLShaderProgram::~OpenGLShaderProgram()
{
    glDeleteProgram(resource);
}

int32_t OpenGLShaderProgram::GetParameterLocation(const char *name)
{
    return glGetUniformLocation(resource, name);
}

void OpenGLShaderProgram::SetBool(int32_t location, bool value)
{
    glProgramUniform1i(resource, location, value);
}

void OpenGLShaderProgram::SetFloat(int32_t location, const float value)
{
    glProgramUniform1f(resource, location, value);
}

void OpenGLShaderProgram::SetVec3(int32_t location, const float *values)
{
    glProgramUniform3fv(resource, location, 1, values);
}

void OpenGLShaderProgram::SetMat3(int32_t location, const float *values)
{
    glProgramUniformMatrix3fv(resource, location, 1, GL_FALSE, values);
}

void OpenGLShaderProgram::SetMat4(int32_t location, const float *values)
{
    glProgramUniformMatrix4fv(resource, location, 1, GL_FALSE, values);
}

//
// OpenGL Sampler State
//
OpenGLSamplerState::~OpenGLSamplerState()
{
    glDeleteSamplers(1, &resource);
}

//==============================
// OpenGL Image
//==============================

template <class BaseType>
OpenGLTextureBase<BaseType>::~OpenGLTextureBase()
{
    glDeleteTextures(1, &resource);
}

//==============================
// Render Device
//==============================


GLint TranslateWrapMode(SamplerWrapMode mode)
{
    switch (mode)
    {
    case SamplerWrap_Repeat: return GL_REPEAT;
    case SamplerWrap_RepeatMirror: return GL_MIRRORED_REPEAT;
    case SamplerWrap_Clamp: return GL_CLAMP_TO_EDGE;
    }

    return GL_REPEAT;
}

void GLAPIENTRY DebugOutputCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei /*length*/, const GLchar *message, const void * /*userParam*/)
{
    static std::unordered_map<GLenum, const char *> toString = 
    {
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

void OpenGLRenderDevice::Init()
{
    assert(isInitialized == false);

    glewExperimental = true;
    const GLenum err = glewInit();
    THROW_FATAL_COND(err != GLEW_OK, std::string("glew init: ") + (char *)glewGetErrorString(err));

    // show driver and library versions
    DEBUG_LOG_TEXT("Using OpenGL version %s", glGetString(GL_VERSION));
    DEBUG_LOG_TEXT("Shader language %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
    DEBUG_LOG_TEXT("GLEW %s", glewGetString(GLEW_VERSION));

    // show GPU memory on supported devices
    if (GL_NVX_gpu_memory_info)
    {
        GLint totalMemKb = 0;
        GLint avialableMemKb = 0;
        glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &totalMemKb);
        glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &avialableMemKb);
        DEBUG_LOG_TEXT("Total / Available GPU memory: %dKb / %dKb", totalMemKb, avialableMemKb);
    }
   
    // enable opengl debug output
    glDebugMessageCallback(DebugOutputCallback, NULL);
    glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_OTHER, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

    // initialize gl context state
    GLint maxTextureUnits = 0;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureUnits);
    DEBUG_LOG_TEXT("Max texture units: %d", maxTextureUnits);

    glGenVertexArrays(1, &vaoId);

    // setup default states
    glFrontFace(GL_CCW);

   // glEnable(GL_MULTISAMPLE);

    // set default filter mode to anisotropic with max anisotropy
    glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, (GLint *)&imageFilterMaxAnisotropy);
    DEBUG_LOG_TEXT("Max texture anisotropy: %d", imageFilterMaxAnisotropy);
    const SamplerStateInitializer initializer(SamplerFilter_Anisotropic, SamplerWrap_Repeat, SamplerWrap_Repeat, SamplerWrap_Repeat, imageFilterMaxAnisotropy);
    const std::shared_ptr<ISamplerState> samplerState = CreateSamplerState(initializer);
    SetSamplerState(0, samplerState);
    SetSamplerState(1, samplerState);
    SetSamplerState(2, samplerState);
    SetSamplerState(3, samplerState);

    isInitialized = true;
}

void OpenGLRenderDevice::Shutdown()
{
    if (isInitialized)
    {
        glDeleteVertexArrays(1, &vaoId);
        isInitialized = false;
    }
}

std::shared_ptr<IBuffer> OpenGLRenderDevice::CreateBuffer(int usageFlags, const void *data, size_t size)
{
    GLenum target = GL_INVALID_ENUM;
    switch (usageFlags & Buffer_TypeMask)
    {
    case Buffer_Vertex: target = GL_ARRAY_BUFFER; break;
    case Buffer_Index:  target = GL_ELEMENT_ARRAY_BUFFER; break;
    }

    const GLenum usage = (usageFlags & Buffer_ReadOnly) ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;
    
    GLuint resource = 0;
    glCreateBuffers(1, &resource);
    glBindBuffer(target, resource);
    glBufferData(target, size, data, usage);

    return std::make_shared<OpenGLBuffer>(usageFlags, resource, target, usage, size);
}

std::shared_ptr<IVertexDeclaration> OpenGLRenderDevice::CreateVertexDelclaration(const VertexElementList &vertexElements)
{
    const auto searchResult = vertexDeclarationCache.find(vertexElements);
    if (searchResult != vertexDeclarationCache.end())
    {
        return searchResult->second;
    }

    OpenGLVertexElementList openGLVertexElements;
    for (const auto &element : vertexElements)
    {
        const OpenGLVertexElementUsageInfo *useInfo = &vertexElementUsageInfo[element.usage];
        const OpenGLVertexElementFormatInfo *typeInfo = &vertexElementTypeInfo[element.format];
        openGLVertexElements.emplace_back(
            useInfo->attribLocation,
            typeInfo->numComponents,
            typeInfo->elementType,
            typeInfo->normalized ? GL_TRUE : GL_FALSE,
            static_cast<GLsizei>(element.stride),
            static_cast<GLuint>(element.alignedOffset));
    }

    auto vertexDeclaration =  std::make_shared<OpenGLVertexDeclaration>(openGLVertexElements);
    vertexDeclarationCache[vertexElements] = vertexDeclaration;

    return vertexDeclaration;
}

std::shared_ptr<IShaderProgram> OpenGLRenderDevice::CreateShaderProgram(const ShaderBytecode &VS, const ShaderBytecode &FS)
{
    OpenGLShaderCompiler compiler;
    compiler.CompileShaderStage(GL_VERTEX_SHADER, VS);
    compiler.CompileShaderStage(GL_FRAGMENT_SHADER, FS);

    GLuint programId = 0;
    const bool validProgram = compiler.LinkAndClearShaderStages(programId);
    return validProgram ? std::make_shared<OpenGLShaderProgram>(programId) : nullptr;
}

std::shared_ptr<IShaderProgram> OpenGLRenderDevice::CreateShaderProgram(const ShaderBytecode &VS, const ShaderBytecode &GS, const ShaderBytecode &FS)
{
    OpenGLShaderCompiler compiler;
    compiler.CompileShaderStage(GL_VERTEX_SHADER, VS);
    compiler.CompileShaderStage(GL_GEOMETRY_SHADER, GS);
    compiler.CompileShaderStage(GL_FRAGMENT_SHADER, FS);

    GLuint programId = 0;
    const bool validProgram = compiler.LinkAndClearShaderStages(programId);
    return validProgram ? std::make_shared<OpenGLShaderProgram>(programId) : nullptr;
}

void OpenGLRenderDevice::SetShaderProgram(const std::shared_ptr<IShaderProgram> program)
{
    currentShaderProgram = std::static_pointer_cast<OpenGLShaderProgram>(program);
    glUseProgram(currentShaderProgram->resource);
}

std::shared_ptr<ITexture2D> OpenGLRenderDevice::CreateTexture2D(int32_t width, int32_t height, PixelFormat format, int32_t numMipMaps, const void *data)
{
    const OpenGLTextureFormatInfo *formatInfo = &pixelFormats[format];
    const GLenum target = GL_TEXTURE_2D;
    GLuint textureId = 0;
    
    glCreateTextures(target, 1, &textureId);
    glBindTexture(target, textureId);

    glTextureParameteri(textureId, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(textureId, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(textureId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(textureId, GL_TEXTURE_MIN_FILTER, numMipMaps > 1 ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST);

    glTexImage2D(target, 0, formatInfo->internalFormat, width, height, 0, formatInfo->format, formatInfo->type, data);
    glGenerateMipmap(target);

    auto texture = std::make_shared<OpenGLTexture2D>(
        textureId,
        target,
        width,
        height,
        CalculateNumMipLevels(width, height),
        format);
    return texture;
}

std::shared_ptr<ITextureCube> OpenGLRenderDevice::CreateTextureCube(int32_t width, int32_t height, PixelFormat format, const void *data[])
{
    const OpenGLTextureFormatInfo *formatInfo = &pixelFormats[format];
    const GLenum target = GL_TEXTURE_CUBE_MAP;
    GLuint textureId = 0;

    glCreateTextures(target, 1, &textureId);
    glBindTexture(target, textureId);

    glTextureParameteri(textureId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(textureId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(textureId, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTextureParameteri(textureId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(textureId, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    for (uint32_t i = 0; i < 6; i++)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, formatInfo->internalFormat, width, height, 0, formatInfo->format, formatInfo->type, data[i]);
    }

    glGenerateMipmap(target);

    auto texture = std::make_shared<OpenGLTextureCube>(
        textureId,
        target,
        width,
        height,
        CalculateNumMipLevels(width, height),
        format);
    return texture;
}

void OpenGLRenderDevice::SetTexture(uint32_t textureIndex, const std::shared_ptr<ITexture> texture)
{
    glActiveTexture(GL_TEXTURE0 + textureIndex);
    if (texture != nullptr)
    {
        std::shared_ptr<OpenGLTexture2D> glTexture = std::static_pointer_cast<OpenGLTexture2D>(texture);
        glBindTexture(glTexture->target, glTexture->resource);
    }
    else
    {
        // TODO: check context state for what target type is currently bound,
        // and use that to reset the GL drivers texture state.
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

std::shared_ptr<ISamplerState> OpenGLRenderDevice::CreateSamplerState(const SamplerStateInitializer &initializer)
{
    const auto searchResult = samplerStateCache.find(initializer);
    if (searchResult != samplerStateCache.end())
    {
        return searchResult->second;
    }

    auto samplerState = std::make_shared<OpenGLSamplerState>();

    samplerState->wrapS = TranslateWrapMode(initializer.wrapU);
    samplerState->wrapT = TranslateWrapMode(initializer.wrapV);
    samplerState->wrapR = TranslateWrapMode(initializer.wrapW);
    samplerState->LODBias = initializer.mipBias;

    switch (initializer.filter)
    {
    case SamplerFilter_Anisotropic:
        samplerState->magFilter = GL_LINEAR;
        samplerState->minFilter = GL_LINEAR_MIPMAP_LINEAR;
        samplerState->maxAnisotropy = Clamp(initializer.maxAnisotropy, UINT32_C(1), imageFilterMaxAnisotropy);
        break;
    case SamplerFilter_Trilinear:
        samplerState->magFilter = GL_LINEAR;
        samplerState->minFilter = GL_LINEAR_MIPMAP_LINEAR;
        samplerState->maxAnisotropy = 0;
        break;
    case SamplerFilter_Bilinear:
        samplerState->magFilter = GL_LINEAR;
        samplerState->minFilter = GL_LINEAR_MIPMAP_NEAREST;
        samplerState->maxAnisotropy = 0;
        break;
    case SamplerFilter_Point:
        samplerState->magFilter = GL_NEAREST;
        samplerState->minFilter = GL_NEAREST_MIPMAP_NEAREST;
        samplerState->maxAnisotropy = 0;
        break;
    }

    glGenSamplers(1, &samplerState->resource);
    glSamplerParameteri(samplerState->resource, GL_TEXTURE_WRAP_S, samplerState->wrapS);
    glSamplerParameteri(samplerState->resource, GL_TEXTURE_WRAP_T, samplerState->wrapT);
    glSamplerParameteri(samplerState->resource, GL_TEXTURE_WRAP_R, samplerState->wrapR);
    glSamplerParameteri(samplerState->resource, GL_TEXTURE_LOD_BIAS, samplerState->LODBias);
    glSamplerParameteri(samplerState->resource, GL_TEXTURE_MAG_FILTER, samplerState->magFilter);
    glSamplerParameteri(samplerState->resource, GL_TEXTURE_MIN_FILTER, samplerState->minFilter);
    glSamplerParameteri(samplerState->resource, GL_TEXTURE_MAX_ANISOTROPY_EXT, samplerState->maxAnisotropy);

    samplerStateCache[initializer] = samplerState;
    return samplerState;
}

void OpenGLRenderDevice::SetSamplerState(uint32_t textureIndex, const std::shared_ptr<ISamplerState> state)
{
    assert(state);
    const std::shared_ptr<OpenGLSamplerState> samplerState = std::static_pointer_cast<OpenGLSamplerState>(state);
    glActiveTexture(GL_TEXTURE0 + textureIndex);
    glBindSampler(textureIndex, samplerState->resource);
}

void OpenGLRenderDevice::Clear(uint32_t targets, const glm::vec4 color, float depth)
{
    const GLbitfield mask = (targets & Clear_Color ? GL_COLOR_BUFFER_BIT : 0) |
        (targets & Clear_Depth ? GL_DEPTH_BUFFER_BIT : 0) |
        (targets & Clear_Stencil ? GL_STENCIL_BUFFER_BIT : 0);

    glClearColor(color.r, color.g, color.b, color.a);
    glClearDepth(depth);
    glClear(mask);
}

void OpenGLUpdateDrawState(uint32_t state)
{
    uint32_t depthMask = state & DrawState_DepthTest_Mask;
    if (!!depthMask)
    {
        const uint32_t compareFunctionIndex = depthMask >> DrawState_DepthTest_Shift;
        assert(compareFunctionIndex < _countof(compareFunctionTable));
        const GLenum depthCompareFunction = compareFunctionTable[compareFunctionIndex];

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(depthCompareFunction);
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
    }

    uint32_t cullMask = state & DrawState_Cull_Mask;
    if (!!cullMask)
    {
        const uint32_t cullFaceIndex = cullMask >> DrawState_Cull_Shift;
        assert(cullFaceIndex < _countof(cullFaceTable));
        const GLenum cullFace = cullFaceTable[cullFaceIndex];

        glEnable(GL_CULL_FACE);
        glCullFace(cullFace);
    }
    else
    {
        glDisable(GL_CULL_FACE);
    }
}

void OpenGLRenderDevice::Render(const Surface *surf, const ICamera *camera)
{
    assert(surf);
    assert(camera);
    assert(currentShaderProgram);

    glBindVertexArray(vaoId);

    // setup transformation matrices
    // TODO: add support for model matrices (this is kinda hacked together atm)
    int32_t projMatrixLoc = currentShaderProgram->GetParameterLocation("u_projMatrix");
    int32_t viewMatrixLoc = currentShaderProgram->GetParameterLocation("u_viewMatrix");
    int32_t modelViewMatrixLoc = currentShaderProgram->GetParameterLocation("u_modelViewMatrix");

    int32_t skyboxViewMatrixLoc = currentShaderProgram->GetParameterLocation("skyboxViewMatrix");
    if (skyboxViewMatrixLoc != -1)
    {
        glm::mat4 skyboxViewMatrix = glm::mat4(glm::mat3(glm::make_mat4(camera->GetViewMatrix())));
        currentShaderProgram->SetMat4(skyboxViewMatrixLoc, glm::value_ptr(skyboxViewMatrix));
    }

    currentShaderProgram->SetMat4(projMatrixLoc, camera->GetProjMatrix());
    currentShaderProgram->SetMat4(viewMatrixLoc, camera->GetViewMatrix());
    currentShaderProgram->SetMat4(modelViewMatrixLoc, camera->GetViewMatrix());

    int32_t viewPosLoc = currentShaderProgram->GetParameterLocation("u_viewPos");
    currentShaderProgram->SetVec3(viewPosLoc, camera->GetViewPositionVector());

    // setup states
    OpenGLUpdateDrawState(surf->drawStateFlags);

    // setup materials
    const SurfaceMaterial *material = &surf->material;
    currentShaderProgram->SetVec3(currentShaderProgram->GetParameterLocation("Ka"), glm::value_ptr(material->ambient));
    currentShaderProgram->SetVec3(currentShaderProgram->GetParameterLocation("Kd"), glm::value_ptr(material->diffuse));
    currentShaderProgram->SetVec3(currentShaderProgram->GetParameterLocation("Ks"), glm::value_ptr(material->specular));
    currentShaderProgram->SetFloat(currentShaderProgram->GetParameterLocation("Ns"), material->shininess);

    if (material->texture[0])
    {
        if (material->sampler[0])
        {
            SetSamplerState(0, material->sampler[0]);
        }

        SetTexture(0, material->texture[0]);    // TODO: Texture state should be manually set by the user!
        glProgramUniform1i(currentShaderProgram->resource, currentShaderProgram->GetParameterLocation("diffuseTexture"), 0);
    }

    if (material->texture[1])
    {
        if (material->sampler[0])
        {
            SetSamplerState(1, material->sampler[0]);
        }

        currentShaderProgram->SetBool(currentShaderProgram->GetParameterLocation("useSpecularTexture"), true);
        SetTexture(1, material->texture[1]);
        glProgramUniform1i(currentShaderProgram->resource, currentShaderProgram->GetParameterLocation("specularTexture"), 1);

    }

    if (material->texture[2])
    {
        if (material->sampler[0])
        {
            SetSamplerState(2, material->sampler[0]);
        }

        currentShaderProgram->SetBool(currentShaderProgram->GetParameterLocation("useNormalTexture"), true);
        SetTexture(2, material->texture[2]);
        glProgramUniform1i(currentShaderProgram->resource, currentShaderProgram->GetParameterLocation("normalTexture"), 2);
    }
    else
    {
        currentShaderProgram->SetBool(currentShaderProgram->GetParameterLocation("useNormalTexture"), false);
    }

    // setup geometry
    const std::shared_ptr<OpenGLBuffer> VBO = std::static_pointer_cast<OpenGLBuffer>(surf->vertexBuffer);
    glBindBuffer(VBO->target, VBO->resource);

    const std::shared_ptr<OpenGLVertexDeclaration> vertexDeclaration = std::static_pointer_cast<OpenGLVertexDeclaration>(surf->vertexDeclaration);
    for (const auto &element : vertexDeclaration->vertexElements)
    {
        glEnableVertexAttribArray(element.attributeLocation);
        glVertexAttribPointer(element.attributeLocation, element.numComponents, element.type, element.normalized, element.stride, (const GLvoid *)element.offset);
    }

    const uint32_t primIndex = (surf->drawStateFlags & DrawState_Primitive_Mask) >> DrawState_Primitive_Shift;
    assert(primIndex < _countof(primitiveTable));
    const OpenGLPrimitiveInfo &primitiveInfo = primitiveTable[primIndex];

    if (surf->indexBuffer != nullptr)
    {
        const std::shared_ptr<OpenGLBuffer> IBO = std::static_pointer_cast<OpenGLBuffer>(surf->indexBuffer);
        glBindBuffer(IBO->target, IBO->resource);

        int32_t numIndices = surf->numIndices;
        if (numIndices == UINT32_MAX)
        {
            numIndices = IBO->size / sizeof(uint32_t);
        }

        GLenum indexType = (IBO->flags & Buffer_32BitIndex) ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT;
        glDrawElements(primitiveInfo.type, numIndices, indexType, NULL);
    }
    else
    {
        int32_t numVertices = surf->numVertices;
        if (numVertices == UINT32_MAX)
        {
            numVertices = VBO->size / vertexDeclaration->vertexElements[0].stride;
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glDrawArrays(primitiveInfo.type, 0, numVertices);
    }
    
    for (const auto &element : vertexDeclaration->vertexElements)
    {
        glDisableVertexAttribArray(element.attributeLocation);
    }
}

std::shared_ptr<IRenderDevice> CreateRenderDevice()
{
    return std::make_shared<OpenGLRenderDevice>();
}

} // renderer