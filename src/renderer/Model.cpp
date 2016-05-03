#include "Precompiled.h"
#include "Model.h"
#include "Base/Debug.h"
#include "Base/Macros.h"
#include "Texture.h"
#include "Model_obj.h"

namespace renderer
{

Model::Model()
{
    InitEmpty("<unknown>");
} 

Model::Model(const std::string &modelName)
{
    InitEmpty(modelName);
}

Model::~Model()
{
}

void Model::InitEmpty(const std::string &modelName)
{
    surfaces.clear();
    name = modelName;
}

void Model::AddSurface(renderer::Surface surf)
{
    surfaces.push_back(surf);
}

void Model::Render(std::shared_ptr<renderer::IRenderDevice> device, const ICamera *camera)
{
    for (const auto &surf : surfaces) {
        device->Render(&surf, camera);
    }
}

std::shared_ptr<Model> Model::LoadOBJ(std::shared_ptr<renderer::IRenderDevice> device, const std::string &filename)
{
    static const renderer::VertexElementList vertexElements = {
        { VertexElement(renderer::VertexElementUsage_Position,  renderer::VertexElementFormat_Float3, offsetof(priv::OBJVertex, position), sizeof(priv::OBJVertex)) },
        { VertexElement(renderer::VertexElementUsage_Normal,    renderer::VertexElementFormat_Float3, offsetof(priv::OBJVertex, normal),   sizeof(priv::OBJVertex)) },
        { VertexElement(renderer::VertexElementUsage_TexCoord0, renderer::VertexElementFormat_Float2, offsetof(priv::OBJVertex, uv),       sizeof(priv::OBJVertex)) }
    };
    auto vertexDeclaration = device->CreateVertexDelclaration(vertexElements);

    priv::ObjModel *objModel = priv::OBJ_Load(filename.c_str());
    THROW_FATAL_COND(!objModel, std::string("Failed to read model " + filename));
    auto model = std::make_shared<Model>(objModel->name.empty() ? "<unknown>" : objModel->name);

    for (auto &objSurface : objModel->surfaces) {
        renderer::Surface convertedSurface;
        renderer::SurfaceGeometry *convertedGeometry = &convertedSurface.geometry;

        // copy geometry
        convertedGeometry->vertexDeclaration = vertexDeclaration;
        convertedGeometry->VBO = device->CreateBuffer(&objSurface.vertices[0], CONTAINER_CONTENT_SIZE(objSurface.vertices), Buffer_Vertex | Buffer_ReadOnly);
        convertedGeometry->IBO = device->CreateBuffer(&objSurface.indices[0], CONTAINER_CONTENT_SIZE(objSurface.indices), Buffer_Index | Buffer_ReadOnly);
        convertedGeometry->indexCount = (uint32_t)objSurface.indices.size();
        
        // load textures
        renderer::SurfaceMaterial *mat = &convertedSurface.material;
        if (!objSurface.material->diffuseTexture.empty())
        {
            mat->texture[0] = renderer::globalTextureCache->LoadTexture2DFromFile(objSurface.material->diffuseTexture.c_str());
        }

        // finish up and add to model
        convertedSurface.name = objSurface.name;
        convertedSurface.rasterState = RasterizerState(renderer::CullMode_CW, renderer::FillMode_Solid);
        model->AddSurface(convertedSurface);
    }

    priv::OBJ_Free(objModel);
    return model;
}

} // engine