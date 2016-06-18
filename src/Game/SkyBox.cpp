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
        { renderer::VertexElement(VertexElementUsage_Position,  VertexElementFormat_Float3, 0, sizeof(float)*3) }
    };

    surface.Clear();
    surface.drawStateFlags = DrawState_DepthTest_LessEqual | DrawState_Cull_CW;
    surface.vertexBuffer = device->CreateBuffer(Buffer_Vertex | Buffer_ReadOnly, skyboxVertices, sizeof(skyboxVertices));
    surface.vertexDeclaration = device->CreateVertexDelclaration(vertexLayout);
    surface.primitiveCount = 36;

    surface.material.texture[0] = renderer::globalTextureCache->LoadTextureCubeFromFiles(textureFileNames);
    surface.material.sampler[0] = device->CreateSamplerState(renderer::SamplerStateInitializer(
        SamplerFilter_Anisotropic,
        SamplerWrap_Clamp,
        SamplerWrap_Clamp,
        SamplerWrap_Clamp
    ));
}