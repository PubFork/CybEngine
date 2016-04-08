#include "Precompiled.h"
#include "Model.h"

#include "Base/Debug.h"
#include "Base/FileUtils.h"
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

void Model::Render(std::shared_ptr<renderer::IRenderDevice> device, const glm::mat4 &transform)
{
    for (const auto &surf : surfaces) {
        device->Render(&surf, transform);
    }
}

std::shared_ptr<Model> Model::LoadOBJ(std::shared_ptr<renderer::IRenderDevice> device, const std::string &filename)
{
    priv::ObjModel *objModel = priv::OBJ_Load(filename.c_str());
    THROW_FATAL_COND(!objModel, std::string("Failed to read model " + filename));
    auto model = std::make_shared<Model>(objModel->name.empty() ? "<unknown>" : objModel->name);

    for (auto &objSurface : objModel->surfaces) {
        renderer::Surface convertedSurface;
        renderer::SurfaceGeometry *convertedGeometry = &convertedSurface.geometry;

        // copy geometry
        convertedGeometry->VBO = device->CreateVertexBuffer(&objSurface.vertices[0], VECTOR_BYTESIZE(objSurface.vertices));
        convertedGeometry->IBO = device->CreateIndexBuffer(&objSurface.indices[0], VECTOR_BYTESIZE(objSurface.indices));
        convertedGeometry->indexCount = (uint32_t)objSurface.indices.size();

        //device->CreateSurface(&vertices, &indices, &material);
        
        // load materials
        renderer::SurfaceMaterial *mat = &convertedSurface.material;
        if (!objSurface.material->diffuseTexture.empty())
        {
            std::string path = GetBasePath(filename.c_str()) + objSurface.material->diffuseTexture;
            mat->texture[0] = device->ImageFromFile(path.c_str());
        }

        // finish up and add to model
        convertedSurface.pipelineState = device->BuiltintPipelineState(renderer::BuiltintPipelineState_Default);
        convertedSurface.name = objSurface.name;
        model->AddSurface(convertedSurface);
    }

    priv::OBJ_Free(objModel);
    return model;
}

} // engine