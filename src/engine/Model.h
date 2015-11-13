#pragma once

#include "renderer/RenderDevice.h"

namespace engine
{

class Model
{
public:
    typedef std::list<renderer::Surface> SurfaceList;

    Model(const std::string &modelName);
    ~Model();

    void InitEmpty(const std::string &modelName);
    const std::string &GetName() const { return name; }

    void AddSurface(renderer::Surface surf);
    size_t NumSurfaces() const { return surfaces.size(); }
    const SurfaceList &GetSurfaces() const { return surfaces; }

    virtual void Load(const std::string &filename);

private:
    std::string name;
    SurfaceList surfaces;
};

} // engine