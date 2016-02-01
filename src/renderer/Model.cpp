#include "stdafx.h"
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

void Model::Render(std::shared_ptr<renderer::RenderDevice> device, const glm::mat4 &transform, renderer::PipelineState &pstate)
{
    for (const auto &surf : surfaces) {
        device->Render(&surf, transform, pstate);
    }
}

std::shared_ptr<Model> Model::LoadOBJ(std::shared_ptr<renderer::RenderDevice> device, const std::string &filename)
{
    priv::ObjModel *objModel = priv::OBJ_Load(filename.c_str());
    THROW_FATAL_COND(!objModel, std::string("Failed to read model " + filename));
    auto model = std::make_shared<Model>(objModel->name.empty() ? "<unknown>" : objModel->name);

    for (auto &objSurf : objModel->surfaces) {
        renderer::Surface convertedSurface;
        renderer::SurfaceGeometry *convertedGeometry = &convertedSurface.geometry;
        
        convertedGeometry->vertexBuffer = device->CreateBuffer(renderer::Buffer::Vertex, &objSurf.vertices[0], VECTOR_BYTESIZE(objSurf.vertices));
//        convertedGeometry->format = renderer::VertexFormat_Standard;
        convertedGeometry->indexBuffer = device->CreateBuffer(renderer::Buffer::Index, &objSurf.indices[0], VECTOR_BYTESIZE(objSurf.indices));
        convertedGeometry->indexCount = (uint32_t)objSurf.indices.size();
//        convertedGeometry->rasterState = renderer::Raster_DrawSolid | renderer::Raster_PrimTriangle | renderer::Raster_CullBack;

        renderer::SurfaceMaterial *mat = &convertedSurface.material;
        //mat->shader = shader;
        if (!objSurf.material->diffuseTexture.empty())
        {
            std::string path = GetBasePath(filename.c_str()) + objSurf.material->diffuseTexture;
            mat->texture[0] = renderer::globalImages->ImageFromFile(path.c_str(), renderer::Image::Sample_Anisotropic);
        }
        
        convertedSurface.name = objSurf.name;
        model->AddSurface(convertedSurface);
    }

    priv::OBJ_Free(objModel);
    return model;
}

} // engine