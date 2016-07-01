#pragma once
#include "RenderDevice.h"

namespace renderer
{

class Model
{
public:
    Model(const std::string &inName);
    ~Model();

    void InitEmpty(const std::string &inName);
    const std::string &GetName() const { return name; }
    void AddSurface(renderer::Surface surf);
    void Render(std::shared_ptr<renderer::IRenderDevice> device, const ICamera *camera);

private:
    std::string name;
    std::list<renderer::Surface> surfaceList;
};

std::shared_ptr<Model> LoadOBJModel(std::shared_ptr<renderer::IRenderDevice> device, const std::string &filename);

} // engine