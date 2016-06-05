#pragma once
#include <GL/glew.h>

namespace renderer
{

class OpenGLBuffer : public IBuffer
{
public:
    OpenGLBuffer(GLuint inResource, GLenum inTarget, GLenum inUsage, GLsizeiptr inSize) :
        resource(inResource),
        target(inTarget),
        usage(inUsage),
        size(inSize)
    {
    }

    virtual ~OpenGLBuffer();
    virtual void *Map();
    virtual void Unmap();

    GLuint resource;
    GLenum target;
    GLenum usage;
    GLsizeiptr size;
};

struct OpenGLVertexElementUsageInfo
{
    GLint attribLocation;
    const char *attribName;
};

struct OpenGLVertexElementFormatInfo
{
    GLenum elementType;
    GLint numComponents;
    GLsizei alignedSíze;
    GLboolean normalized;
};

struct OpenGLVertexElement
{
    OpenGLVertexElement(
        GLuint inAttributeLocation,
        GLint inNumComponents,
        GLenum inType,
        GLboolean inNormalized,
        GLsizei inStride​,
        GLintptr inOffset​) :
        attributeLocation(inAttributeLocation),
        numComponents(inNumComponents),
        type(inType),
        normalized(inNormalized),
        stride(inStride​),
        offset(inOffset​)
    {
    }

    GLuint attributeLocation;
    GLint numComponents;
    GLenum type;
    GLboolean normalized;
    GLsizei stride;
    GLintptr offset;
};

typedef std::vector<OpenGLVertexElement> OpenGLVertexElementList;

class OpenGLVertexDeclaration : public IVertexDeclaration
{
public:
    explicit OpenGLVertexDeclaration(const OpenGLVertexElementList &inElements) :
        vertexElements(inElements)
    {
    }

public:
    OpenGLVertexElementList vertexElements;
};

class OpenGLShaderCompiler
{
public:
    enum { InfoLogSize = 1024 * 4 };
    enum Error
    {
        NoError,
        FailedToCompile,
        FailedToLink
    };

    OpenGLShaderCompiler() :
        errorFlag(NoError)
    {
    }

    ~OpenGLShaderCompiler();
    Error CompileShaderStage(GLenum stage, const ShaderBytecode &bytecode);
    Error LinkAndClearShaderStages(GLuint &outProgram); // Note: this clears any previous error flag
    Error GetErrorFlag() const { return errorFlag; }

private:
    std::vector<GLuint> compiledShaderStages;
    Error errorFlag;
};

class OpenGLShaderProgram : public IShaderProgram
{
public:
    OpenGLShaderProgram(GLuint inResource) :
        resource(inResource)
    {
    }

    virtual ~OpenGLShaderProgram();
    virtual int32_t GetParameterLocation(const char *name);
    virtual void SetFloatArray(int32_t location, size_t num, const float *values);
    virtual void SetVec3(int32_t location, const float *values);
    virtual void SetMat3(int32_t location, const float *values);
    virtual void SetMat4(int32_t location, const float *values);

    GLuint resource;
};

class OpenGLSamplerState : public ISamplerState
{
public:
    OpenGLSamplerState() :
        magFilter(GL_NEAREST),
        minFilter(GL_NEAREST),
        wrapS(GL_REPEAT),
        wrapT(GL_REPEAT),
        wrapR(GL_REPEAT),
        LODBias(0),
        maxAnisotropy(1)
    {
    }
    virtual ~OpenGLSamplerState();

    GLuint resource;
    GLint magFilter;
    GLint minFilter;
    GLint wrapS;
    GLint wrapT;
    GLint wrapR;
    GLint LODBias;
    GLint maxAnisotropy;
};

struct OpenGLTextureFormatInfo
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
        PixelFormat inFormat) :
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
    OpenGLBaseTexture2D(uint32_t inWidth, uint32_t inHeight, uint32_t inNumMips, PixelFormat inFormat) :
        ITexture2D(inWidth, inHeight, inNumMips, inFormat)
    {
    }
};

class OpenGLBaseTextureCube : public ITextureCube
{
public:
    OpenGLBaseTextureCube(uint32_t inWidth, uint32_t inHeight, uint32_t inNumMips, PixelFormat inFormat) :
        ITextureCube(inWidth, inHeight, inNumMips, inFormat)
    {
    }
};

typedef OpenGLTextureBase<ITexture>              OpenGLTexture;
typedef OpenGLTextureBase<OpenGLBaseTexture2D>   OpenGLTexture2D;
typedef OpenGLTextureBase<OpenGLBaseTextureCube> OpenGLTextureCube;

class OpenGLRenderDevice : public IRenderDevice
{
public:
    OpenGLRenderDevice() : isInitialized(false) {}
    virtual ~OpenGLRenderDevice() { Shutdown(); }

    virtual void Init();
    virtual void Shutdown();

    virtual std::shared_ptr<IBuffer> CreateBuffer(int usageFlags, const void *data, size_t size);
    virtual std::shared_ptr<IVertexDeclaration> CreateVertexDelclaration(const VertexElementList &vertexElements);
    virtual std::shared_ptr<IShaderProgram> CreateShaderProgram(const ShaderBytecode &VS, const ShaderBytecode &FS);
    virtual std::shared_ptr<IShaderProgram> CreateShaderProgram(const ShaderBytecode &VS, const ShaderBytecode &GS, const ShaderBytecode &FS);
    virtual void SetShaderProgram(const std::shared_ptr<IShaderProgram> program);
    virtual std::shared_ptr<ITexture2D> CreateTexture2D(int32_t width, int32_t height, PixelFormat format, int32_t numMipMaps, const void *data);
    virtual std::shared_ptr<ITextureCube> CreateTextureCube(int32_t width, int32_t height, PixelFormat format, const void *data[]);
    virtual void SetTexture(uint32_t textureIndex, const std::shared_ptr<ITexture> texture);
    virtual std::shared_ptr<ISamplerState> OpenGLRenderDevice::CreateSamplerState(const SamplerStateInitializer &initializer);
    virtual void SetSamplerState(uint32_t textureIndex, const std::shared_ptr<ISamplerState> state);

    virtual void Clear(uint32_t targets, const glm::vec4 color, float depth = 1.0f);
    virtual void Render(const Surface *surf, const ICamera *camera);

private:
    GLuint vaoId;
    std::shared_ptr<OpenGLShaderProgram> currentShaderProgram;
    std::unordered_map<VertexElementList, std::shared_ptr<OpenGLVertexDeclaration>, VertexElementListHasher> vertexDeclarationCache;
    std::unordered_map<SamplerStateInitializer, std::shared_ptr<OpenGLSamplerState>, SamplerStateInitializerHasher> samplerStateCache;
    uint32_t imageFilterMaxAnisotropy;
    bool isInitialized;
};

} // renderer