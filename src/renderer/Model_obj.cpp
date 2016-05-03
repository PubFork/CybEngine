#include "Precompiled.h"
#include "Model_obj.h"
#include "Base/Debug.h"
#include "Base/File.h"

/*
 * Some parts are based of syoyo's tinyobjloader:
 * https://github.com/syoyo/tinyobjloader
 */

namespace renderer
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

float ReadFloat(const char *&buffer)
{
    buffer += strspn(buffer, " \t");
    const float value = (float)atof(buffer);
    buffer += strcspn(buffer, " \t\r\n");
    return value;
}

glm::vec2 ReadVec2(const char *&buffer)
{
    const float x = ReadFloat(buffer);
    const float y = ReadFloat(buffer);
    return glm::vec2(x, y);
}

glm::vec3 ReadVec3(const char *&buffer)
{
    const glm::vec2 xy(ReadVec2(buffer));
    const float z = ReadFloat(buffer);
    return glm::vec3(xy, z);
}

bool ReadToken(const char *&buffer, const char *token)
{
    const size_t tokenLength = strlen(token);
    if (!strncmp(buffer, token, tokenLength))
    {
        buffer += tokenLength;
        buffer += strspn(buffer, " \t");
        return true;
    }

    return false;
}

// read a string from token, using newline as string terminator
std::string ReadString(const char *&buffer)
{
    const size_t end = strcspn(buffer, "\r\n");
    std::string str(buffer, &buffer[end]);
    buffer += end;
    return str;
}

uint16_t FixIndex(int idx)
{
    return (uint16_t)(idx > 0 ? idx - 1 : 0);
}

ObjIndex ReadFaceIndex(const char *&token)
{
    ObjIndex vi = { 0, 0, 0 };

    vi.v = FixIndex(atoi(token));
    token += strcspn(token, "/ \t\r\n");
    if (token[0] != '/')
        return vi;

    // i//k
    token++;
    if (token[0] == '/')
    {
        token++;
        vi.vn = FixIndex(atoi(token));
        token += strcspn(token, "/ \t\r\n");
        return vi;
    }

    // i/j/k or i/j
    vi.vt = FixIndex(atoi(token));
    token += strcspn(token, "/ \t\r\n");
    if (token[0] != '/')
        return vi;

    // i/j/k
    token++;
    vi.vn = FixIndex(atoi(token));
    token += strcspn(token, "/ \t\r\n");

    return vi;
}

ObjFace ReadFace(const char *&buffer)
{
    ObjFace face;

    while (buffer[0] != '\r' && buffer[0] != '\n' && buffer[0] != '\0')
    {
        face.emplace_back(ReadFaceIndex(buffer));
        buffer += strspn(buffer, " \t");
    }

    return face;
}

uint16_t UpdateVertex(std::map<ObjIndex, uint16_t> &vertexCache, ObjSurface &surface, ObjMaterial * /*material*/, const std::vector<glm::vec3> &positions, const std::vector<glm::vec3> &normals, const std::vector<glm::vec2> &texCoords, const ObjIndex &index)
{
    // check vertex cache first
    auto it = vertexCache.find(index);
    if (it != vertexCache.end())
        return it->second;

    // create vertices not found in cache and insert them
    const uint16_t idx = static_cast<uint16_t>(surface.vertices.size());
    vertexCache[index] = idx;
    surface.vertices.emplace_back(positions[index.v],
                                  index.vn ? normals[index.vn] : glm::vec3(),
                                  index.vt ? texCoords[index.vt] : glm::vec2());
    return idx;
}

bool ExportFaceGroupToSurface(ObjSurface &surface, ObjMaterial *material, const ObjFaceGroup &faceGroup, const std::vector<glm::vec3> &positions, const std::vector<glm::vec3> &normals, const std::vector<glm::vec2> &texCoords, const std::string name)
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

        const size_t npolys = face.size();

        // Polygon -> triangle fan conversion
        for (size_t k = 2; k < npolys; k++)
        {
            i1 = i2;
            i2 = face[k];

            surface.indices.emplace_back(UpdateVertex(vertexCache, surface, material, positions, normals, texCoords, i0));
            surface.indices.emplace_back(UpdateVertex(vertexCache, surface, material, positions, normals, texCoords, i1));
            surface.indices.emplace_back(UpdateVertex(vertexCache, surface, material, positions, normals, texCoords, i2));
        }
    }

    surface.material = material;
    surface.name = name;
    return true;
}

void MakeDefaultMaterial(ObjMaterial &material)
{
    material.name            = "_default";
    material.ambientColor    = glm::vec3(0.0f, 0.0f, 0.0f);
    material.diffuseColor    = glm::vec3(1.0f, 1.0f, 1.0f);
    material.specularColor   = glm::vec3(0.0f, 0.0f, 0.0f);
    material.ambientTexture.clear();
    material.diffuseTexture.clear();
    material.specularTexture.clear();
    material.dissolve        = 1.0f;
    material.shininess       = 1.0f;
}

bool ReadLine(IFile *file, char *buffer, size_t length)
{
    char c = 0;
    size_t count = 0;

    char *buf = buffer;
    do
    {
        count = file->Read((uint8_t *)&c, 1);
        *buf++ = c;
    } while (c != '\n' && c != '\0' && count == 1 && (buf - buffer) < (ptrdiff_t)length);

    return count == 1;
}

