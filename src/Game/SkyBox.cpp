#include "Precompiled.h"
#include "SkyBox.h"
#include "Renderer/Texture.h"
#include "Renderer/Definitions.h"

void CreateSkyBoxSurface(std::shared_ptr<renderer::IRenderDevice> device, renderer::Surface &surface, const char *textureFileNames[6])
{
    const float skyboxVertices[] = 
    {
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f
    };

    const renderer::VertexElementList vertexLayout = 
    {
        { renderer::VertexElement(renderer::VertexElementUsage_Position,  renderer::VertexElementFormat_Float3, 0, sizeof(float)*3) }
    };

    surface.Clear();
    surface.drawStateFlags = DrawState_DepthTest_LessEqual | DrawState_Cull_CW;
    surface.vertexBuffer = device->CreateBuffer(renderer::Buffer_Vertex | renderer::Buffer_ReadOnly, skyboxVertices, sizeof(skyboxVertices));
    surface.vertexDeclaration = device->CreateVertexDelclaration(vertexLayout);
    surface.primitiveCount = 36;

    surface.material.texture[0] = renderer::globalTextureCache->LoadTextureCubeFromFiles(textureFileNames);
    surface.material.sampler[0] = device->CreateSamplerState(renderer::SamplerStateInitializer(
        renderer::SamplerFilter_Anisotropic,
        renderer::SamplerWrap_Clamp,
        renderer::SamplerWrap_Clamp,
        renderer::SamplerWrap_Clamp
    ));
}