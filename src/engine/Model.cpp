#include "stdafx.h"
#include "Model.h"

#include "core/Log.h"
#include "core/FileUtils.h"
#include "Model_obj.h"

namespace engine
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

void Model::Render(std::shared_ptr<renderer::RenderDevice> device, const glm::mat4 &transform)
{
    for (const auto &surf : surfaces) {
        device->Render(&surf, transform);
    }
}

std::shared_ptr<Model> Model::LoadOBJ(std::shared_ptr<renderer::RenderDevice> device, std::shared_ptr<renderer::ShaderSet> shader, const std::string &filename)
{
    priv::ObjModel *objModel = priv::OBJ_Load(filename.c_str());
    THROW_FATAL_COND(!objModel, std::string("Failed to read model " + filename));
    auto model = std::make_shared<Model>(objModel->name.empty() ? "<unknown>" : objModel->name);

    for (auto &objSurf : objModel->surfaces) {
        renderer::Surface surf;
        renderer::SurfaceGeometry *geo = &surf.geometry;
        geo->vertexBuffer = device->CreateBuffer(renderer::Buffer::Vertex, &objSurf.vertices[0], VECTOR_BYTESIZE(objSurf.vertices));
        geo->format = renderer::VertexFormat_Standard;
        geo->indexBuffer = device->CreateBuffer(renderer::Buffer::Index, &objSurf.indices[0], VECTOR_BYTESIZE(objSurf.indices));
        geo->indexCount = (uint32_t)objSurf.indices.size();
        geo->rasterState = renderer::Raster_DrawSolid | renderer::Raster_PrimTriangle | renderer::Raster_CullBack;

        renderer::SurfaceMaterial *mat = &surf.material;
        mat->shader = shader;
        if (!objSurf.material->diffuseTexture.empty())
        {
            std::string path = core::GetBasePath(filename.c_str()) + objSurf.material->diffuseTexture;
            mat->texture[0] = renderer::globalImages->ImageFromFile(path.c_str(), renderer::Sample_Anisotropic);
        }
        
        surf.name = objSurf.name;
        model->AddSurface(surf);
    }

    priv::OBJ_Free(objModel);
    return model;
}

} // engine