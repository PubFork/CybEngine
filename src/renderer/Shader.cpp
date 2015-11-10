#include "stdafx.h"
#include "Shader.h"

#include "core/Log.h"
#include "InputLayout.h"

namespace renderer
{

static const char *attribName[Attrib_Count] = {
    "a_position",
    "a_normal",
    "a_color0",
    "a_color1",
    "a_texCoord0",
    "a_texCoord1",
    "a_texCoord2",
    "a_texCoord3",
};

GLenum GLStage(ShaderStage stage)
{
    switch (stage) {
    case Shader_Vertex:   return GL_VERTEX_SHADER;
    case Shader_Geometry: return GL_GEOMETRY_SHADER;
    case Shader_Fragment: return GL_FRAGMENT_SHADER;
    case Shader_Compute:  return GL_COMPUTE_SHADER;
    default: assert(0); break;
    }

    return GL_NONE;
}

Shader::Shader(ShaderStage st, const char *source) :
    shaderId(0),
    stage(st)
{
    THROW_FATAL_COND(Compile(source) != true, "Failed to compile shader source");
}

Shader::~Shader()
{
    if (shaderId)
        glDeleteShader(shaderId);
}

bool Shader::Compile(const char *source)
{
    if (!shaderId)
        shaderId = glCreateShader(GLStage(stage));

    glShaderSource(shaderId, 1, &source, 0);
    glCompileShader(shaderId);

    GLint compiled = 0;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLchar infoLog[1024];
        glGetShaderInfoLog(shaderId, sizeof(infoLog), 0, infoLog);
        DEBUG_LOG_TEXT_COND(infoLog[0], "Compiling shader:\n%s\nFailed: %s", source, infoLog);
        return false;
    }

    return true;
} 

ShaderSet::ShaderSet() :
    progId(0)
{
    progId = glCreateProgram();
}

ShaderSet::~ShaderSet()
{
    glDeleteProgram(progId);
}

void ShaderSet::SetShader(std::shared_ptr<Shader> s)
{
    shaders[s->stage] = s;
    glAttachShader(progId, s->shaderId);

    if (shaders[Shader_Vertex] && shaders[Shader_Fragment])
        THROW_FATAL_COND(Link() != true, "Failed to link shader program.");
}

void ShaderSet::UnsetShader(ShaderStage stage)
{
    if (shaders[stage]) {
        glDetachShader(progId, shaders[stage]->shaderId);
        shaders[stage] = nullptr;
    }
}

bool ShaderSet::Link()
{
    glLinkProgram(progId);
    GLint linked = 0;
    glGetProgramiv(progId, GL_LINK_STATUS, &linked);
    if (!linked) {
        GLchar infoLog[1024];
        glGetProgramInfoLog(progId, sizeof(infoLog), 0, infoLog);
        DEBUG_LOG_TEXT_COND(infoLog[0], "Linking shaders failed: %s", infoLog);
        return false;
    }

    glUseProgram(progId);

    GLint uniformCount = 0;
    glGetProgramiv(progId, GL_ACTIVE_UNIFORMS, &uniformCount);
    for (GLint i = 0; i < uniformCount; i++) {
        GLint size = 0;
        GLenum type;
        GLchar name[32];
        glGetActiveUniform(progId, i, sizeof(name), 0, &size, &type, name);

        if (size) {
            Uniform u;
            u.name = name;
            u.location = glGetUniformLocation(progId, name);

            switch (type) {
            case GL_FLOAT:      u.numFloats = 1; break;
            case GL_FLOAT_VEC2: u.numFloats = 2; break;
            case GL_FLOAT_VEC3: u.numFloats = 3; break;
            case GL_FLOAT_VEC4: u.numFloats = 4; break;
            //case GL_FLOAT_MAT3: u.numFloats = 12; break;
            case GL_FLOAT_MAT4: u.numFloats = 16; break;
            default: continue;
            }

            uniformInfo.push_back(u);
        }
    }

    for (int i = 0; i < Attrib_Count; i++) 
        attribLocations.push_back(glGetAttribLocation(progId, attribName[i]));

    return true;
}

GLint ShaderSet::GetAttribLocation(uint32_t attrib) const
{
    return attribLocations[attrib];
}

bool ShaderSet::SetUniform(const char *name, uint32_t numFloats, const float *v)
{
    for (const auto &uniform : uniformInfo) {
        if (!strcmp(uniform.name.c_str(), name)) {
            glUseProgram(progId);

            switch (uniform.numFloats) {
            case 1:   glUniform1fv(uniform.location, numFloats, v); break;
            case 2:   glUniform2fv(uniform.location, numFloats / 2, v); break;
            case 3:   glUniform3fv(uniform.location, numFloats / 3, v); break;
            case 4:   glUniform4fv(uniform.location, numFloats / 4, v); break;
            //case 12:  glUniformMatrix3fv(uniform.location, 1, 1, v); break;
            case 16:  glUniformMatrix4fv(uniform.location, 1, GL_FALSE, v); break;
            default: assert(0);
            }

            return true;
        }
    }

    DEBUG_LOG_TEXT("Warning: uniform %s not present in selected shader", name);
    return 0;
}

bool ShaderSet::SetUniform1f(const char *name, float x)
{
    const float v[] = { x };
    return SetUniform(name, 1, v);
}

bool ShaderSet::SetUniform2f(const char *name, float x, float y)
{
    const float v[] = { x, y };
    return SetUniform(name, 2, v);
}

bool ShaderSet::SetUniform3f(const char *name, float x, float y, float z)
{
    const float v[] = { x, y, z };
    return SetUniform(name, 3, v);
}

bool ShaderSet::SetUniform4f(const char *name, float x, float y, float z, float w)
{
    const float v[] = { x, y, z, w };
    return SetUniform(name, 4, v);
}

bool ShaderSet::SetUniform4fv(const char *name, const float *v)
{
    return SetUniform(name, 4, v);
}

bool ShaderSet::SetUniform4x4f(const char *name, const glm::mat4 &m)
{
    return SetUniform(name, 16, glm::value_ptr(m));
}

} // renderer