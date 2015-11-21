#include "stdafx.h"
#include "Model_obj.h"

#include "core/FileUtils.h"
#include "core/Log.h"

/*
 * This .obj loader has big parts copied straight from syoyo's tinyobjloader:
 * https://github.com/syoyo/tinyobjloader
 */

namespace engine
{
namespace obj_loader
{

inline bool operator<(const Obj_VertexIndex &a, const Obj_VertexIndex &b)
{
    if (a.v != b.v)
        return (a.v < b.v);
    if (a.vn != b.vn)
        return (a.vn < b.vn);
    if (a.vt != b.vt)
        return (a.vt < b.vt);

    return false;
}

inline bool IsNewLine(const char c)
{
    return (c == '\r') || (c == '\n') || (c == '\0');
}

inline float ParseFloat(const char *&token)
{
    token += strspn(token, " \t");
    float value = (float)atof(token);
    token += strcspn(token, " \t\r\n");
    return value;
}

inline void ParseFloat2(const char *&token, float &x, float &y)
{
    x = ParseFloat(token);
    y = ParseFloat(token);
}

inline void ParseFloat3(const char *&token, float &x, float &y, float &z)
{
    x = ParseFloat(token);
    y = ParseFloat(token);
    z = ParseFloat(token);
}

inline std::string ParseString(const char *&token)
{
    token += strspn(token, " \t");
    size_t e = strcspn(token, " \t\r\n");
    std::string str(token, &token[e]);
    token += e;
    return str;
}

inline uint16_t FixIndex(int idx)
{
    if (idx > 0)
        return (uint16_t)idx - 1;
    return (uint16_t)idx;
}

inline Obj_VertexIndex ParseFaceIndex(const char *&token, Obj_VertexIndex &vi)
{
    vi = { 0, 0, 0 };

    vi.v = FixIndex(atoi(token));
    token += strcspn(token, "/ \t\r");
    if (token[0] != '/')
        return vi;

    token++;
    
    // i//k
    if (token[0] == '/') {
        token++;
        vi.vn = FixIndex(atoi(token));
        token += strcspn(token, "/ \t\r");
        return vi;
    }

    // i/j/k or i/j
    vi.vt = FixIndex(atoi(token));
    token += strcspn(token, "/ \t\r");
    if (token[0] != '/')
        return vi;

    // i/j/k
    token++;
    vi.vn = FixIndex(atoi(token));
    token += strcspn(token, "/ \t\r\n");

    //DEBUG_LOG_TEXT("FaceIndex %d/%d/%d", vi.v+1, vi.vt+1, vi.vn+1);
    return vi;
}

uint16_t UpdateVertex(std::map<Obj_VertexIndex, uint16_t> &vertexCache, Obj_Surface &surface, const std::vector<float> &positions, const Obj_VertexIndex &vi)
{
    auto it = vertexCache.find(vi);
    if (it != vertexCache.end())
        return it->second;

    assert(positions.size() > (unsigned int)(3 * vi.v + 2));

    Obj_Vertex vertex;
    vertex.position[0] = positions[3 * vi.v + 0];
    vertex.position[1] = positions[3 * vi.v + 1];
    vertex.position[2] = positions[3 * vi.v + 2];

    surface.vertices.push_back(vertex);
    uint16_t idx = static_cast<uint16_t>(surface.vertices.size() - 1);
    vertexCache[vi] = idx;

    return idx;
}

bool ExportFaceGroupToSurface(Obj_Surface &surface, const Obj_FaceGroup &faceGroup, const std::vector<float> &positions, const std::string name)
{
    std::map<Obj_VertexIndex, uint16_t> vertexCache;

    if (faceGroup.empty())
        return false;

    // Flatten vertices and indices
    for (size_t i = 0; i < faceGroup.size(); i++) {
        const std::vector<Obj_VertexIndex> &face = faceGroup[i];

        Obj_VertexIndex i0 = face[0];
        Obj_VertexIndex i1 = {};
        Obj_VertexIndex i2 = face[1];

        size_t npolys = face.size();

        // Polygon -> triangle fan conversion
        for (size_t k = 2; k < npolys; k++) {
            i1 = i2;
            i2 = face[k];

            uint16_t v0 = UpdateVertex(vertexCache, surface, positions, i0);
            uint16_t v1 = UpdateVertex(vertexCache, surface, positions, i1);
            uint16_t v2 = UpdateVertex(vertexCache, surface, positions, i2);

            surface.indices.push_back(v0);
            surface.indices.push_back(v1);
            surface.indices.push_back(v2);
        }
    }
    
    surface.name = name;
    return true;
}

Obj_Model *OBJ_Load(const char *filename)
{
    core::FileReader file(filename);

    Obj_Model *model = new Obj_Model();

    std::vector<float> v;
    std::vector<float> vn;
    std::vector<float> vt;
    Obj_FaceGroup faceGroup;
    std::string name;

    Obj_Face face;
    Obj_VertexIndex vertexIndex;

    while (file.Peek() != -1) {
        size_t lineLength = 0;
        const char *linebuf = file.GetLine(&lineLength);

        // skip leading whitespaces
        while (isspace(*linebuf))
            linebuf++;

        // skip comments and empty lines
        if (linebuf[0] == '#' || linebuf[0] == '\n' || lineLength <= 1)
            continue;

        // vertex
        if (linebuf[0] == 'v' && isspace(linebuf[1])) {
            linebuf += 2;
            float x, y, z;
            ParseFloat3(linebuf, x, y, z);
            v.push_back(x);
            v.push_back(y);
            v.push_back(z);
            //DEBUG_LOG_TEXT("v %f %f %f", x, y, z);
            continue;
        }

        // face
        if (linebuf[0] == 'f' && isspace(linebuf[1])) {
            linebuf += 2;
            linebuf += strspn(linebuf, " \t");

            face.clear();
            while (!IsNewLine(linebuf[0])) {
                ParseFaceIndex(linebuf, vertexIndex);
                face.push_back(vertexIndex);

                size_t n = strspn(linebuf, " \t\r");
                linebuf += n;
            }

            //DEBUG_LOG_TEXT("====[ End face with %d edges", face.size());
            faceGroup.push_back(face);
            continue;
        }

        // group name
        if (linebuf[0] == 'g' && isspace(linebuf[1])) {
            // save current surface to model
            Obj_Surface surf;

            if (ExportFaceGroupToSurface(surf, faceGroup, v, name)) {
                model->surfaces.push_back(surf);
            }

            // reset facegroup and get the name of the next one
            faceGroup = Obj_FaceGroup();

            // parse name for next surface
            linebuf += 2;
            name = ParseString(linebuf);
            //DEBUG_LOG_TEXT("Group name %s", name.c_str());
        }
    }

    Obj_Surface surf;
    if (ExportFaceGroupToSurface(surf, faceGroup, v, name)) {
        model->surfaces.push_back(surf);
    }

    return model;
}

void OBJ_Free(Obj_Model *model)
{
    delete model;
}

} // obj_loader
} // engine