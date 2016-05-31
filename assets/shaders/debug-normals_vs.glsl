#version 330 core

in vec3 a_position;
in vec3 a_normal;

out VertexInfo
{
	vec3 normal;
} outVertex;

void main()
{
	gl_Position = vec4(a_position, 1.0);
	outVertex.normal = a_normal;
}