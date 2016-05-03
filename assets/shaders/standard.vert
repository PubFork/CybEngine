#version 420 core

uniform mat4 ProjMatrix;
uniform mat4 ModelViewMatrix;

in vec3 Position;
in vec3 Normal;
in vec2 TexCoord0;

out block
{
    vec3 normal;
    vec2 texCoord0;
} Out;

void main()
{
    Out.normal = vec3(ModelViewMatrix * vec4(Normal, 0));
    Out.texCoord0 = TexCoord0;
    gl_Position = ProjMatrix * (ModelViewMatrix * vec4(Position, 1));
}