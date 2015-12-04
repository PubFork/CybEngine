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
namespace priv
{

bool operator<(const ObjIndex &a, const ObjIndex &b)
{
    if (a.v != b.v)
        return (a.v < b.v);
    if (a.vn != b.vn)
        return (a.vn < b.vn);
    if (a.vt != b.vt)
        return (a.vt < b.vt);

    return false;
}

float ReadFloat(const char*& token)
{
    token += strspn(token, " \t");
    float value = (float)atof(token);
    token += strcspn(token, " \t\n");
    return value;
}

glm::vec2 ReadVec2(const char*& buffer)
{
    float x = ReadFloat(buffer);
    float y = ReadFloat(buffer);
    return glm::vec2(x, y);
}

glm::vec3 ReadVec3(const char*& buffer)
{
    float x = ReadFloat(buffer);
    float y = ReadFloat(buffer);
    float z = ReadFloat(buffer);
    return glm::vec3(x, y, z);
}

bool ReadToken(const char *&token, const char *elementStr)
{
    size_t elementLen = strlen(elementStr);
    if (!strncmp(token, elementStr, elementLen))
    {
        token += elementLen;
        token += strspn(token, " \t");
        return true;
    }

    return false;
}

// read a string from token, using newline as string terminator
std::string ReadString(const char *&token)
{
    size_t end = strcspn(token, "\n");
    std::string str(token, &token[end]);
    token += end;
    return str;
}

uint16_t FixIndex(int idx)
{
    return (uint16_t)(idx > 0 ? idx - 1 : 0);
}

ObjIndex ParseFaceIndex(const char *&token, ObjIndex &vi)
{
    vi = { 0, 0, 0 };

    vi.v = FixIndex(atoi(token));
    token += strcspn(token, "/ \t\n");
    if (token[0] != '/')
        return vi;

    token++;

    // i//k
    if (token[0] == '/')
    {
        token++;
        vi.vn = FixIndex(atoi(token));
        token += strcspn(token, "/ \t\n");
        return vi;
    }

    // i/j/k or i/j
    vi.vt = FixIndex(atoi(token));
    token += strcspn(token, "/ \t\n");
    if (token[0] != '/')
        return vi;

    // i/j/k
    token++;
    vi.vn = FixIndex(atoi(token));
    token += strcspn(token, "/ \t\n");

    return vi;
}

uint16_t UpdateVertex(std::map<ObjIndex, uint16_t>& vertexCache, ObjSurface& surface, const std::vector<glm::vec3>& positions, const std::vector<glm::vec3>& normals, const std::vector<glm::vec2>& texCoords, const ObjIndex& index)
{
    // check vertex cache first
    auto it = vertexCache.find(index);
    if (it != vertexCache.end())
        return it->second;

    // create vertices not found in cache and insert them
    ObjVertex vertex;
    vertex.position = positions[index.v];

    if (index.vn)
        vertex.normal = normals[index.vn];

    if (index.vt)
        vertex.texCoord = texCoords[index.vt];

    surface.vertices.push_back(vertex);
    uint16_t idx = static_cast<uint16_t>(surface.vertices.size() - 1);
    vertexCache[index] = idx;

    return idx;
}

bool ExportFaceGroupToSurface(ObjSurface& surface, const ObjFaceGroup& faceGroup, const std::vector<glm::vec3>& positions, const std::vector<glm::vec3>& normals, const std::vector<glm::vec2>& texCoords, const std::string name)
{
    std::map<ObjIndex, uint16_t> vertexCache;

    if (faceGroup.empty())
        return false;

    // Flatten vertices and indices
    for (size_t i = 0; i < faceGroup.size(); i++)
    {
        const std::vector<ObjIndex> &face = faceGroup[i];

        ObjIndex i0 = face[0];
        ObjIndex i1 = {};
        ObjIndex i2 = face[1];

        size_t npolys = face.size();

        // Polygon -> triangle fan conversion
        for (size_t k = 2; k < npolys; k++)
        {
            i1 = i2;
            i2 = face[k];

            surface.indices.push_back(UpdateVertex(vertexCache, surface, positions, normals, texCoords, i0));
            surface.indices.push_back(UpdateVertex(vertexCache, surface, positions, normals, texCoords, i1));
            surface.indices.push_back(UpdateVertex(vertexCache, surface, positions, normals, texCoords, i2));
        }
    }

    surface.name = name;
    return true;
}

ObjModel *OBJ_Load(const char *filename)
{
    core::FileReader file(filename);

    ObjModel *model = new ObjModel();

    std::vector<glm::vec3> v;
    std::vector<glm::vec3> vn;
    std::vector<glm::vec2> vt;
    ObjFaceGroup faceGroup;
    std::string name;

    ObjFace face;
    ObjIndex vertexIndex;

    while (file.Peek() != -1)
    {
        size_t lineLength = 0;
        const char *linebuf = file.GetLine(&lineLength);

        // skip comments and empty lines
        if (linebuf[0] == '#' || linebuf[0] == '\n' || lineLength <= 1)
            continue;

        if (ReadToken(linebuf, "v "))
        {
            // vertex
            v.emplace_back(ReadVec3(linebuf));
        } else if (ReadToken(linebuf, "vn "))
        {
            // vertex normal
            vn.emplace_back(ReadVec3(linebuf));
        } else if (ReadToken(linebuf, "vt "))
        {
            // texcoord
            vt.emplace_back(ReadVec2(linebuf));
        } else if (ReadToken(linebuf, "f "))
        {
            // face
            face.clear();
            while ((linebuf[0] != '\n') && (linebuf[0] != '\0'))
            {
                ParseFaceIndex(linebuf, vertexIndex);
                face.push_back(vertexIndex);

                size_t n = strspn(linebuf, " \t\r");
                linebuf += n;
            }

            faceGroup.push_back(face);
        } else if (ReadToken(linebuf, "g "))
        {
            // facegroup name
            ObjSurface surf;
            if (ExportFaceGroupToSurface(surf, faceGroup, v, vn, vt, name))
                model->surfaces.push_back(surf);

            // reset facegroup and get the name of the next one
            faceGroup = ObjFaceGroup();
            name = ReadString(linebuf);
        } else if (ReadToken(linebuf, "mtllib "))
        {
            std::string materialFilename = core::GetBasePath(filename) + ReadString(linebuf);
            DEBUG_LOG_TEXT("MATERIAL: %s", materialFilename.c_str());
        }
    }

    ObjSurface surf;
    if (ExportFaceGroupToSurface(surf, faceGroup, v, vn, vt, name))
        model->surfaces.push_back(surf);

    return model;
}

void OBJ_Free(ObjModel *model)
{
    delete model;
}

} // priv
} // engine