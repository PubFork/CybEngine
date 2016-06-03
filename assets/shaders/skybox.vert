#version 330 core

uniform mat4 u_projMatrix;
uniform mat4 skyboxViewMatrix;

in vec3 a_position;

out VertexInfo
{
	vec3 texCoord;
} outVertex;

void main()
{
	vec4 pos = u_projMatrix * skyboxViewMatrix * vec4(a_position, 1.0);
	gl_Position = pos.xyww;
	outVertex.texCoord = a_position;
}