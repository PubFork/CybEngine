#version 420 core

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;

layout(location = 1) in vec3 position;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 texCoord0;

out block
{
    vec3 normal;
    vec2 texCoord0;
} Out;

void main()
{
    Out.normal = vec3(modelViewMatrix * vec4(normal, 0));
    Out.texCoord0 = texCoord0;
    gl_Position = projectionMatrix * (modelViewMatrix * vec4(position, 1));
}