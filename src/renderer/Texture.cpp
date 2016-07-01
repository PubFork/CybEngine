#include "Precompiled.h"
#include "Renderer/Texture.h"
#include "Base/Debug.h"
#include "Base/Sys.h"
#include "Base/File.h"
#include "Base/MurmurHash.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace renderer
{

static TextureCache staticTextureCache;
TextureCache *globalTextureCache = &staticTextureCache;

void TextureCache::Initialize(std::shared_ptr<IRenderDevice> inDevice)
{
    device = inDevice;
}

void TextureCache::Destroy()
{
    imageCache.clear();
}

void TextureCache::Flush()
{
    std::vector<uint32_t> deleteList;

    for (auto cacheEntry : imageCache)
    {
        if (cacheEntry.second.use_count() == 1)
        {
            deleteList.push_back(cacheEntry.first);
        }
    }

    for (auto textureHash : deleteList)
    {
        imageCache.erase(textureHash);
    }
}

std::shared_ptr<ITexture2D> TextureCache::LoadTexture2DFromFile(const char *filename)
{
    assert(filename);

    const uint32_t hashKey = CalculateMurmurHash(filename, strlen(filename));
    auto image = FindImage(hashKey);
    if (!image)
    {
        SysFile textureFile(filename, FileOpen_Read);
        if (textureFile.IsValid())
        {
            DebugPrintf("Loading image from file %s [hash 0x%x]...\n", filename, CalculateMurmurHash(filename, strlen(filename)));

            const size_t textureBufferSize = textureFile.GetLength();
            const stbi_uc *rawTextureBuffer = new stbi_uc[textureBufferSize];
            textureFile.Read((uint8_t *)rawTextureBuffer, textureBufferSize);

            int width, height, bpp;
            stbi_set_flip_vertically_on_load(1);        // convert to opengl texture coordniate system
            unsigned char *data = stbi_load_from_memory(rawTextureBuffer, (int)textureBufferSize, &width, &height, &bpp, 4);
            delete[] rawTextureBuffer;
            if (!data)
            {
                Sys_ErrorPrintf("Failed to load image %s: %s\n", filename, stbi_failure_reason());
                return nullptr;
            }

            image = ImageFromMemoryInternal(hashKey, width, height, PixelFormat_R8G8B8A8, data);
            stbi_image_free(data);
        }
    }

    return image;
}

std::shared_ptr<ITexture2D> TextureCache::LoadTexture2DFromMemory(const char *name, uint32_t width, uint32_t height, PixelFormat format, const void *pixels)
{
    const uint32_t hashKey = CalculateMurmurHash(name, strlen(name));
    auto image = FindImage(hashKey);
    if (!image)
    {
        image = ImageFromMemoryInternal(hashKey, width, height, format, pixels);
    }

    return image;
}

std::shared_ptr<ITextureCube> TextureCache::LoadTextureCubeFromFiles(const char *filenames[6])
{
    stbi_uc *imageBuffers[6] = {};
    int32_t cubeWidth = 0;
    int32_t cubeHeight = 0;
    bool errorFlag = false;

    for (uint32_t i = 0; i < 6 && !errorFlag; i++)
    {
        const char *filename = filenames[i];

        SysFile textureFile(filename, FileOpen_Read);
        if (textureFile.IsValid())
        {
            DebugPrintf("Loading image from file %s [hash 0x%x]...\n", filename, CalculateMurmurHash(filename, strlen(filename)));

            const size_t textureBufferSize = textureFile.GetLength();
            const stbi_uc *rawTextureBuffer = new stbi_uc[textureBufferSize];
            textureFile.Read((uint8_t *)rawTextureBuffer, textureBufferSize);

            int width, height, bpp;
            stbi_set_flip_vertically_on_load(1);        // convert to opengl texture coordniate system

            imageBuffers[i] = stbi_load_from_memory(rawTextureBuffer, (int)textureBufferSize, &width, &height, &bpp, 4);
            cubeWidth = width;
            cubeHeight = height;
            delete[] rawTextureBuffer;
            
            if (!imageBuffers[i])
            {
                Sys_ErrorPrintf("Failed to load image %s: %s\n", filename, stbi_failure_reason());
                errorFlag = true;
            }
        }
        else
        {
            errorFlag = true;
        }
    }

    std::shared_ptr<ITextureCube> cubeTexture;
    if (!errorFlag)
    {
        cubeTexture = device->CreateTextureCube(cubeWidth, cubeHeight, PixelFormat_R8G8B8A8, (const void **)imageBuffers);
    }

    for (uint32_t i = 0; i < 6; i++)
    {
        stbi_image_free(imageBuffers[i]);
    }

    return cubeTexture;
}

std::shared_ptr<ITexture2D> TextureCache::FindImage(uint32_t hashKey)
{
    const auto searchResult = imageCache.find(hashKey);
    if (searchResult != imageCache.end())
    {
        return searchResult->second;
    }

    return nullptr;
}

std::shared_ptr<ITexture2D> TextureCache::ImageFromMemoryInternal(uint32_t hashKey, uint32_t width, uint32_t height, PixelFormat format, const void *pixels)
{
    auto image = device->CreateTexture2D(width, height, format, 0, pixels);
    imageCache[hashKey] = image;
    return image;
}

}   // renderer