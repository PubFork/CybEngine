#pragma once

#include "renderer/RenderDevice.h"

namespace engine
{

class Model
{
public:
    typedef std::list<renderer::Surface> SurfaceList;

    Model();
    Model(const std::string &modelName);
    ~Model();

    void InitEmpty(const std::string &modelName);
    const std::string &GetName() const { return name; }

    void AddSurface(renderer::Surface surf);
    size_t NumSurfaces() const { return surfaces.size(); }
    const SurfaceList &GetSurfaces() const { return surfaces; }

    void LoadOBJ(std::shared_ptr<renderer::RenderDevice> device, std::shared_ptr<renderer::ShaderSet> shader, const std::string &filename);
    void Render(std::shared_ptr<renderer::RenderDevice> device, const glm::mat4 &transform);

private:
    std::string name;
    SurfaceList surfaces;
};

} // engine