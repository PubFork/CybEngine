#pragma once
#include "renderer/RenderDevice.h"

namespace renderer
{

namespace priv
{

struct OBJVertex
{
    OBJVertex(const glm::vec3 &inPosition,
              const glm::vec3 &inNormal,
              const glm::vec2 &inUv) :
        position(inPosition),
        normal(inNormal),
        uv(inUv)
    {
    }

    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
};

struct ObjIndex
{
    uint16_t v;
    uint16_t vt;
    uint16_t vn;
};

typedef std::vector<ObjIndex> ObjFace;
typedef std::vector<ObjFace> ObjFaceGroup;

struct OBJ_Material
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

typedef std::unordered_map<std::string, OBJ_Material> ObjMaterialMap;

struct ObjSurface
{
    std::string name;
    std::vector<OBJVertex> vertices;
    std::vector<uint16_t> indices;
    OBJ_Material *material;
};

//
//   =< dev >=
////////////////////////////////////////////////////////////////////////////
namespace dev
{

typedef uint16_t OBJ_Index;

struct OBJ_Edge
{
    OBJ_Edge() = default;
    OBJ_Edge(const OBJ_Index inVertexIndex, const OBJ_Index inTexCoordIndex) :
        vertexIndex(inVertexIndex),
        texCoordIndex(inTexCoordIndex)
    {
    }

    OBJ_Index vertexIndex;          // this is also used as normal index
    OBJ_Index texCoordIndex;
};

struct OBJ_Face
{
    OBJ_Face() = default;

    // temporary for easy conversion from the deprecated code
    OBJ_Face(const ObjFace &src)
    {
        for (const auto &it : src)
        {
            edges.emplace_back(it.v, it.vt);
        }
    }

    std::vector<OBJ_Edge> edges;
    glm::vec3 normal;
};

struct OBJ_FaceGroup
{
    OBJ_FaceGroup(const std::string &faceGroupName) :
        name(faceGroupName)
    {
    }

    std::string name;
    std::string materialName;
    std::vector<OBJ_Face> faces;
};

struct OBJ_RawModel
{
    OBJ_RawModel() :
        name("<unknown>")
    {
    }

    OBJ_FaceGroup *CreateEmptyFaceGroup(const std::string &faceGroupName)
    {
        if (!faceGroups.empty() && faceGroups.back().faces.empty())
        {
            faceGroups.back().name = faceGroupName;
        } else
        {
            faceGroups.emplace_back(faceGroupName);
        }

        return &faceGroups.back();
    }

    std::string name;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texCoords;
    std::vector<OBJ_FaceGroup> faceGroups;
    std::unordered_map<std::string, OBJ_Material> materials;
};

struct OBJ_Vertex
{
    OBJ_Vertex() = default;
    OBJ_Vertex(const glm::vec3 &inPosition,
               const glm::vec3 &inNormal,
               const glm::vec2 &inTexCoord) :
        position(inPosition),
        normal(inNormal),
        texCoord(inTexCoord)
    {
    }

    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
};

struct OBJ_TriSurface
{
    std::string name;
    std::vector<OBJ_Vertex> vertices;
    std::vector<OBJ_Index> indices;
    OBJ_Material material;
};

struct OBJ_CompiledModel
{
    std::string name;
    std::vector<OBJ_TriSurface> surfaces;
};

}   // namespace dev

struct ObjModel
{
    std::string name;
    std::vector<ObjSurface> surfaces;
    ObjMaterialMap materials;
};

bool MTL_Load(const char *filename, ObjMaterialMap &materials);
std::shared_ptr<dev::OBJ_RawModel> OBJ_Load(const char *filename);
std::shared_ptr<dev::OBJ_CompiledModel> OBJ_CompileRawModel(const std::shared_ptr<dev::OBJ_RawModel> rawModel);

} // priv
} // engine