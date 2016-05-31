#version 330 core

uniform mat4 u_projMatrix;
uniform mat4 u_modelViewMatrix;
uniform float u_debugNormalLength = 0.35;

layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in VertexInfo
{
	vec3 normal;
} inVertex[];

void GenerateLine(int index)
{
	vec3 P = gl_in[index].gl_Position.xyz;
	vec3 N = normalize(inVertex[index].normal);

    gl_Position = u_projMatrix * u_modelViewMatrix * vec4(P, 1.0);
    EmitVertex();
    gl_Position = u_projMatrix * u_modelViewMatrix * vec4(P + N * u_debugNormalLength, 1.0);
    EmitVertex();
    EndPrimitive();
}

void main()
{
    GenerateLine(0); // First vertex normal
    GenerateLine(1); // Second vertex normal
    GenerateLine(2); // Third vertex normal
}  