bool MTL_Load(const char *filename, ObjMaterialMap &outMaterials)
{
    SysFile mtlFile(filename, FileOpen_Read);
    if (!mtlFile.IsValid())
        return false;

    DEBUG_LOG_TEXT("Loading %s...", filename);
    ObjMaterial material;

    char buffer[2000];
    while (ReadLine(&mtlFile, buffer, 2000))
    {
        const char *lineBuffer = buffer;
        lineBuffer += strspn(lineBuffer, " \t");
        if (lineBuffer[0] == '#' || lineBuffer[0] == '\r'|| lineBuffer[0] == '\n' || lineBuffer[0] == '\0')
            continue;

        if (ReadToken(lineBuffer, "newmtl "))
        {
            if (!material.name.empty())
                outMaterials[material.name] = material;

            MakeDefaultMaterial(material);
            material.name = ReadString(lineBuffer);
        } else if (ReadToken(lineBuffer, "Ka "))
            material.ambientColor = ReadVec3(lineBuffer);
        else if (ReadToken(lineBuffer, "Kd "))
            material.diffuseColor = ReadVec3(lineBuffer);
        else if (ReadToken(lineBuffer, "Ks "))
            material.specularColor = ReadVec3(lineBuffer);
        else if (ReadToken(lineBuffer, "map_Ka "))
            material.ambientTexture = std::string(mtlFile.GetFileBaseDir()) + ReadString(lineBuffer);
        else if (ReadToken(lineBuffer, "map_Kd "))
            material.diffuseTexture = std::string(mtlFile.GetFileBaseDir()) + ReadString(lineBuffer);
        else if (ReadToken(lineBuffer, "map_Ks "))
            material.diffuseTexture = std::string(mtlFile.GetFileBaseDir()) + ReadString(lineBuffer);
        else if (ReadToken(lineBuffer, "d "))
            material.dissolve = ReadFloat(lineBuffer);
        else if (ReadToken(lineBuffer, "Tr "))
            material.dissolve = 1.0f - ReadFloat(lineBuffer);
        else if (ReadToken(lineBuffer, "Ns "))
            material.shininess = ReadFloat(lineBuffer);
    }

    outMaterials[material.name] = material;
    return true;
}

ObjModel *OBJ_Load(const char *filename)
{
    SysFile objFile(filename, FileOpen_Read);
    if (!objFile.IsValid())
        return nullptr;

    DEBUG_LOG_TEXT("Loading %s...", filename);
    ObjModel *model = new ObjModel();

    std::vector<glm::vec3> v;
    std::vector<glm::vec3> vn;
    std::vector<glm::vec2> vt;
    ObjFaceGroup facegroup;
    std::string facegroupName;
    ObjMaterial defaultMaterial;
    ObjMaterial *material = &defaultMaterial;

    MakeDefaultMaterial(defaultMaterial);

    char buffer[2000];
    while (ReadLine(&objFile, buffer, 2000))
    {
        const char *linebuf = buffer;

        // skip comments and empty lines
        linebuf += strspn(linebuf, " \t");
        if (linebuf[0] == '#' || linebuf[0] == '\r' || linebuf[0] == '\n' || linebuf[0] == '\0')
            continue;

        // parse all tokens
        if (ReadToken(linebuf, "v "))
            v.emplace_back(ReadVec3(linebuf));
        else if (ReadToken(linebuf, "vn "))
            vn.emplace_back(ReadVec3(linebuf));
        else if (ReadToken(linebuf, "vt "))
          vt.emplace_back(ReadVec2(linebuf));
        else if (ReadToken(linebuf, "f "))
            facegroup.emplace_back(ReadFace(linebuf));
        else if (ReadToken(linebuf, "o "))
            model->name = ReadString(linebuf);
        else if (ReadToken(linebuf, "g "))
        {
            ObjSurface surf;
            if (material->name == defaultMaterial.name)
            {
                model->materials[defaultMaterial.name] = defaultMaterial;
                material = &model->materials[defaultMaterial.name];
            }

            if (ExportFaceGroupToSurface(surf, material, facegroup, v, vn, vt, facegroupName))
                model->surfaces.emplace_back(surf);

            facegroup = ObjFaceGroup();
            facegroupName = ReadString(linebuf);
        } else if (ReadToken(linebuf, "mtllib "))
        {
            const std::string materialFilename = std::string(objFile.GetFileBaseDir()) + ReadString(linebuf);
            const bool result = MTL_Load(materialFilename.c_str(), model->materials);
            
            if (!result || model->materials.empty())
            {
                DEBUG_LOG_TEXT_COND(true, "Failed to read MTL file %s (Using default)", materialFilename.c_str());
                model->materials[defaultMaterial.name] = defaultMaterial;  // fallback to default material if none is defined
            }

            material = &model->materials.begin()->second;
        } else if (ReadToken(linebuf, "usemtl "))
        {
            const std::string matString = ReadString(linebuf);
            auto searchResult = model->materials.find(matString);
            if (searchResult != model->materials.end())
            {
                material = &searchResult->second;
            } else
            {
                // if the material isn't found, create a default one in the model
                // and point the material pointer to it.
                model->materials[defaultMaterial.name] = defaultMaterial;
                material = &model->materials[defaultMaterial.name];
            }
        }
    }

    ObjSurface surf;
    if (ExportFaceGroupToSurface(surf, material, facegroup, v, vn, vt, facegroupName))
        model->surfaces.emplace_back(surf);

    return model;
}

void OBJ_Free(ObjModel *model)
{
    delete model;
}

} // priv
} // engine