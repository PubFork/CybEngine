#version 330 core

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
	mat3 TBN;
} outVertex;

uniform mat4 u_projMatrix;
uniform mat4 u_modelViewMatrix;
uniform vec3 u_viewPos;

void main()
{
	const vec3 lightPos = vec3(0.0, 100.0, 0.0);

	gl_Position = u_projMatrix * u_modelViewMatrix * vec4(a_position, 1.0);
	outVertex.position = a_position;
	outVertex.normal = a_normal;
	outVertex.tangent = a_tangent;
	outVertex.texCoord = a_texCoord0;

	// calculate tangent space transformation matrix
	vec3 N = normalize(a_normal);
	vec3 T = normalize(a_tangent);
	vec3 B = cross(N, T);
	outVertex.TBN = mat3(T, B, N);
}