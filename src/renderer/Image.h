#pragma once

namespace renderer
{

struct ImageParams
{
    uint32_t width;
    uint32_t height;
    uint32_t numLevels;
    uint32_t sampleMode;
    uint32_t format;
};

void SetDefaultImageParams(ImageParams &param);

class Image
{
public:
    enum : uint32_t { Not_Loaded = 0xffffffff };

    enum
    {
        Sample_Nearest      = 0x0000,
        Sample_Linear       = 0x0001,
        Sample_Anisotropic  = 0x0002,
        Sample_FilterMask   = 0x000f,

        Sample_Repeat       = 0x0000,
        Sample_Clamp        = 0x0010,
        Sample_ClampBorder  = 0x0020,
        Sample_AddressMask  = 0x00f0
    };

    enum
    {
        Format_None         = 0x1001,
        Format_RGBA8        = 0x1001,       // 32 bpp
        Format_Alpha        = 0x1002,       // 8 bpp single channel (unimplemented)
        Format_DXT1         = 0x1003,       // 4 bpp                (unimplemented)
        Format_DXT5         = 0x1004        // 8 bpp                (unimplemented)
    };

    Image(const char *imageName);
    ~Image();

    void Create(const void *pixels, uint32_t width, uint32_t height, uint32_t format, uint32_t sampleMode);
    void Destroy();
    void Bind(int slot);

    void SubImageUpload(uint32_t level, int32_t x, int32_t y, uint32_t width, uint32_t height, const void *pixels);
    void SampleMode(uint32_t flags);
    const std::string &GetName() const { return name; }

private:
    void AllocImage();
    void GenerateMipmaps(const void *pixels, uint32_t level);
    void UpdateTextureParams() const;

    std::string name;
    ImageParams param;

    // API specific members
    uint32_t textureId;
    uint32_t glFormat;
    uint32_t glType;
};

class ImageCache
{
public:
    // if the image allready exist is the cache, sampleFlags is ignored
    std::shared_ptr<Image> ImageFromFile(const char *filename, uint32_t sampleFlags);
    std::shared_ptr<Image> ImageFromMemory(const char *name, const void *data, uint32_t width, uint32_t height, uint32_t format, uint32_t sampleFlags);

    // search the cache for a named image, returns nullptr if not found
    std::shared_ptr<Image> FindImage(const char *name);

private:
    std::shared_ptr<Image> ImageFromMemoryInternal(const char *name, const void *data, uint32_t width, uint32_t height, uint32_t format, uint32_t sampleFlags);

    std::unordered_map<uint32_t, std::shared_ptr<Image>> imageCacheMap;
};

} // renderer