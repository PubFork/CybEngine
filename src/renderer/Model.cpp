#include "Precompiled.h"
#include "Definitions.h"
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
    static const renderer::VertexElementList vertexElements = 
    {
        { VertexElement(renderer::VertexElementUsage_Position,  renderer::VertexElementFormat_Float3, offsetof(OBJ_Vertex, position), sizeof(OBJ_Vertex)) },
        { VertexElement(renderer::VertexElementUsage_Normal,    renderer::VertexElementFormat_Float3, offsetof(OBJ_Vertex, normal),   sizeof(OBJ_Vertex)) },
        { VertexElement(renderer::VertexElementUsage_Tangent,   renderer::VertexElementFormat_Float3, offsetof(OBJ_Vertex, tangent),  sizeof(OBJ_Vertex)) },
        { VertexElement(renderer::VertexElementUsage_TexCoord0, renderer::VertexElementFormat_Float2, offsetof(OBJ_Vertex, texCoord), sizeof(OBJ_Vertex)) }
    };
    auto vertexDeclaration = device->CreateVertexDelclaration(vertexElements);

    auto objModel = OBJ_CompileRawModel(OBJ_LoadModel(filename.c_str()));
    //THROW_FATAL_COND(!objModel, std::string("Failed to read model " + filename));
    auto model = std::make_shared<Model>(objModel->name.empty() ? "<unknown>" : objModel->name);

    for (auto &objSurface : objModel->surfaces)
    {
        renderer::Surface surface;

        surface.Clear();
        surface.vertexBuffer = device->CreateBuffer(Buffer_Vertex | Buffer_ReadOnly, &objSurface.vertices[0], sizeof(OBJ_Vertex) * objSurface.vertices.size());
        surface.vertexDeclaration = vertexDeclaration;
        surface.indexBuffer = device->CreateBuffer(Buffer_Index | Buffer_ReadOnly, &objSurface.indices[0], sizeof(uint32_t) * objSurface.indices.size());
        surface.indexCount = (uint32_t)objSurface.indices.size();

        renderer::SurfaceMaterial *mat = &surface.material;
        if (!objSurface.material.diffuseTexture.empty())
        {
            mat->texture[0] = renderer::globalTextureCache->LoadTexture2DFromFile(objSurface.material.diffuseTexture.c_str());
        }

        if (!objSurface.material.specularTexture.empty())
        {
            mat->texture[1] = renderer::globalTextureCache->LoadTexture2DFromFile(objSurface.material.specularTexture.c_str());
        }

        if (!objSurface.material.bumpTexture.empty())
        {
            mat->texture[2] = renderer::globalTextureCache->LoadTexture2DFromFile(objSurface.material.bumpTexture.c_str());
        }

        mat->ambient = objSurface.material.ambientColor;
        mat->diffuse = objSurface.material.diffuseColor;
        mat->specular = objSurface.material.specularColor;
        mat->shininess = objSurface.material.shininess;

        // add surface to the model
        model->AddSurface(surface);
    }

    return model;
}

} // engine