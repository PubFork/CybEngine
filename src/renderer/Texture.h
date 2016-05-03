#pragma once
#include "RenderDevice.h"

namespace renderer
{

class TextureCache
{
public:
    void Initialize(std::shared_ptr<IRenderDevice> inDevice);
    
    // This clear all entries from the cache. There's however no ensurance 
    // texture will be accually unloaded due to the nature of shared pointers.
    void Destroy();

    // Remove all textures this cache holds the only ref to.
    void Flush();

    std::shared_ptr<ITexture2D> LoadTexture2DFromFile(const char *filename);
    std::shared_ptr<ITexture2D> LoadTexture2DFromMemory(const char *name, uint32_t width, uint32_t height, PixelFormat format, const void *pixels);

private:
    std::shared_ptr<ITexture2D> FindImage(uint32_t hashKey);
    std::shared_ptr<ITexture2D> ImageFromMemoryInternal(uint32_t hashKey, uint32_t width, uint32_t height, PixelFormat format, const void *pixels);

    std::shared_ptr<IRenderDevice> device;
    std::unordered_map<uint32_t, std::shared_ptr<ITexture2D>> imageCache;
};

// TODO: This global will do for now, but i don't whant it here
extern TextureCache *globalTextureCache;

}   // renderer