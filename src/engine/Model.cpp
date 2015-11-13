#include "stdafx.h"
#include "Model.h"

namespace engine
{

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

void Model::Load(const std::string &filename)
{
    InitEmpty(filename);
}

} // engine