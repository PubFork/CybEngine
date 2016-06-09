#version 330 core
out vec4 color;

in FragInfo
{
	vec4 color;
} inFrag;

void main()
{
    color = inFrag.color;
} 