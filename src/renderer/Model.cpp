#include "Precompiled.h"
#include "Model.h"
#include "Base/Debug.h"
#include "Base/Algorithm.h"
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
    for (const auto &surf : surfaces) 
    {
        device->Render(&surf, camera);
    }
}

std::shared_ptr<Model> Model::LoadOBJ(std::shared_ptr<renderer::IRenderDevice> device, const std::string &filename)
{
    static const renderer::VertexElementList vertexElements = {
        { VertexElement(renderer::VertexElementUsage_Position,  renderer::VertexElementFormat_Float3, offsetof(OBJ_Vertex, position), sizeof(OBJ_Vertex)) },
        { VertexElement(renderer::VertexElementUsage_Normal,    renderer::VertexElementFormat_Float3, offsetof(OBJ_Vertex, normal),   sizeof(OBJ_Vertex)) },
        { VertexElement(renderer::VertexElementUsage_TexCoord0, renderer::VertexElementFormat_Float2, offsetof(OBJ_Vertex, texCoord), sizeof(OBJ_Vertex)) }
    };
    auto vertexDeclaration = device->CreateVertexDelclaration(vertexElements);

    auto objModel = OBJ_CompileRawModel(OBJ_LoadModel(filename.c_str()));
    //THROW_FATAL_COND(!objModel, std::string("Failed to read model " + filename));
    auto model = std::make_shared<Model>(objModel->name.empty() ? "<unknown>" : objModel->name);

    for (auto &surface : objModel->surfaces)
    {
        renderer::Surface drawSurface;

        // copy geometry
        renderer::SurfaceGeometry *geo = &drawSurface.geometry;
        geo->primitive = Primitive_TriangleList;
        geo->vertexDeclaration = vertexDeclaration;
        geo->VBO = device->CreateBuffer(Buffer_Vertex | Buffer_ReadOnly, &surface.vertices[0], sizeof(OBJ_Vertex) * surface.vertices.size());
        geo->IBO = device->CreateBuffer(Buffer_Index | Buffer_ReadOnly, &surface.indices[0], sizeof(uint16_t) * surface.indices.size());
        geo->indexCount = (uint32_t)surface.indices.size();

        // copy material
        renderer::SurfaceMaterial *mat = &drawSurface.material;
        if (!surface.material.diffuseTexture.empty())
        {
            mat->texture[0] = renderer::globalTextureCache->LoadTexture2DFromFile(surface.material.diffuseTexture.c_str());
        }

        mat->ambient = surface.material.ambientColor;
        mat->diffuse = surface.material.diffuseColor;
        mat->specular = surface.material.specularColor;
        mat->shininess = surface.material.shininess;

        // finish up and add to model
        drawSurface.name = surface.name;
        drawSurface.rasterState = RasterizerState(renderer::CullMode_CW, renderer::FillMode_Solid);
        model->AddSurface(drawSurface);
    }

    return model;
}

} // engine