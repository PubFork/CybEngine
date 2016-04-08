#include "Precompiled.h"
#include "RenderDevice.h"
#include "RenderDeviceOpenGL.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Base/Debug.h"
#include "Base/MurmurHash.h"
#include "Base/FileUtils.h"

namespace renderer
{

static const OpenGLTextureFormat pixelFormats[PixelFormat_Count] = {
//    internalFormat        format              type                compressed
    { GL_NONE,              GL_NONE,            GL_NONE,            GL_FALSE },     // PixelFormat_Unknown
    { GL_RGBA8,             GL_RGBA,            GL_UNSIGNED_BYTE,   GL_FALSE },     // PixelFormat_R8G8B8A8
    { GL_R8,                GL_RED,             GL_UNSIGNED_BYTE,   GL_FALSE },     // PixelFormat_R8
    { GL_RGBA32F,           GL_RGBA,            GL_FLOAT,           GL_FALSE },     // PixelFormat_R32G32B32A32F
    { GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_FLOAT,           GL_FALSE },     // PixelFormat_Depth24
};

size_t SamplerStateInitializerHasher::operator()(const SamplerStateInitializer &state) const
{
    return CalculateMurmurHash(&state, sizeof(state));
}

//==============================
// OpenGL Buffer
//==============================

template <class BaseType>
OpenGLBufferBase<BaseType>::~OpenGLBufferBase()
{
    glDeleteBuffers(1, &resource);
}

//==============================
// OpenGL Image
//==============================

template <class BaseType>
OpenGLTextureBase<BaseType>::~OpenGLTextureBase()
{
    glDeleteTextures(1, &resource);
}

int CalculateNumMipLevels(uint32_t width, uint32_t height)
{
    int n = 1;
    while (width > 1 || height > 1)
    {
        width >>= 1;
        height >>= 1;
        n++;
    }

    return n;
}

//=========================================================================================//

static VertexInputElement standardVertexLayout[] =
{
    { "position",  1, VertexFormat_Float3, 0 },
    { "normal",    2, VertexFormat_Float3, 0 },
    { "texCoord0", 3, VertexFormat_Float2, 0 },
};

static VertexInputElement skydomeVertexLayout[] =
{
    { "position",  1, VertexFormat_Float3, 0 },
    { "texCoord0", 2, VertexFormat_Float2, 0 },
};

static const CreatePipelineStateInfo builtintPipelineStatesCreateInfo[BuiltintPipelineState_Count] = 
{
    {
        // Standard
        "assets/shaders/standard.vert",
        "assets/shaders/standard.frag",
        { standardVertexLayout , _countof(standardVertexLayout) },
        renderer::Raster_DefaultState
    },
    {   // Skydome
        "assets/shaders/standard.vert",
        "assets/shaders/standard.frag",
        { skydomeVertexLayout , _countof(skydomeVertexLayout) },
        renderer::Raster_PrimTriangleStrip | renderer::Raster_CullFront | renderer::Raster_FrontCCW
    },
    {
        "assets/shaders/standard.vert",
        "assets/shaders/standard.frag",
        { standardVertexLayout , _countof(standardVertexLayout) },
        renderer::Raster_DefaultState
    }
};

//==============================
// Render Device
//==============================

GLint TranslateWrapMode(ESamplerWrapMode mode)
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

void RenderDevice::Init()
{
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
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
   // glEnable(GL_MULTISAMPLE);

    // set default filter mode to anisotropic with max anisotropy
    glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, (GLint *)&imageFilterMaxAnisotropy);
    DEBUG_LOG_TEXT("Max texture anisotropy: %d", imageFilterMaxAnisotropy);
    const SamplerStateInitializer initializer(SamplerFilter_Anisotropic, renderer::SamplerWrap_Repeat, renderer::SamplerWrap_Repeat, imageFilterMaxAnisotropy);
    const std::shared_ptr<ISamplerState> samplerState = CreateSamplerState(initializer);
    SetSamplerState(0, samplerState);
    SetSamplerState(1, samplerState);
    SetSamplerState(2, samplerState);
    SetSamplerState(3, samplerState);

    // Initialize builtin pipeline states
    for (uint32_t i = 0; i < BuiltintPipelineState_Count; i++)
    {
        PipelineState *pipelineState = &builtintPipelineStates[i];
        const CreatePipelineStateInfo &createInfo = builtintPipelineStatesCreateInfo[i];
        pipelineState->Create(createInfo);
    }

    isInititialized = true;
}

void RenderDevice::Shutdown()
{
    if (isInititialized)
    {
        glDeleteVertexArrays(1, &vaoId);
        isInititialized = false;
    }
}

