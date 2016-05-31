#version 420 core
out vec4 fragColor;

in VertexInfo
{
	vec3 position;
	vec3 normal;
	vec2 texCoord0;
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
	const vec3 L = normalize(lightPos - inVertex.position);
	const vec3 N = normalize(inVertex.normal);
	const vec3 V = normalize(u_viewPos - inVertex.position);
	const vec3 H = normalize(L + V);
	
	const float NdotL = max(dot(N, L), 0.0);

	float specular = 0.0;
	if (NdotL > 0)
	{
		const float NdotH = max(dot(N, H), 0.0);
		specular = pow(NdotH, Ns);
	}

	const vec3 finalLight = Ka*La + Kd*NdotL*Ld + Ks*specular*Ls;
	fragColor = vec4(finalLight * texture(tex0, inVertex.texCoord0).rgb, 1.0);
}