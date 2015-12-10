#pragma once

namespace engine
{
namespace priv
{

struct ObjIndex
{
    uint16_t v;
    uint16_t vt;
    uint16_t vn;
};

typedef std::vector<ObjIndex> ObjFace;
typedef std::vector<ObjFace> ObjFaceGroup;

struct ObjVertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
};

struct ObjMaterial
{
    std::string name;
    glm::vec3 ambientColor;
    glm::vec3 diffuseColor;
    glm::vec3 specularColor;
    std::string ambientTexture;
    std::string diffuseTexture;
    std::string specularTexture;
    float dissolve;
    float shininess;
};

typedef std::unordered_map<std::string, ObjMaterial> ObjMaterialMap;

struct ObjSurface
{
    std::string name;
    std::vector<ObjVertex> vertices;
    std::vector<uint16_t> indices;
    ObjMaterial *material;
};

struct ObjModel
{
    std::string name;
    std::vector<ObjSurface> surfaces;
    ObjMaterialMap materials;
};

bool MTL_Load(const char *filename, ObjMaterialMap &materials);
ObjModel *OBJ_Load(const char *filename);
void OBJ_Free(ObjModel *model);

} // priv
} // engine