void RenderDevice::SetProjection(const glm::mat4 &proj)
{
    projection = proj;
}

PipelineState *RenderDevice::BuiltintPipelineState(uint32_t pipelineStateEnum)
{
    assert(pipelineStateEnum < BuiltintPipelineState_Count);
    return &builtintPipelineStates[pipelineStateEnum];
}

std::shared_ptr<ITexture2D> RenderDevice::ImageFromFile(const char *filename)
{
    auto image = FindImage(filename);
    if (image)
    {
        return image;
    }

    FileReader file(filename, true);
    DEBUG_LOG_TEXT("Loading image from file %s [hash 0x%x]...", filename, CalculateMurmurHash(filename, strlen(filename)));

    int width, height, bpp;
    stbi_set_flip_vertically_on_load(1);        // convert to opengl texture coordniate system
    unsigned char *data = stbi_load_from_memory((const stbi_uc*)file.RawBuffer(), (int)file.Size(), &width, &height, &bpp, 4);
    if (!data)
    {
        DEBUG_LOG_TEXT("Failed to load image %s: %s", filename, stbi_failure_reason());
        throw FatalException(std::string("Failed to load texture ") + filename);
    }

    image = ImageFromMemoryInternal(filename, data, width, height, PixelFormat_R8G8B8A8);
    stbi_image_free(data);

    return image;
}

std::shared_ptr<ITexture2D> RenderDevice::ImageFromMemory(const char *name, const void *data, uint32_t width, uint32_t height, EPixelFormat format)
{
    auto image = FindImage(name);
    return image ? image : ImageFromMemoryInternal(name, data, width, height, format);
}

std::shared_ptr<ITexture2D> RenderDevice::ImageFromMemoryInternal(const char *name, const void *data, uint32_t width, uint32_t height, EPixelFormat format)
{
    const uint32_t hash = CalculateMurmurHash(name, strlen(name));
    auto image = CreateTexture2D(width, height, format, 0, data);
    imageCache[hash] = image;

    return image;
}

std::shared_ptr<ITexture2D> RenderDevice::FindImage(const char *name)
{
    const uint32_t hash = CalculateMurmurHash(name, strlen(name));
    const auto searchResult = imageCache.find(hash);
    return searchResult != imageCache.end() ? searchResult->second : nullptr;
}

GLint RenderDevice::OpenGLCreateBuffer(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage)
{
    GLuint resource = 0;
    glCreateBuffers(1, &resource);
    glBindBuffer(target, resource);
    glBufferData(target, size, data, usage);
    return resource;
}

std::shared_ptr<IVertexBuffer> RenderDevice::CreateVertexBuffer(const void *data, size_t size)
{
    const GLenum usage = GL_STATIC_DRAW;       // NOTE: only support static draw usage for now....
    const GLuint resource = OpenGLCreateBuffer(GL_ARRAY_BUFFER, size, data, usage);
    return std::make_shared<OpenGLVertexBuffer>(resource, GL_ARRAY_BUFFER, usage, size);
}

std::shared_ptr<IIndexBuffer> RenderDevice::CreateIndexBuffer(const void *data, size_t size)
{
    const GLenum usage = GL_STATIC_DRAW;       // NOTE: only support static draw usage for now....
    const GLuint resource = OpenGLCreateBuffer(GL_ELEMENT_ARRAY_BUFFER, size, data, usage);
    return std::make_shared<OpenGLIndexBuffer>(resource, GL_ELEMENT_ARRAY_BUFFER, usage, size);
}

std::shared_ptr<ITexture2D> RenderDevice::CreateTexture2D(int32_t width, int32_t height, EPixelFormat format, int32_t numMipMaps, const void *data)
{
    const OpenGLTextureFormat *GLFormat = &pixelFormats[format];

    GLuint textureId = 0;
    const GLenum target = GL_TEXTURE_2D;
    glCreateTextures(target, 1, &textureId);
    glBindTexture(target, textureId);

    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, numMipMaps > 1 ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST);

    glTexImage2D(target, 0, GLFormat->internalFormat, width, height, 0, GLFormat->format, GLFormat->type, data);
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

void RenderDevice::SetTexture(uint32_t textureIndex, const std::shared_ptr<ITexture> texture)
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

