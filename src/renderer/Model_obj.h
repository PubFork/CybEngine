#pragma once
#include "renderer/RenderDevice.h"

namespace renderer
{

typedef uint32_t OBJ_Index;

struct OBJ_Edge
{
    //OBJ_Edge() = default;
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

typedef std::unordered_map<std::string, OBJ_Material> OBJ_MaterialMap;

struct OBJ_RawModel
{
    OBJ_RawModel(const std::string &inName) :
        name(inName)
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
    OBJ_MaterialMap materials;
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
    OBJ_TriSurface(const std::string &inName, const OBJ_Material &inMaterial) :
        name(inName),
        material(inMaterial)
    {
    }

    std::string name;
    std::vector<OBJ_Vertex> vertices;
    std::vector<OBJ_Index> indices;
    OBJ_Material material;
};

struct OBJ_CompiledModel
{
    OBJ_CompiledModel(const std::string &inName) :
        name(inName)
    {
    }

    std::string name;
    std::vector<OBJ_TriSurface> surfaces;
};

std::shared_ptr<OBJ_RawModel> OBJ_LoadModel(const char *filename);
std::shared_ptr<OBJ_CompiledModel> OBJ_CompileRawModel(const std::shared_ptr<OBJ_RawModel> rawModel);

} // engine