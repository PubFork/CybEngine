#include "stdafx.h"
#include "RenderDevice.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"
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

///////////////////////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////////////////

// Generate image mipmap using stbi_image_resize with default hi-quality 
// STBIR_FILTER_CATMULLROM / STBIR_FILTER_MITCHELL filtering.
void GenerateMipmaps(uint32_t width, uint32_t height, int level, const void *pixels)
{
    uint32_t srcw = width;
    uint32_t srch = height;
    uint8_t *mipmaps = new uint8_t[(width >> 1) * (height >> 1)* 4];

    do
    {
        level++;
        int mipw = srcw >> 1; if (mipw < 1) mipw = 1;
        int miph = srch >> 1; if (miph < 1) miph = 1;

        stbir_resize_uint8(level == 1 ? (const uint8_t*)pixels : mipmaps, srcw, srch, 0, mipmaps, mipw, miph, 0, 4);
        glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA, mipw, miph, 0, GL_RGBA, GL_UNSIGNED_BYTE, mipmaps);
        srcw = mipw;
        srch = miph;
    } while (srcw > 1 || srch > 1);
    
    delete[] mipmaps;
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
    GenerateMipmaps(width, height, 0, params->pixels);
    //glGenerateMipmap(GL_TEXTURE_2D);

    // set texture params
    static const GLint wrapModeToGL[] = {
        GL_REPEAT,                                          // ImageWrap_Repeat
        GL_CLAMP_TO_EDGE,                                   // ImageWrap_Clamp
        GL_CLAMP_TO_BORDER                                  // ImageWrap_ClampBorder
    };
    
    glTextureParameteri(textureId, GL_TEXTURE_MAX_LEVEL, numLevels);
    glTextureParameteri(textureId, GL_TEXTURE_WRAP_S, wrapModeToGL[wrapMode]);
    glTextureParameteri(textureId, GL_TEXTURE_WRAP_T, wrapModeToGL[wrapMode]);
    UpdateFilterMode(params->filterMode);
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

    if (filtering == ImageFilter_Anisotropic)
    {
        glTextureParameteri(textureId, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16 /*maxTextureAnisotroy*/);
    }
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

void RenderDevice::Init()
{
    // Initialize builtin pipeline states
    for (uint32_t i = 0; i < BuiltintPipelineState_Count; i++)
    {
        PipelineState *pipelineState = &builtintPipelineStates[i];
        const CreatePipelineStateInfo &createInfo = builtintPipelineStatesCreateInfo[i];
        pipelineState->Create(createInfo);
    }

    isInititialized = true;
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

std::shared_ptr<IImage> RenderDevice::ImageFromFile(const char *filename, uint32_t sampleFlags)
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

    image = ImageFromMemoryInternal(filename, data, width, height, ImageFormat_RGBA8, sampleFlags);
    stbi_image_free(data);

    return image;
}

std::shared_ptr<IImage> RenderDevice::ImageFromMemory(const char *name, const void *data, uint32_t width, uint32_t height, uint32_t format, uint32_t sampleFlags)
{
    auto image = FindImage(name);
    return image ? image : ImageFromMemoryInternal(name, data, width, height, format, sampleFlags);
}

std::shared_ptr<IImage> RenderDevice::ImageFromMemoryInternal(const char *name, const void *data, uint32_t width, uint32_t height, uint32_t format, uint32_t sampleFlags)
{
    ImageCreateParams params = {};
    params.width = width;
    params.height = height;
    params.format = (ImageFormat)format;
    params.pixels = data;
    params.filterMode = ImageFilter_Anisotropic;
    params.wrapMode = ImageWrap_Repeat;
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

} // renderer