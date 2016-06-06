#version 330 core
out vec4 fragColor;

in VertexInfo
{
	vec3 position;
	vec3 normal;
	vec2 texCoord;
} inVertex;

uniform sampler2D tex0;
uniform vec3 u_viewPos;

uniform vec3 Ka = vec3(1.0);
uniform vec3 Kd = vec3(1.0);
uniform vec3 Ks = vec3(1.0);
uniform float Ns = 80;

uniform vec3 La = vec3(0.7);
uniform vec3 Ld = vec3(1.0);
uniform vec3 Ls = vec3(0.8);

void main()
{
	const vec3 lightPos = vec3(0.0, 10.0, 0.0);
	vec3 L = normalize(lightPos - inVertex.position);
	vec3 N = normalize(inVertex.normal);
	vec3 V = normalize(u_viewPos - inVertex.position);
	vec3 H = normalize(L + V);
	
	float NdotL = max(dot(N, L), 0.0);

	float specular = 0.0;
	if (NdotL > 0)
	{
		float NdotH = max(dot(N, H), 0.0);
		specular = pow(NdotH, Ns);
	}

	vec3 finalLight = Ka*La + Kd*NdotL*Ld + Ks*specular*Ls;
	fragColor = vec4(finalLight * texture(tex0, inVertex.texCoord).rgb, 1.0);
}