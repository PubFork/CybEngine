#version 330 core

uniform mat4 u_projMatrix;
uniform mat4 u_modelViewMatrix;

in vec3 a_position;
in vec3 a_normal;
in vec3 a_tangent;
in vec2 a_texCoord0;

out VertexInfo
{
	vec3 position;
	vec3 normal;
	vec3 tangent;
	vec2 texCoord;
} outVertex;

void main()
{
	gl_Position = u_projMatrix * u_modelViewMatrix * vec4(a_position, 1.0);
	outVertex.position = a_position;
	outVertex.normal = a_normal;
	outVertex.tangent = a_tangent;
	outVertex.texCoord = a_texCoord0;
}