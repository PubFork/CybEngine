#include "stdafx.h"
#include <GL/glew.h>
#include <Windows.h>
#include "Image.h"
#include "Base/Debug.h"
#include "Base/FileUtils.h"
#include "Base/MurmurHash.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace renderer
{

static ImageManager staticGlobalImages;
ImageManager *globalImages = &staticGlobalImages;

void FilterRgba2x2(const uint8_t *src, int w, int h, uint8_t *dest)
{
    for (int j = 0; j < (h & ~1); j += 2)
    {
        const uint8_t* psrc = src + (w * j * 4);
        uint8_t*       pdest = dest + ((w >> 1) * (j >> 1) * 4);

        int wp = 0;
        for (int i = 0; i < w >> 1; i++, psrc += 8, pdest += 4)
        {
            wp = w << 2;
            pdest[0] = (((int)psrc[0]) + psrc[4] + psrc[wp + 0] + psrc[wp + 4]) >> 2;
            pdest[1] = (((int)psrc[1]) + psrc[5] + psrc[wp + 1] + psrc[wp + 5]) >> 2;
            pdest[2] = (((int)psrc[2]) + psrc[6] + psrc[wp + 2] + psrc[wp + 6]) >> 2;
            pdest[3] = (((int)psrc[3]) + psrc[7] + psrc[wp + 3] + psrc[wp + 7]) >> 2;
        }
    }
}

int GetNumMipLevels(int width, int height)
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

void SetDefaultImageParams(ImageParams &param)
{
    param.width = 0;
    param.height = 0;
    param.numLevels = 0;
    param.sampleMode = Image::Sample_Anisotropic | Image::Sample_Repeat;
    param.format = Image::Format_None;
}

Image::Image(const char *imageName)
{
    name = imageName;
    SetDefaultImageParams(param);

    textureId = Not_Loaded;
    glFormat = 0;
    glType = 0;
}

Image::~Image()
{
    Destroy();
}

void Image::Create(const void *pixels, uint32_t width, uint32_t height, uint32_t format, uint32_t sampleMode)
{
    param.width = width;
    param.height = height;
    param.numLevels = GetNumMipLevels(width, height);
    param.format = format;
    param.sampleMode = sampleMode;
    AllocImage();
    
    SubImageUpload(0, 0, 0, width, height, pixels);
    GenerateMipmaps(pixels, 0);
}

void Image::Destroy()
{
    if (textureId != Not_Loaded)
    {
        glDeleteTextures(1, &textureId);
        textureId = Not_Loaded;
    }
}

void Image::Bind(int slot)
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, textureId);
}

void Image::SubImageUpload(uint32_t level, int32_t x, int32_t y, uint32_t width, uint32_t height, const void *pixels)
{
    assert(x >= 0 && y >= 0 && pixels != nullptr);
    glTextureSubImage2D(textureId, level, x, y, width, height, glFormat, glType, pixels);
}

void Image::SampleMode(uint32_t flags)
{
    param.sampleMode = flags;
    UpdateTextureParams();
}

void Image::AllocImage()
{
    Destroy();

    switch (param.format)
    {
    case Format_RGBA8:
        glFormat = GL_RGBA;
        glType = GL_UNSIGNED_BYTE;
        break;

    default:
        throw FatalException(std::string("Unsupported image format ") + std::to_string(param.format));
    }

    glCreateTextures(GL_TEXTURE_2D, 1, &textureId);
    assert(textureId != Not_Loaded);
    glBindTexture(GL_TEXTURE_2D, textureId);

    // allocate all the mip levels with null data
    uint32_t w = param.width;
    uint32_t h = param.height;
    for (uint32_t level = 0; level < param.numLevels; level++)
    {
        glTexImage2D(GL_TEXTURE_2D, level, glFormat, w, h, 0, glFormat, glType, NULL);
        w = std::max<uint32_t>(1, w >> 1);
        h = std::max<uint32_t>(1, h >> 1);
    }

    UpdateTextureParams();
}

void Image::GenerateMipmaps(const void *pixels, uint32_t level)
{
    uint32_t srcw = param.width;
    uint32_t srch = param.height;
    uint8_t *mipmaps = nullptr;
    do
    {
        level++;
        int mipw = srcw >> 1; if (mipw < 1) mipw = 1;
        int miph = srch >> 1; if (miph < 1) miph = 1;

        if (!mipmaps)
            mipmaps = new uint8_t[mipw * miph * 4];
        FilterRgba2x2(level == 1 ? (const uint8_t*)pixels : mipmaps, srcw, srch, mipmaps);
        SubImageUpload(level, 0, 0, mipw, miph, mipmaps);
        srcw = mipw;
        srch = miph;
    } while (srcw > 1 || srch > 1);
    if (mipmaps)
        delete[] mipmaps;

    glTextureParameteri(textureId, GL_TEXTURE_MAX_LEVEL, level);
}

void Image::UpdateTextureParams() const
{
    switch (param.sampleMode & Sample_FilterMask)
    {
    case Sample_Nearest:
        glTextureParameteri(textureId, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(textureId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTextureParameteri(textureId, GL_TEXTURE_MAX_ANISOTROPY_EXT, 0);
        break;

    case Sample_Linear:
        glTextureParameteri(textureId, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTextureParameteri(textureId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(textureId, GL_TEXTURE_MAX_ANISOTROPY_EXT, 0);
        break;

    case Sample_Anisotropic:
        glTextureParameteri(textureId, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTextureParameteri(textureId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(textureId, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16 /*maxTextureAnisotroy*/);
        break;
    }

    switch (param.sampleMode & Sample_AddressMask)
    {
    case Sample_Repeat:
        glTextureParameteri(textureId, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(textureId, GL_TEXTURE_WRAP_T, GL_REPEAT);
        break;

    case Sample_Clamp:
        glTextureParameteri(textureId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(textureId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        break;

    case Sample_ClampBorder:
        glTextureParameteri(textureId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTextureParameteri(textureId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        break;
    }
}

std::shared_ptr<Image> ImageManager::ImageFromFile(const char *filename, uint32_t sampleFlags)
{
    // first check if image allready exists in cache
    auto image = Find(filename);
    if (image)
        return image;

    // load imag using stbi_image
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

    image = AllocImage(filename);
    image->Create(data, width, height, Image::Format_RGBA8, sampleFlags);
    stbi_image_free(data);

    return image;
}

std::shared_ptr<Image> ImageManager::AllocImage(const char *name)
{
    uint32_t hash = CalculateMurmurHash(name, strlen(name));

    auto image = std::make_shared<Image>(name);
    images[hash] = image;

    return image;
}

std::shared_ptr<Image> ImageManager::Find(const char *name)
{
    uint32_t hash = CalculateMurmurHash(name, strlen(name));
    auto searchResult = images.find(hash);
    return searchResult != images.end() ? searchResult->second : nullptr;
}

} // renderer

