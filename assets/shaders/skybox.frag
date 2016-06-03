#version 330 core
out vec4 fragColor;

in VertexInfo
{
	vec3 texCoord;
} inVertex;

uniform samplerCube skybox;

void main()
{
	// HACK: inv y axis, dunnu why really, but it works...
	vec3 uv = vec3(inVertex.texCoord.x, -inVertex.texCoord.y, inVertex.texCoord.z);	
	fragColor = texture(skybox, uv);
}