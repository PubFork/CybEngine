#include "Precompiled.h"
#include "SkyBox.h"
#include "Renderer/Texture.h"

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

    surface.name = "SkyBox";
    surface.geometry = renderer::SurfaceGeometry(
        renderer::Primitive_TriangleList,
        device->CreateVertexDelclaration(vertexLayout),
        device->CreateBuffer(renderer::Buffer_Vertex | renderer::Buffer_ReadOnly, skyboxVertices, sizeof(skyboxVertices)),
        nullptr,
        0,
        36
    );

    surface.material.texture[0] = renderer::globalTextureCache->LoadTextureCubeFromFiles(textureFileNames);
    surface.material.sampler[0] = device->CreateSamplerState(renderer::SamplerStateInitializer(
        renderer::SamplerFilter_Anisotropic,
        renderer::SamplerWrap_Clamp,
        renderer::SamplerWrap_Clamp,
        renderer::SamplerWrap_Clamp
    ));

    surface.rasterState = renderer::RasterizerState(renderer::CullMode_CW, renderer::FillMode_Solid);
    surface.depthState = renderer::DepthBufferState(true, false, renderer::CmpFunc_LessEqual);
}