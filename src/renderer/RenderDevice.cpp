#include "stdafx.h"
#include "RenderDevice.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Base/Debug.h"
#include "Base/MurmurHash.h"
#include "Base/FileUtils.h"
#include <GL/glew.h>
#include <Windows.h>

namespace renderer
{

//==============================
// OpenGL Buffer
//==============================

class Buffer : public IBuffer
{
public:
    Buffer(IRenderDevice *rd);
    Buffer(IRenderDevice *rd, const BufferCreateParams *params);
    virtual ~Buffer();

    virtual void Create(const BufferCreateParams *params);
    virtual void Bind() const;

private:
    IRenderDevice *device;
    BufferType type;
    size_t size;

    // API specific members
    GLuint bufferId;
    GLenum target;
};

Buffer::Buffer(IRenderDevice *rd) :
    device(rd),
    type(Buffer_Invalid),
    size(0),
    bufferId(0),
    target(GL_INVALID_ENUM)
{
    assert(rd != nullptr);
}

Buffer::Buffer(IRenderDevice *rd, const BufferCreateParams *params) :
    Buffer(rd)
{
    Create(params);
}

Buffer::~Buffer()
{
    if (bufferId != 0)
    {
        glDeleteBuffers(1, &bufferId);
    }
}

void Buffer::Create(const BufferCreateParams *params)
{
    assert(params);
    assert(params->data != nullptr);
    assert(params->dataLength > 0);
    assert(device->IsInitialized());
    assert(bufferId == 0);

    switch (params->usage)
    {
    case Buffer_Index:  target = GL_ELEMENT_ARRAY_BUFFER; break;
    case Buffer_Vertex: target = GL_ARRAY_BUFFER; break;
    default: assert(!"Invalid enum"); break;
    }

    glCreateBuffers(1, &bufferId);
    size = params->dataLength;

    glBindBuffer(target, bufferId);
    glBufferData(target, size, params->data, GL_STATIC_DRAW);
}

void Buffer::Bind() const
{
    glBindBuffer(target, bufferId);
}

//==============================
// OpenGL Image
//==============================

class Image : public IImage
{
public:
    Image(IRenderDevice *rd);
    Image(IRenderDevice *rd, const ImageCreateParams *params);
    virtual ~Image();

    virtual void Create(const ImageCreateParams *params);
    virtual void Bind(uint8_t slot);
    virtual void UpdateFilterMode(ImageFilterMode filterMode);

private:
    IRenderDevice *device;
    uint32_t width;
    uint32_t height;
    uint32_t numLevels;
    ImageFormat format;
    ImageFilterMode filtering;
    ImageWrapMode wrapMode;

    // API specific members
    uint32_t textureId;
    uint32_t glFormat;
    uint32_t glType;
};

int NumMipLevels(uint32_t width, uint32_t height)
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

Image::Image(IRenderDevice *rd) :
    device(rd),
    width(0),
    height(0),
    numLevels(0),
    format(ImageFormat_None),
    filtering(ImageFilter_Nearest),
    wrapMode(ImageWrap_Repeat),
    textureId(0),
    glFormat(0),
    glType(0)
{
    assert(rd != nullptr);
}

Image::Image(IRenderDevice *rd, const ImageCreateParams *params) :
    Image(rd)
{
    Create(params);
}

Image::~Image()
{
    if (textureId != 0)
    {
        glDeleteTextures(1, &textureId);
    }
}

void Image::Create(const ImageCreateParams *params)
{
    assert(params != nullptr);
    assert(params->pixels != nullptr);
    assert(device->IsInitialized());
    assert(textureId == 0);

    // setup local data
    width = params->width;
    height = params->height;
    numLevels = NumMipLevels(width, height);
    format = params->format;

    switch (params->format)
    {
    case ImageFormat_RGBA8:
        glFormat = GL_RGBA;
        glType = GL_UNSIGNED_BYTE;
        break;
    default: assert(!"Invalid enum"); break;
    }

    // create and upload image
    glCreateTextures(GL_TEXTURE_2D, 1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexImage2D(GL_TEXTURE_2D, 0, glFormat, width, height, 0, glFormat, glType, params->pixels);
    glGenerateMipmap(GL_TEXTURE_2D);

    // set texture params
    static const GLint wrapModeToGL[] = {
        GL_REPEAT,                                          // ImageWrap_Repeat
        GL_CLAMP_TO_EDGE,                                   // ImageWrap_Clamp
        GL_CLAMP_TO_BORDER                                  // ImageWrap_ClampBorder
    };
    
    glTextureParameteri(textureId, GL_TEXTURE_MAX_LEVEL, numLevels);
    glTextureParameteri(textureId, GL_TEXTURE_WRAP_S, wrapModeToGL[wrapMode]);
    glTextureParameteri(textureId, GL_TEXTURE_WRAP_T, wrapModeToGL[wrapMode]);
    UpdateFilterMode(params->filtering);
}

void Image::Bind(uint8_t slot)
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, textureId);
}

