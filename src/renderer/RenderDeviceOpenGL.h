#pragma once

#include <GL/glew.h>
#include <Windows.h>

namespace renderer
{

// SamplerStateInitializer hasher for compatibility with std::unordered_map<>
struct SamplerStateInitializerHasher
{
    size_t operator()(const SamplerStateInitializer &state) const;
};

template <class BaseType>
class OpenGLBufferBase : public BaseType
{
public:
    OpenGLBufferBase(GLuint inResource, GLenum inTarget, GLenum inUsage, GLsizeiptr inSize) :
        resource(inResource),
        target(inTarget),
        usage(inUsage),
        size(inSize)
    {
    }

    virtual ~OpenGLBufferBase();

    GLuint resource;
    GLenum target;
    GLenum usage;
    GLsizeiptr size;
};

class OpenGLVertexBufferBase : public IVertexBuffer {};
class OpenGLIndexBufferBase : public IIndexBuffer {};
typedef OpenGLBufferBase<OpenGLVertexBufferBase> OpenGLVertexBuffer;
typedef OpenGLBufferBase<OpenGLIndexBufferBase> OpenGLIndexBuffer;

struct OpenGLSamplerStateData
{
    OpenGLSamplerStateData() :
        magFilter(GL_NEAREST),
        minFilter(GL_NEAREST),
        wrapS(GL_REPEAT),
        wrapT(GL_REPEAT),
        LODBias(0),
        maxAnisotropy(1)
    {
    }

    GLint magFilter;
    GLint minFilter;
    GLint wrapS;
    GLint wrapT;
    GLint LODBias;
    GLint maxAnisotropy;
};

class OpenGLSamplerState : public ISamplerState
{
public:
    GLuint resource;
    OpenGLSamplerStateData data;

    ~OpenGLSamplerState()
    {
        glDeleteSamplers(1, &resource);
    }
};

struct OpenGLTextureFormat
{
    GLenum internalFormat;
    GLenum format;
    GLenum type;
    GLboolean compressed;
};

template <class BaseType>
class OpenGLTextureBase : public BaseType
{
public:
    OpenGLTextureBase(
        GLuint inResource,
        GLenum inTarget,
        uint32_t inWidth,
        uint32_t inHeight,
        uint32_t inNumMips,
        EPixelFormat inFormat) :
        BaseType(inWidth, inHeight, inNumMips, inFormat),
        resource(inResource),
        target(inTarget)
    {
    }

    virtual ~OpenGLTextureBase();

public:
    GLuint resource;
    GLenum target;
};

class OpenGLBaseTexture2D : public ITexture2D
{
public:
    OpenGLBaseTexture2D(uint32_t inWidth, uint32_t inHeight, uint32_t inNumMips, EPixelFormat inFormat) :
        ITexture2D(inWidth, inHeight, inNumMips, inFormat)
    {
    }
};

typedef OpenGLTextureBase<ITexture>            OpenGLTexture;
typedef OpenGLTextureBase<OpenGLBaseTexture2D> OpenGLTexture2D;

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

    virtual std::shared_ptr<ITexture2D> ImageFromFile(const char *filename);
    virtual std::shared_ptr<ITexture2D> ImageFromMemory(const char *name, const void *data, uint32_t width, uint32_t height, EPixelFormat format);
    std::shared_ptr<ITexture2D> ImageFromMemoryInternal(const char *name, const void *data, uint32_t width, uint32_t height, EPixelFormat format);
    std::shared_ptr<ITexture2D> FindImage(const char *name);

    GLint OpenGLCreateBuffer(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);
    virtual std::shared_ptr<IVertexBuffer> CreateVertexBuffer(const void *data, size_t size);
    virtual std::shared_ptr<IIndexBuffer> CreateIndexBuffer(const void *data, size_t size);

    virtual std::shared_ptr<ITexture2D> CreateTexture2D(int32_t width, int32_t height, EPixelFormat format, int32_t numMipMaps, const void *data);
    virtual void SetTexture(uint32_t textureIndex, const std::shared_ptr<ITexture> texture);

    virtual std::shared_ptr<ISamplerState> RenderDevice::CreateSamplerState(const SamplerStateInitializer &initializer);
    virtual void SetSamplerState(uint32_t textureIndex, const std::shared_ptr<ISamplerState> state);

    virtual void Clear(uint32_t targets, const glm::vec4 color, float depth = 1.0f);
    virtual void Render(const Surface *surf, const glm::mat4 &transform);

private:
    GLuint vaoId;
    glm::mat4 projection;
    PipelineState builtintPipelineStates[BuiltintPipelineState_Count];
    std::unordered_map<uint32_t, std::shared_ptr<ITexture2D>> imageCache;

    std::unordered_map<SamplerStateInitializer, std::shared_ptr<OpenGLSamplerState>, SamplerStateInitializerHasher> samplerStateCache;

    uint32_t imageFilterMaxAnisotropy;
    bool isInititialized;
};

} // renderer