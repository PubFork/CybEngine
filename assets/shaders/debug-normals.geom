#version 330 core

uniform mat4 u_projMatrix;
uniform mat4 u_modelViewMatrix;
uniform float u_debugNormalLength = 4.35;

layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in VertexInfo
{
	vec3 normal;
	vec3 tangent;
} inVertex[];

out FragInfo
{
	vec4 color;
} outFrag;

void GenerateLine(int index)
{
	vec3 P = gl_in[index].gl_Position.xyz;
	vec3 N = normalize(inVertex[index].normal);
	vec3 T = normalize(inVertex[index].tangent);

	// draw normal (blue)
	outFrag.color = vec4(0, 0, 1, 1);
    gl_Position = u_projMatrix * u_modelViewMatrix * vec4(P, 1.0);
    EmitVertex();
	outFrag.color = vec4(0, 0, 1, 1);
    gl_Position = u_projMatrix * u_modelViewMatrix * vec4(P + N * u_debugNormalLength, 1.0);
    EmitVertex();
    EndPrimitive();
	
	// draw tangent (red)
	outFrag.color = vec4(1, 0, 0, 1);
	gl_Position = u_projMatrix * u_modelViewMatrix * vec4(P, 1.0);
    EmitVertex();
	outFrag.color = vec4(1, 0, 0, 1);
    gl_Position = u_projMatrix * u_modelViewMatrix * vec4(P + T * u_debugNormalLength, 1.0);
    EmitVertex();
    EndPrimitive();

	// draw bi-tangent (green)
	vec3 b = cross(N, T);
	outFrag.color = vec4(0, 1, 0, 1);
	gl_Position = u_projMatrix * u_modelViewMatrix * vec4(P, 1.0);
    EmitVertex();
	outFrag.color = vec4(0, 1, 0, 1);
    gl_Position = u_projMatrix * u_modelViewMatrix * vec4(P + b * u_debugNormalLength, 1.0);
    EmitVertex();
    EndPrimitive();
}

void main()
{
    GenerateLine(0);	// First vertex normal
    GenerateLine(1);	// Second vertex normal
    GenerateLine(2);	// Third vertex normal
}  