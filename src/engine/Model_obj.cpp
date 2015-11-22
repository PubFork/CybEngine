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

inline uint16_t FixIndex(int idx)
{
    if (idx > 0)
        return (uint16_t)idx - 1;
    return 0;
}

inline Obj_VertexIndex ParseFaceIndex(const char *&token, Obj_VertexIndex &vi)
{
    vi = { 0, 0, 0 };

    vi.v = (uint16_t)atoi(token);
    token += strcspn(token, "/ \t\r");
    if (token[0] != '/')
        return vi;

    token++;
    
    // i//k
    if (token[0] == '/') {
        token++;
        vi.vn = (uint16_t)atoi(token);
        token += strcspn(token, "/ \t\r");
        return vi;
    }

    // i/j/k or i/j
    vi.vt = (uint16_t)atoi(token);
    token += strcspn(token, "/ \t\r");
    if (token[0] != '/')
        return vi;

    // i/j/k
    token++;
    vi.vn = (uint16_t)atoi(token);
    token += strcspn(token, "/ \t\r\n");

    return vi;
}

uint16_t UpdateVertex(std::map<Obj_VertexIndex, uint16_t> &vertexCache, Obj_Surface &surface, const std::vector<float> &positions, const std::vector<float> &normals, const std::vector<float> &texCoords, const Obj_VertexIndex &vi)
{
    auto it = vertexCache.find(vi);
    if (it != vertexCache.end())
        return it->second;

    uint32_t vertexIndex = FixIndex(vi.v) * 3;;
    assert(positions.size() > (unsigned int)(vertexIndex + 2));

    Obj_Vertex vertex = { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } };
    vertex.position[0] = positions[vertexIndex + 0];
    vertex.position[1] = positions[vertexIndex + 1];
    vertex.position[2] = positions[vertexIndex + 2];

    if (vi.vn)
    { 
        uint32_t normalIndex = FixIndex(vi.vn) * 3;
        vertex.normal[0] = normals[normalIndex + 0];
        vertex.normal[1] = normals[normalIndex + 1];
        vertex.normal[2] = normals[normalIndex + 2];
    }

    if (vi.vt)
    {
        uint32_t texCoordIndex = FixIndex(vi.vt) * 2;
        vertex.texCoord[0] = texCoords[texCoordIndex + 0];
        vertex.texCoord[1] = texCoords[texCoordIndex + 1];
    }

    surface.vertices.push_back(vertex);
    uint16_t idx = static_cast<uint16_t>(surface.vertices.size() - 1);
    vertexCache[vi] = idx;

    return idx;
}

bool ExportFaceGroupToSurface(Obj_Surface &surface, const Obj_FaceGroup &faceGroup, const std::vector<float> &positions, const std::vector<float> &normals, const std::vector<float> &texCoords, const std::string name)
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

            uint16_t v0 = UpdateVertex(vertexCache, surface, positions, normals, texCoords, i0);
            uint16_t v1 = UpdateVertex(vertexCache, surface, positions, normals, texCoords, i1);
            uint16_t v2 = UpdateVertex(vertexCache, surface, positions, normals, texCoords, i2);

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
        if (!strncmp(linebuf, "v ", 2)) {
            linebuf += 2;
            v.push_back(ParseFloat(linebuf));
            v.push_back(ParseFloat(linebuf));
            v.push_back(ParseFloat(linebuf));
            continue;
        }

        // vertex normal
        if (!strncmp(linebuf, "vn ", 3))
        {
            linebuf += 3;
            vn.push_back(ParseFloat(linebuf));
            vn.push_back(ParseFloat(linebuf));
            vn.push_back(ParseFloat(linebuf));
            continue;
        }

        // vertex tex coord
        if (!strncmp(linebuf, "vt ", 3))
        {
            linebuf += 3;
            vt.push_back(ParseFloat(linebuf));
            vt.push_back(ParseFloat(linebuf));
            continue;
        }

        // face
        if (!strncmp(linebuf, "f ", 2)) {
            linebuf += 2;
            linebuf += strspn(linebuf, " \t");

            face.clear();
            while (!IsNewLine(linebuf[0])) {
                ParseFaceIndex(linebuf, vertexIndex);
                face.push_back(vertexIndex);

                size_t n = strspn(linebuf, " \t\r");
                linebuf += n;
            }

            faceGroup.push_back(face);
            continue;
        }

        // group name
        if (!strncmp(linebuf, "g ", 2)) {
            // save current surface to model
            Obj_Surface surf;

            if (ExportFaceGroupToSurface(surf, faceGroup, v, vn, vt, name)) {
                model->surfaces.push_back(surf);
            }

            // reset facegroup and get the name of the next one
            faceGroup = Obj_FaceGroup();

            // parse name for next surface
            linebuf += 2;
            linebuf += strspn(linebuf, " \t");
            size_t e = strcspn(linebuf, "\r\n");
            name = std::string(linebuf, &linebuf[e]);
            linebuf += e;
        }
    }

    Obj_Surface surf;
    if (ExportFaceGroupToSurface(surf, faceGroup, v, vn, vt, name)) {
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