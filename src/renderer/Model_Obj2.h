#pragma once

namespace dev
{

typedef glm::vec3 OBJ_Vertex;
typedef glm::vec2 OBJ_TexCoord;
typedef uint16_t OBJ_Index;

struct OBJ_Face
{
    std::vector<OBJ_Index> vertices;
    std::vector<OBJ_Index> texCoords;
};

struct OBJ_FaceGroup
{
    OBJ_FaceGroup() :
        name("<unknown>")
    {
    }

    std::string name;
    std::vector<OBJ_Face> faces;
};

//
// Raw OBJ model
// Note: Contains no normals as they're manually calculated when compiling
//
struct OBJ_RawModel
{
    OBJ_RawModel() :
        name("<unknown>")
    {
    }

    std::string name;
    std::vector<OBJ_Vertex> vertices;
    std::vector<OBJ_TexCoord> texCoords;
    std::vector<OBJ_FaceGroup> faceGroups;
};

} // dev