std::shared_ptr<ISamplerState> RenderDevice::CreateSamplerState(const SamplerStateInitializer &initializer)
{
    auto searchResult = samplerStateCache.find(initializer);
    if (searchResult != samplerStateCache.end())
    {
        return searchResult->second;
    }

    auto samplerState = std::make_shared<OpenGLSamplerState>();

    samplerState->data.wrapS = TranslateWrapMode(initializer.wrapU);
    samplerState->data.wrapT = TranslateWrapMode(initializer.wrapV);
    samplerState->data.LODBias = initializer.mipBias;

    switch (initializer.filter)
    {
    case SamplerFilter_Anisotropic:
        samplerState->data.magFilter = GL_LINEAR;
        samplerState->data.minFilter = GL_LINEAR_MIPMAP_LINEAR;
        samplerState->data.maxAnisotropy = Clamp(initializer.maxAnisotropy, 1U, imageFilterMaxAnisotropy);
        break;
    case SamplerFilter_Trilinear:
        samplerState->data.magFilter = GL_LINEAR;
        samplerState->data.minFilter = GL_LINEAR_MIPMAP_LINEAR;
        break;
    case SamplerFilter_Bilinear:
        samplerState->data.magFilter = GL_LINEAR;
        samplerState->data.minFilter = GL_LINEAR_MIPMAP_NEAREST;
        break;
    case SamplerFilter_Point:
        samplerState->data.magFilter = GL_NEAREST;
        samplerState->data.minFilter = GL_NEAREST_MIPMAP_NEAREST;
    }

    glGenSamplers(1, &samplerState->resource);
    glSamplerParameteri(samplerState->resource, GL_TEXTURE_WRAP_S, samplerState->data.wrapS);
    glSamplerParameteri(samplerState->resource, GL_TEXTURE_WRAP_T, samplerState->data.wrapT);
    glSamplerParameteri(samplerState->resource, GL_TEXTURE_LOD_BIAS, samplerState->data.LODBias);
    glSamplerParameteri(samplerState->resource, GL_TEXTURE_MAG_FILTER, samplerState->data.magFilter);
    glSamplerParameteri(samplerState->resource, GL_TEXTURE_MIN_FILTER, samplerState->data.minFilter);
    glSamplerParameteri(samplerState->resource, GL_TEXTURE_MAX_ANISOTROPY_EXT, samplerState->data.maxAnisotropy);

    samplerStateCache[initializer] = samplerState;
    return samplerState;
}

void RenderDevice::SetSamplerState(uint32_t textureIndex, const std::shared_ptr<ISamplerState> state)
{
    assert(state);
    const std::shared_ptr<OpenGLSamplerState> samplerState = std::static_pointer_cast<OpenGLSamplerState>(state);
    glActiveTexture(GL_TEXTURE0 + textureIndex);
    glBindSampler(textureIndex, samplerState->resource);
}

void RenderDevice::Clear(uint32_t targets, const glm::vec4 color, float depth)
{
    const GLbitfield mask = (targets & Clear_Color ? GL_COLOR_BUFFER_BIT : 0) |
        (targets & Clear_Depth ? GL_DEPTH_BUFFER_BIT : 0) |
        (targets & Clear_Stencil ? GL_STENCIL_BUFFER_BIT : 0);

    glClearColor(color.r, color.g, color.b, color.a);
    glClearDepth(depth);
    glClear(mask);
}

void RenderDevice::Render(const Surface *surf, const glm::mat4 &transform)
{
    assert(surf);

    glBindVertexArray(vaoId);

    // setup material & pipeline state
    const SurfaceMaterial *material = &surf->material;
    PipelineState *pipelineState = surf->pipelineState;
    pipelineState->Bind();
    pipelineState->SetParamMat4(renderer::Param_Proj, glm::value_ptr(projection));
    pipelineState->SetParamMat4(renderer::Param_ModelView, glm::value_ptr(transform));

    if (material->texture[0])
        SetTexture(0, material->texture[0]);    // TODO: Texture state should be manually set by the user!

    // setup geometry
    const SurfaceGeometry *geo = &surf->geometry;
    const std::shared_ptr<OpenGLVertexBuffer> VBO = std::static_pointer_cast<OpenGLVertexBuffer>(geo->VBO);
    const std::shared_ptr<OpenGLIndexBuffer> IBO = std::static_pointer_cast<OpenGLIndexBuffer>(geo->IBO);
    glBindBuffer(VBO->target, VBO->resource);
    glBindBuffer(IBO->target, IBO->resource);

    // draw
    BindVertexLayout(pipelineState->GetVertexLayout());
    glDrawElements(pipelineState->GetGLPrimType(), geo->indexCount, GL_UNSIGNED_SHORT, NULL);
    UnBindVertexLayout(pipelineState->GetVertexLayout());
}

std::shared_ptr<IRenderDevice> CreateRenderDevice()
{
    return std::make_shared<RenderDevice>();
}

} // renderer