void Image::UpdateFilterMode(ImageFilterMode filterMode)
{
    static const std::pair<GLint, GLint> filterToGL[] = {
        { GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST },           // ImageFilter_Nearest
        { GL_LINEAR_MIPMAP_LINEAR,  GL_LINEAR  },           // ImageFilter_Linear
        { GL_LINEAR_MIPMAP_LINEAR,  GL_LINEAR  }            // ImageFilter_Anisotropic
    };

    filtering = filterMode;
    glTextureParameteri(textureId, GL_TEXTURE_MIN_FILTER, filterToGL[filterMode].first);
    glTextureParameteri(textureId, GL_TEXTURE_MAG_FILTER, filterToGL[filterMode].second);
    glTextureParameteri(textureId, GL_TEXTURE_MAX_ANISOTROPY_EXT, filtering != ImageFilter_Anisotropic ? 1 : device->ImageFilterMaxAnisotropy());
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

class RenderDevice : public IRenderDevice
{
public:
    RenderDevice() : isInititialized(false) {}
    virtual ~RenderDevice() { Shutdown(); }

    virtual void Init();
    virtual void Shutdown();
    virtual bool IsInitialized() const { return isInititialized; }

    virtual void SetProjection(const glm::mat4 &proj);

    virtual PipelineState *BuiltintPipelineState(uint32_t pipelineStateEnum);

    virtual std::shared_ptr<IBuffer> CreateBuffer(BufferType usage, const void *dataBuffer, size_t dataBufferSize);
    virtual std::shared_ptr<IImage> ImageFromFile(const char *filename, ImageWrapMode wrapMode);
    virtual std::shared_ptr<IImage> ImageFromMemory(const char *name, const void *data, uint32_t width, uint32_t height, uint32_t format, ImageWrapMode wrapMode);
    std::shared_ptr<IImage> ImageFromMemoryInternal(const char *name, const void *data, uint32_t width, uint32_t height, uint32_t format, ImageWrapMode wrapMode);
    std::shared_ptr<IImage> FindImage(const char *name);
    virtual void SetImageFilterMode(ImageFilterMode mode, bool applyToCachedImages);
    virtual uint32_t ImageFilterMaxAnisotropy() const { return imageFilterMaxAnisotropy; }

    virtual void Clear(uint32_t targets, const glm::vec4 color, float depth = 1.0f);
    virtual void Render(const Surface *surf, const glm::mat4 &transform);

protected:
    GLuint vaoId;
    glm::mat4 projection;
    PipelineState builtintPipelineStates[BuiltintPipelineState_Count];
    std::unordered_map<uint32_t, std::shared_ptr<IImage>> imageCache;
    ImageFilterMode imageFilterMode;
    uint32_t imageFilterMaxAnisotropy;
    bool isInititialized;
};

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
    GLenum err = glewInit();
    THROW_FATAL_COND(err != GLEW_OK, std::string("glew init: ") + (char *)glewGetErrorString(err));
    DEBUG_LOG_TEXT("Using OpenGL version %s", glGetString(GL_VERSION));
    DEBUG_LOG_TEXT("Shader language %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
    DEBUG_LOG_TEXT("GLEW %s", glewGetString(GLEW_VERSION));
   
    glDebugMessageCallback(DebugOutputCallback, NULL);
    glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_OTHER, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

    glGenVertexArrays(1, &vaoId);

    // setup default states
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
   // glEnable(GL_MULTISAMPLE);

    // set default filter mode to anisotropic with max anisotropy
    glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, (GLint *)&imageFilterMaxAnisotropy);
    DEBUG_LOG_TEXT("Max texture anisotropy: %d", imageFilterMaxAnisotropy);
    SetImageFilterMode(ImageFilter_Anisotropic, true);

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

std::shared_ptr<IBuffer> RenderDevice::CreateBuffer(BufferType usage, const void *dataBuffer, size_t dataBufferSize)
{
    BufferCreateParams params = {};
    params.usage = usage;
    params.data = dataBuffer;
    params.dataLength = dataBufferSize;

    return std::make_shared<Buffer>(this, &params);
}

std::shared_ptr<IImage> RenderDevice::ImageFromFile(const char *filename, ImageWrapMode wrapMode)
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

    image = ImageFromMemoryInternal(filename, data, width, height, ImageFormat_RGBA8, wrapMode);
    stbi_image_free(data);

    return image;
}

std::shared_ptr<IImage> RenderDevice::ImageFromMemory(const char *name, const void *data, uint32_t width, uint32_t height, uint32_t format, ImageWrapMode wrapMode)
{
    auto image = FindImage(name);
    return image ? image : ImageFromMemoryInternal(name, data, width, height, format, wrapMode);
}

std::shared_ptr<IImage> RenderDevice::ImageFromMemoryInternal(const char *name, const void *data, uint32_t width, uint32_t height, uint32_t format, ImageWrapMode wrapMode)
{
    ImageCreateParams params = {};
    params.width = width;
    params.height = height;
    params.format = (ImageFormat)format;
    params.pixels = data;
    params.filtering = imageFilterMode;
    params.wrapMode = wrapMode;
    auto image = std::make_shared<Image>(this, &params);

    uint32_t hash = CalculateMurmurHash(name, strlen(name));
    imageCache[hash] = image;

    return image;
}

std::shared_ptr<IImage> RenderDevice::FindImage(const char *name)
{
    uint32_t hash = CalculateMurmurHash(name, strlen(name));
    auto searchResult = imageCache.find(hash);
    return searchResult != imageCache.end() ? searchResult->second : nullptr;
}

void RenderDevice::SetImageFilterMode(ImageFilterMode mode, bool applyToCachedImages)
{
    imageFilterMode = mode;

    if (applyToCachedImages)
    {
        for (auto &image : imageCache)
        {
            image.second->UpdateFilterMode(imageFilterMode);
        }
    }
}

void RenderDevice::Clear(uint32_t targets, const glm::vec4 color, float depth)
{
    GLbitfield mask = (targets & Clear_Color ? GL_COLOR_BUFFER_BIT : 0) |
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
        material->texture[0]->Bind(0);

    // setup geometry
    const SurfaceGeometry *geo = &surf->geometry;
    geo->VBO->Bind();
    geo->IBO->Bind();

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