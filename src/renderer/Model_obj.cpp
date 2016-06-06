#include "Precompiled.h"
#include "Model_obj.h"
#include "Base/Debug.h"
#include "Base/File.h"
#include "Base/MurmurHash.h"

#include "Base/Profiler.h"

#define LINEBUFFER_SIZE         1024
#define DEFAULT_MODEL_NAME      "<unknown>"
#define DEFAULT_FACEGROUP_NAME  "default"
#define DEFAULT_MATERIAL_NAME   "_default"

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
    return rhs.vertexIndex == lhs.vertexIndex &&
        rhs.texCoordIndex == lhs.texCoordIndex;
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
    material.ambientColor = glm::vec3(0.0f, 0.0f, 0.0f);
    material.diffuseColor = glm::vec3(1.0f, 1.0f, 1.0f);
    material.specularColor = glm::vec3(0.0f, 0.0f, 0.0f);
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

    DEBUG_LOG_TEXT("Loading %s...", filename);
    OBJ_Material material;

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

    outMaterials[material.name] = material;
    return true;
}

void CalculateNormals(const std::vector<glm::vec3> &vertexList, std::vector<OBJ_FaceGroup> &faceGroupList, std::vector<glm::vec3> &normalList)
{
    for (auto &faceGroup : faceGroupList)
    {
        for (auto &face : faceGroup.faces)
        {
            assert(face.edges.size() >= 3);
            const glm::vec3 &a = vertexList[face.edges[0].vertexIndex - 1];
            const glm::vec3 &b = vertexList[face.edges[1].vertexIndex - 1];
            const glm::vec3 &c = vertexList[face.edges[2].vertexIndex - 1];

            face.normal = (glm::cross(b - a, c - a));

            for (const auto &edge : face.edges)
            {
                normalList[edge.vertexIndex - 1] += face.normal;
            }
        }
    }

    for (auto &normal : normalList)
    {
        normal = glm::normalize(normal);
    }
}

// TODO: Clean up material handling code
std::shared_ptr<OBJ_RawModel> OBJ_LoadModel(const char *filename)
{
    SysFile objFile(filename, FileOpen_Read);
    if (!objFile.IsValid())
        return nullptr;

    DEBUG_LOG_TEXT("Loading %s...", filename);
    auto rawModel = std::make_shared<OBJ_RawModel>(DEFAULT_MODEL_NAME);
    OBJ_FaceGroup *rawFaceGroup = rawModel->CreateEmptyFaceGroup(DEFAULT_FACEGROUP_NAME);

    std::string mtllibPath("");

    char buffer[LINEBUFFER_SIZE];
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
            const glm::vec3 pos(ReadVec3(linebuf));
            rawModel->vertices.push_back(pos);
            rawModel->normals.push_back(glm::vec3(0.0f, 0.0f, 0.0f));   // add an zero normal for all vertices
        } 
        else if (ReadToken(linebuf, "vn "))
        {
            const glm::vec3 norm(ReadVec3(linebuf));
            rawModel->normals.push_back(norm);
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
            rawFaceGroup = rawModel->CreateEmptyFaceGroup(faceGroupName);
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
    if (!mtllibPath.empty())
    {
        const bool succeededToMTLLoad = MTL_Load(mtllibPath.c_str(), rawModel->materials);
        if (!succeededToMTLLoad || rawModel->materials.empty())
        {
            DEBUG_LOG_TEXT_COND(true, "Failed to read MTL file %s (Using default)", mtllibPath.c_str());
            rawModel->materials[DEFAULT_MATERIAL_NAME] = CreateDefaultMaterial(DEFAULT_MATERIAL_NAME);
        }
    }

    CalculateNormals(rawModel->vertices, rawModel->faceGroups, rawModel->normals);
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

    SCOOPED_PROFILE_EVENT("OBJ COMPILATION");
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
                        triSurface.vertices.emplace_back(
                            rawModel->vertices[edge.vertexIndex - 1],
                            rawModel->normals[edge.vertexIndex - 1],
                            edge.texCoordIndex ? rawModel->texCoords[edge.texCoordIndex - 1] : glm::vec2());
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