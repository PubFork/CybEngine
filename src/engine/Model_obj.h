#pragma once

namespace engine
{
namespace obj_loader
{

struct Obj_VertexIndex
{
    uint16_t v;
    uint16_t vt;
    uint16_t vn;
};

typedef std::vector<Obj_VertexIndex> Obj_Face;
typedef std::vector<Obj_Face> Obj_FaceGroup;

struct Obj_Vertex
{
    float position[3];
    float normal[3];
    float texCoord[2];
};

struct Obj_Surface
{
    std::string name;
    std::vector<Obj_Vertex> vertices;
    std::vector<uint16_t> indices;
    bool hasNormals;
    bool hasTexCoords;
};

struct Obj_Model
{
    std::string name;
    std::vector<Obj_Surface> surfaces;
};

Obj_Model *OBJ_Load(const char *filename);
void OBJ_Free(Obj_Model *model);

} // obj_loader
} // engine