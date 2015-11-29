#include "stdafx.h"
#include "Model.h"

#include "Model_obj.h"

#define VECTOR_BYTESIZE(v) (sizeof(v[0]) * v.size())

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

void Model::LoadOBJ(std::shared_ptr<renderer::RenderDevice> device, std::shared_ptr<renderer::ShaderSet> shader, const std::string &filename)
{
    obj_loader::Obj_Model *objModel = obj_loader::OBJ_Load(filename.c_str());
    InitEmpty(objModel->name.empty() ? "<unknown>" : objModel->name);

    for (auto &objSurf : objModel->surfaces) {
        renderer::Surface surf;
        renderer::SurfaceGeometry *geo = &surf.geometry;
        geo->vertexBuffer = device->CreateBuffer(renderer::Buffer_Vertex, &objSurf.vertices[0], VECTOR_BYTESIZE(objSurf.vertices));
        geo->vfmt = renderer::VF_Standard;
        geo->indexBuffer = device->CreateBuffer(renderer::Buffer_Index, &objSurf.indices[0], VECTOR_BYTESIZE(objSurf.indices));
        geo->indexCount = (uint32_t)objSurf.indices.size();
        surf.shader = shader;
        surf.name = objSurf.name;

        AddSurface(surf);
    }

    obj_loader::OBJ_Free(objModel);
}

} // engine