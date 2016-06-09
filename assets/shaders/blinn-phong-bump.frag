#version 330 core
out vec4 fragColor;

in VertexInfo
{
	vec3 position;
	vec3 normal;
	vec3 tangent;
	vec2 texCoord;
} inVertex;

uniform sampler2D diffuseTexture;
uniform bool useSpecularTexture = false;
uniform sampler2D specularTexture;
uniform bool useNormalTexture = false;
uniform sampler2D normalTexture;
uniform vec3 u_viewPos;

uniform vec3 Ka = vec3(1.0);
uniform vec3 Kd = vec3(1.0);
uniform vec3 Ks = vec3(1.0);
uniform float Ns = 80;

uniform vec3 La = vec3(0.6);
uniform vec3 Ld = vec3(1.0);
uniform vec3 Ls = vec3(0.8);

void main()
{
	const vec3 lightPos = vec3(.0, 100.0, .0);

	// Tangent view space
	vec3 tanN = normalize(inVertex.normal);
	vec3 tanT = normalize(inVertex.tangent);
	vec3 tanB = cross(tanN, tanT);

	// Lighting vectors
	vec3 Ltmp = normalize(lightPos - inVertex.position);
	vec3 Vtmp = normalize(u_viewPos - inVertex.position);
	vec3 Htmp = normalize(Ltmp + Vtmp);

	// Transform vectors to tangent space
	vec3 L = normalize(vec3(dot(Ltmp, tanT), dot(Ltmp, tanB), dot(Ltmp, tanN)));
	vec3 V = normalize(vec3(dot(Vtmp, tanT), dot(Vtmp, tanB), dot(Vtmp, tanN)));
	vec3 H = normalize(vec3(dot(Htmp, tanT), dot(Htmp, tanB), dot(Htmp, tanN)));

	vec3 N = vec3(0, 0, 1);
	if (useNormalTexture)
	{
		N = normalize((2.0f * texture(normalTexture, inVertex.texCoord).rgb) - 1.0f);
	}

	float NdotL = max(dot(N, L), 0.0);

	vec3 finalKs = Ks;
	if (useSpecularTexture)
	{
		finalKs = texture(specularTexture, inVertex.texCoord).rgb;
	}

	float specular = 0.0;
	if (NdotL > 0)
	{
		float NdotH = max(dot(N, H), 0.0);
		specular = pow(NdotH, Ns);
	}

	vec3 finalLight = Ka*La + Kd*NdotL*Ld + finalKs*specular*Ls;
	fragColor = vec4(finalLight * texture(diffuseTexture, inVertex.texCoord).rgb, 1.0);
}