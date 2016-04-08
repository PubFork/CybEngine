#pragma once
#include "RenderDevice.h"

namespace renderer
{

class TextureCache
{
public:
    void Initialize(std::shared_ptr<IRenderDevice> inDevice);

    std::shared_ptr<ITexture2D> ImageFromFile(const char *filename);
    std::shared_ptr<ITexture2D> ImageFromMemory(const char *name, const void *data, uint32_t width, uint32_t height, EPixelFormat format);
    std::shared_ptr<ITexture2D> FindImage(const char *name);

private:
    std::shared_ptr<ITexture2D> ImageFromMemoryInternal(const char *name, const void *data, uint32_t width, uint32_t height, EPixelFormat format);

    std::shared_ptr<IRenderDevice> device;
    std::unordered_map<uint32_t, std::shared_ptr<ITexture2D>> imageCache;
};

} // renderer