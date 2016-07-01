#version 330 core

out vec4 fragColor;

in VertexInfo
{
	vec3 position;
	vec3 normal;
	vec3 tangent;
	vec2 texCoord;
	mat3 TBN;
} inVertex;

uniform vec3 u_viewPos;
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D normalTexture;
uniform bool useSpecularTexture = false;
uniform bool useNormalTexture = false;

uniform vec3 Ka = vec3(1.0);
uniform vec3 Kd = vec3(1.0);
uniform vec3 Ks = vec3(1.0);
uniform float Ns = 80;

uniform vec3 La = vec3(0.6);
uniform vec3 Ld = vec3(1.0);
uniform vec3 Ls = vec3(0.8);

void main()
{
	const vec3 lightPos = vec3(0.0, 100.0, 0.0);

	// Transform vectors to tangent space
	vec3 LightDirection = normalize(lightPos - inVertex.position);
	vec3 viewDirection = normalize(u_viewPos - inVertex.position);
	vec3 L = normalize(LightDirection) * inVertex.TBN;
	vec3 V = normalize(viewDirection) * inVertex.TBN;
	vec3 H = normalize(LightDirection + viewDirection) * inVertex.TBN;

	vec3 N = vec3(0, 0, 1);
	if (useNormalTexture)
	{
		N = normalize((2.0f * texture(normalTexture, inVertex.texCoord).rgb) - 1.0f);
	}

	vec3 finalKs = vec3(Ks);
	if (useSpecularTexture)
	{
		finalKs = texture(specularTexture, inVertex.texCoord).rgb;
	}

	float NdotL = max(dot(N, L), 0.0);
	float specular = 0.0;
	if (NdotL > 0)
	{
		float NdotH = max(dot(H, N), 0.0);
		specular = pow(NdotH, Ns);
	}

	vec3 finalLight = Ka*La + Kd*NdotL*Ld + finalKs*specular*Ls;
	fragColor = vec4(finalLight * texture(diffuseTexture, inVertex.texCoord).rgb , 1.0);
}