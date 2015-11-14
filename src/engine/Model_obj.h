#pragma once

namespace engine
{

struct ObjSurface
{
    std::string name;
    std::vector<float> positions;
    std::vector<float> normals;
    std::vector<uint16_t> indices;
};

struct ObjModel
{
    std::string name;
    std::vector<ObjSurface> surfaces;
};

ObjModel *OBJ_Load(const char *filename);
void OBJ_Free(ObjModel *model);

} // engine