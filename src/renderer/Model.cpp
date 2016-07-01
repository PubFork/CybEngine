#include "Precompiled.h"
#include "Renderer/Definitions.h"
#include "Renderer/Model.h"
#include "Renderer/Texture.h"
#include "Renderer/Model_obj.h"
#include "Base/Debug.h"
#include "Base/Algorithm.h"

namespace renderer
{

Model::Model(const std::string &inName)
{
    InitEmpty(inName);
}

Model::~Model()
{
}

void Model::InitEmpty(const std::string &inName)
{
    name = inName;
    surfaceList.clear();
}

void Model::AddSurface(renderer::Surface surf)
{
    surfaceList.push_back(surf);
}

void Model::Render(std::shared_ptr<renderer::IRenderDevice> device, const ICamera *camera)
{
    for (const auto &surf : surfaceList)
    {
        device->Render(&surf, camera);
    }
}

std::shared_ptr<Model> LoadOBJModel(std::shared_ptr<renderer::IRenderDevice> device, const std::string &filename)
{
    static const renderer::VertexElementList vertexElements = 
    {
        { VertexElement(VertexElementUsage_Position,  VertexElementFormat_Float3, offsetof(OBJ_Vertex, position)) },
        { VertexElement(VertexElementUsage_Normal,    VertexElementFormat_Float3, offsetof(OBJ_Vertex, normal))   },
        { VertexElement(VertexElementUsage_Tangent,   VertexElementFormat_Float3, offsetof(OBJ_Vertex, tangent))  },
        { VertexElement(VertexElementUsage_TexCoord0, VertexElementFormat_Float2, offsetof(OBJ_Vertex, texCoord)) }
    };
    auto vertexDeclaration = device->CreateVertexDelclaration(vertexElements, sizeof(OBJ_Vertex));

    auto rawObjModel = OBJ_LoadModel(filename);
    RETURN_NULL_IF(!rawObjModel);

    auto objModel = OBJ_CompileRawModel(rawObjModel);
    auto model = std::make_shared<Model>(objModel->name);

    for (auto &objSurface : objModel->surfaces)
    {
        renderer::Surface surface;

        surface.Clear();
        surface.vertexBuffer = device->CreateBuffer(Buffer_Vertex | Buffer_ReadOnly, &objSurface.vertices[0], sizeof(OBJ_Vertex) * objSurface.vertices.size());
        surface.vertexDeclaration = vertexDeclaration;
        surface.indexBuffer = device->CreateBuffer(Buffer_Index | Buffer_32BitIndex | Buffer_ReadOnly, &objSurface.indices[0], sizeof(uint32_t) * objSurface.indices.size());
        surface.numVertices = (uint32_t)objSurface.vertices.size();
        surface.numIndices = (uint32_t)objSurface.indices.size();

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