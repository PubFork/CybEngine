#pragma once
#include "RenderDevice.h"

namespace renderer
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

    static std::shared_ptr<Model> LoadOBJ(std::shared_ptr<renderer::IRenderDevice> device, const std::string &filename);

    void Render(std::shared_ptr<renderer::IRenderDevice> device, const ICamera *camera);

private:
    std::string name;
    SurfaceList surfaces;
};

} // engine