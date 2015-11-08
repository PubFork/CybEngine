#pragma once

namespace renderer
{

enum ShaderStage
{
    Shader_Vertex,
    Shader_Geometry,
    Shader_Fragment,
    Shader_Compute,
    Shader_Count
};

class Shader
{
public:
    Shader(ShaderStage st, const char *source);
    ~Shader();

    bool Compile(const char *source);
    
    GLuint shaderId;
    ShaderStage stage;
};

class ShaderSet
{
public:
    struct Uniform
    {
        std::string name;
        GLint location;
        uint32_t numFloats;
    };

    ShaderSet();
    ~ShaderSet();

    void SetShader(std::shared_ptr<Shader> s);
    void UnsetShader(ShaderStage stage);
    bool Link();

    bool SetUniform(const char *name, uint32_t numFloats, const float *v);
    bool SetUniform1f(const char *name, float x);
    bool SetUniform2f(const char *name, float x, float y);
    bool SetUniform3f(const char *name, float x, float y, float z);
    bool SetUniform4f(const char *name, float x, float y, float z, float w = 1.0f);
    bool SetUniform4fv(const char *name, const float *v);
    bool SetUniform4x4f(const char *name, const glm::mat4 &m);

    GLuint progId;
    
private:
    std::shared_ptr<Shader> shaders[Shader_Count];
    std::vector<Uniform> uniformInfo;
};

} // renderer