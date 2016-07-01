#include "Precompiled.h"
#include "Renderer/Model_obj.h"
#include "Base/Debug.h"
#include "Base/File.h"
#include "Base/MurmurHash.h"

#define LINEBUFFER_SIZE         1024
#define DEFAULT_MODEL_NAME      "<unknown>"
#define DEFAULT_FACEGROUP_NAME  "Default"
#define DEFAULT_MATERIAL_NAME   "_Default"

/*
 * Some parts are based of syoyo's tinyobjloader:
 * https://github.com/syoyo/tinyobjloader
 */

namespace renderer
{

struct OBJ_EdgeHasher
{
    size_t operator()(const OBJ_Edge &edge) const
    {
        return CalculateMurmurHash(&edge, sizeof(edge));
    }
};

bool operator==(const OBJ_Edge &rhs, const OBJ_Edge &lhs)
{
    return (rhs.vertexIndex == lhs.vertexIndex && rhs.texCoordIndex == lhs.texCoordIndex);
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

Vec3f ReadVec3(const char *&buffer)
{
    const glm::vec2 xy(ReadVec2(buffer));
    const float z = ReadFloat(buffer);
    return Vec3f(xy.x, xy.y, z);
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

OBJ_Edge ReadFaceIndex(const char *&token)
{
    const OBJ_Index vertexIndex = (OBJ_Index)atoi(token);
    assert(vertexIndex != 0);
    token += strcspn(token, "/ \t\r\n");
    if (token[0] != '/')
    {
        return OBJ_Edge(vertexIndex, 0);
    }

    // i//k
    token++;
    if (token[0] == '/')
    {
        token++;
        //edge.normalIndex = atoi(token);       // ignore normal index
        token += strcspn(token, "/ \t\r\n");
        return OBJ_Edge(vertexIndex, 0);
    }

    // i/j/k or i/j
    const OBJ_Index texCoordIndex = (OBJ_Index)atoi(token);
    assert(texCoordIndex != 0);
    token += strcspn(token, "/ \t\r\n");
    if (token[0] != '/')
    {
        return OBJ_Edge(vertexIndex, texCoordIndex);
    }

    // i/j/k
    token++;
    //edge.normalIndex = atoi(token);       // ignore normal index
    token += strcspn(token, "/ \t\r\n");

    return OBJ_Edge(vertexIndex, texCoordIndex);
}

OBJ_Face ReadFace(const char *&buffer)
{
    OBJ_Face face;

    while (buffer[0] != '\r' && buffer[0] != '\n' && buffer[0] != '\0')
    {
        face.edges.emplace_back(ReadFaceIndex(buffer));
        buffer += strspn(buffer, " \t");
    }

    return face;
}

OBJ_Material CreateDefaultMaterial(const std::string &name)
{
    OBJ_Material material;
    material.name = name;
    material.ambientColor = Vec3f(0.0f, 0.0f, 0.0f);
    material.diffuseColor = Vec3f(1.0f, 1.0f, 1.0f);
    material.specularColor = Vec3f(0.0f, 0.0f, 0.0f);
    material.ambientTexture.clear();
    material.diffuseTexture.clear();
    material.specularTexture.clear();
    material.dissolve = 1.0f;
    material.shininess = 1.0f;
    return material;
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

    return (count == 1);
}

bool MTL_Load(const char *filename, OBJ_MaterialMap &outMaterials)
{
    SysFile mtlFile(filename, FileOpen_Read);
    if (!mtlFile.IsValid())
        return false;

    DebugPrintf("Loading %s...\n", filename);
    OBJ_Material material = CreateDefaultMaterial(DEFAULT_MATERIAL_NAME);

    char buffer[LINEBUFFER_SIZE];
    while (ReadLine(&mtlFile, buffer, LINEBUFFER_SIZE))
    {
        const char *lineBuffer = buffer;
        lineBuffer += strspn(lineBuffer, " \t");
        if (lineBuffer[0] == '#' || lineBuffer[0] == '\r' || lineBuffer[0] == '\n' || lineBuffer[0] == '\0')
        {
            continue;
        }

        if (ReadToken(lineBuffer, "newmtl "))
        {
            if (!material.name.empty())
            {
                outMaterials[material.name] = material;
            }

            std::string materialName = ReadString(lineBuffer);
            material = CreateDefaultMaterial(materialName);
        } 
        else if (ReadToken(lineBuffer, "Ka "))
        {
            material.ambientColor = ReadVec3(lineBuffer);
        }
        else if (ReadToken(lineBuffer, "Kd "))
        {
            material.diffuseColor = ReadVec3(lineBuffer);
        }
        else if (ReadToken(lineBuffer, "Ks "))
        {
            material.specularColor = ReadVec3(lineBuffer);
        }
        else if (ReadToken(lineBuffer, "map_Ka "))
        {
            material.ambientTexture = std::string(mtlFile.GetFileBaseDir()) + ReadString(lineBuffer);
        }
        else if (ReadToken(lineBuffer, "map_Kd "))
        {
            material.diffuseTexture = std::string(mtlFile.GetFileBaseDir()) + ReadString(lineBuffer);
        }
        else if (ReadToken(lineBuffer, "map_Ks "))
        {
            material.specularTexture = std::string(mtlFile.GetFileBaseDir()) + ReadString(lineBuffer);
        }
        else if (ReadToken(lineBuffer, "map_bump "))    // TODO: or "bump "
        {
            material.bumpTexture = std::string(mtlFile.GetFileBaseDir()) + ReadString(lineBuffer);
        }
        else if (ReadToken(lineBuffer, "d "))
        {
            material.dissolve = ReadFloat(lineBuffer);
        }
        else if (ReadToken(lineBuffer, "Tr "))
        {
            material.dissolve = 1.0f - ReadFloat(lineBuffer);
        }
        else if (ReadToken(lineBuffer, "Ns "))
        {
            material.shininess = ReadFloat(lineBuffer);
        }
    }

    CondititionalDebugPrintf(outMaterials.empty(), "Warning: Parsed material %s without finding any materials (falling back to default)\n", filename);
    outMaterials[material.name] = material;
    return true;
}

void CalculateNormalsAndTangents(std::shared_ptr<OBJ_RawModel> rawModel)
{
    for (auto &faceGroup : rawModel->faceGroups)
    {
        for (auto &face : faceGroup.faces)
        {
            assert(face.edges.size() >= 3);
            const OBJ_PosNormalTangentVertex &a = rawModel->vertices[face.edges[0].vertexIndex - 1];
            const OBJ_PosNormalTangentVertex &b = rawModel->vertices[face.edges[1].vertexIndex - 1];
            const OBJ_PosNormalTangentVertex &c = rawModel->vertices[face.edges[2].vertexIndex - 1];
            const Vec3f v1 = b.pos - a.pos;
            const Vec3f v2 = c.pos - a.pos;
            face.normal = CrossProduct(v1, v2);

            glm::vec2 st1 = glm::vec2(0, 1) - glm::vec2(0, 0);
            glm::vec2 st2 = glm::vec2(1, 1) - glm::vec2(0, 0);
            bool faceHasTexCoords = (face.edges[0].texCoordIndex && face.edges[1].texCoordIndex && face.edges[2].texCoordIndex);
            if (faceHasTexCoords)
            {
                // align tangent with texCoords
                const glm::vec2 &ta = rawModel->texCoords[face.edges[0].texCoordIndex - 1];
                const glm::vec2 &tb = rawModel->texCoords[face.edges[1].texCoordIndex - 1];
                const glm::vec2 &tc = rawModel->texCoords[face.edges[2].texCoordIndex - 1];
                st1 = tb - ta;
                st2 = tc - ta;           
            }
  
            const float tangentCoef = 1.0f / (st1.s * st2.t - st2.s * st1.t);
            const Vec3f tangent = (v1 * st2.y - v2 * st1.y) * tangentCoef;

            for (const auto &edge : face.edges)
            {
                rawModel->vertices[edge.vertexIndex - 1].normal += face.normal;
                rawModel->vertices[edge.vertexIndex - 1].tangent += tangent;
            }
        }
    }

    for (auto &vertex : rawModel->vertices)
    {
        vertex.normal = Normalize(vertex.normal);
        vertex.tangent = Normalize(vertex.tangent);
    }
}

// TODO: Clean up material handling code
std::shared_ptr<OBJ_RawModel> OBJ_LoadModel(const std::string &filename)
{
    SysFile objFile(filename, FileOpen_Read);
    RETURN_NULL_IF(!objFile.IsValid());

    DebugPrintf("Loading %s...\n", filename.c_str());
    auto rawModel = std::make_shared<OBJ_RawModel>(DEFAULT_MODEL_NAME);
    OBJ_FaceGroup *rawFaceGroup = rawModel->AddEmptyFaceGroup(DEFAULT_FACEGROUP_NAME);
    std::string mtllibPath("");

    char buffer[LINEBUFFER_SIZE] = {};
    while (ReadLine(&objFile, buffer, LINEBUFFER_SIZE))
    {
        const char *linebuf = buffer;

        // skip comments and empty lines
        linebuf += strspn(linebuf, " \t");
        if (linebuf[0] == '#' || linebuf[0] == '\r' || linebuf[0] == '\n' || linebuf[0] == '\0')
        {
            continue;
        }

        // parse all tokens
        if (ReadToken(linebuf, "v "))
        {
            const Vec3f pos(ReadVec3(linebuf));

            // add vertex with pos and a zero normal and tangent
            rawModel->vertices.push_back(OBJ_PosNormalTangentVertex(pos, Vec3f(0.0f, 0.0f, 0.0f), Vec3f(0.0f, 0.0f, 0.0f)));
        } 
        else if (ReadToken(linebuf, "vn "))
        {
            // these are manually calculated after file is loaded
            //const glm::vec3 norm(ReadVec3(linebuf));
            //rawModel->normals.push_back(norm);
        } 
        else if (ReadToken(linebuf, "vt "))
        {
            const glm::vec2 uv(ReadVec2(linebuf));
            rawModel->texCoords.push_back(uv);
        }
        else if (ReadToken(linebuf, "f "))
        {
            const OBJ_Face face = ReadFace(linebuf);  
            rawFaceGroup->faces.push_back(face);
        } 
        else if (ReadToken(linebuf, "o "))
        {
            const std::string modelName = ReadString(linebuf);
            rawModel->name = modelName;
        } 
        else if (ReadToken(linebuf, "g "))
        {
            std::string faceGroupName(ReadString(linebuf));
            rawFaceGroup = rawModel->AddEmptyFaceGroup(faceGroupName);
        } 
        else if (ReadToken(linebuf, "mtllib "))
        {
            const std::string mtllibString = ReadString(linebuf);
            mtllibPath = std::string(objFile.GetFileBaseDir()) + mtllibString;
        } 
        else if (ReadToken(linebuf, "usemtl "))
        {
            rawFaceGroup->materialName = ReadString(linebuf);
        }
    }

    // parse material library is one is specified
    const bool succeededToMTLLoad = MTL_Load(mtllibPath.c_str(), rawModel->materials);
    if (!succeededToMTLLoad)
    {
        DebugPrintf("Failed to read MTL file %s (Using default material)\n", mtllibPath.c_str());
        rawModel->materials[DEFAULT_MATERIAL_NAME] = CreateDefaultMaterial(DEFAULT_MATERIAL_NAME);
    }

    CalculateNormalsAndTangents(rawModel);
    return rawModel;
}

bool operator<(const OBJ_Edge &a, const OBJ_Edge &b)
{
    if (a.vertexIndex != b.vertexIndex)
    {
        return (a.vertexIndex < b.vertexIndex);
    }

    if (a.texCoordIndex != b.texCoordIndex)
    {
        return (a.texCoordIndex < b.texCoordIndex);
    }

    return false;
}

std::shared_ptr<OBJ_CompiledModel> OBJ_CompileRawModel(const std::shared_ptr<OBJ_RawModel> rawModel)
{
    auto compiledModel = std::make_shared<OBJ_CompiledModel>(rawModel->name);

    for (const auto &faceGroup : rawModel->faceGroups)
    {
        std::unordered_map<OBJ_Edge, OBJ_Index, OBJ_EdgeHasher> vertexCache;

        // get the material name
        const auto materialSearch = rawModel->materials.find(faceGroup.materialName);
        const std::string materialName = (materialSearch != rawModel->materials.end()) ? faceGroup.materialName : DEFAULT_MATERIAL_NAME;
        
        // create a new tri surface for the facegroup
        OBJ_TriSurface triSurface(faceGroup.name, rawModel->materials[materialName]);

        for (const auto &face : faceGroup.faces)
        {
            OBJ_Edge triangleEdges[3] = 
            {
                face.edges[0],
                {0, 0},
                face.edges[1]
            };

            // Polygon -> triangle fan conversion
            const size_t numEdges = face.edges.size();
            for (size_t k = 2; k < numEdges; k++)
            {
                triangleEdges[1] = triangleEdges[2];
                triangleEdges[2] = face.edges[k];

                for (const auto &edge : triangleEdges)
                {
                    // check vertex cache first
                    const auto it = vertexCache.find(edge);
                    if (it != vertexCache.end())
                    {
                        triSurface.indices.push_back(it->second);
                    }
                    else
                    {
                        // create vertices not found in cache and insert them
                        const uint16_t idx = static_cast<uint16_t>(triSurface.vertices.size());
                        vertexCache[edge] = idx;

                        const OBJ_PosNormalTangentVertex &vertex = rawModel->vertices[edge.vertexIndex - 1];
                        triSurface.vertices.emplace_back(
                            vertex.pos,
                            vertex.normal,
                            vertex.tangent,
                            edge.texCoordIndex ? rawModel->texCoords[edge.texCoordIndex - 1] : glm::vec2(0.0f));
                        triSurface.indices.push_back(idx);
                    }
                }
            }
        }

        compiledModel->surfaces.push_back(triSurface);
    }

    return compiledModel;
}

} // engine