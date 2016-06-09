#version 330 core

in vec3 a_position;
in vec3 a_normal;
in vec3 a_tangent;

out VertexInfo
{
	vec3 normal;
	vec3 tangent;
} outVertex;

void main()
{
	gl_Position = vec4(a_position, 1.0);
	outVertex.normal = a_normal;
	outVertex.tangent = a_tangent;
}