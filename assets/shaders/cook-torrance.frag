#version 420 core

out vec4 fragColor;

in VertexInfo
{
	vec3 position;
	vec3 normal;
	vec3 tangent;
	vec2 texCoord;
} inVertex;

uniform sampler2D diffuseTexture;
uniform vec3 u_viewPos;

uniform sampler2D specularTexture;
uniform bool useSpecularTexture = false;

uniform vec3 Ka = vec3(1.0);
uniform vec3 Kd = vec3(1.0);
uniform vec3 Ks = vec3(1.0);
uniform float Ns = 80;

uniform vec3 La = vec3(0.6);
uniform vec3 Ld = vec3(1.0);
uniform vec3 Ls = vec3(0.8);

// Schlicks fresnel approximation
vec3 Fresnel_Schlick(float VdotH, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - VdotH, 5.0);
}

// Normal Distribution Function (microfacet distrubation)
float Backman_NDF(float roughness2, float NdotH)
{
	float r1 = 1.0 / ( 4.0 * roughness2 * pow(NdotH, 4.0));
	float r2 = (NdotH * NdotH - 1.0) / (roughness2 * NdotH * NdotH);
	return r1 * exp(r2);
}

// Trowbridge-Reitz
float GGX_NDF(float roughness2, float NdotH)
{
	float alpha2 = roughness2*roughness2;
	float denom = pow( ((NdotH)*(NdotH)) * (alpha2-1.0) + 1.0, 2.0 );
 
	return alpha2 / (3.14 * denom);
}
 
float BlinnPhong_NDF(float roughness2, float NdotH)
{
	float alpha2 = roughness2*roughness2;
 
	float first = 1.0 / (3.14 * alpha2);
	float sec = pow(NdotH, (2.0 / (alpha2)-2.0));
 
	return first * sec;
}

float Geo_Implicit(float NdotL, float NdotV)
{
	return NdotL * NdotV;
}

void main()
{
	const vec3 lightPos = vec3(0.0, 100.0, 0.0);

	const vec3 L = normalize(lightPos - inVertex.position);
	const vec3 N = normalize(inVertex.normal);
	const vec3 V = normalize(u_viewPos - inVertex.position);
	const vec3 H = normalize(L + V);

	vec3 finalKs = vec3(1);
	

	// set important material values
    float roughnessValue = 0.2; // 0 : smooth, 1: rough
    float F0 = 1.2; // fresnel reflectance at normal incidence
	roughnessValue = pow(1 - (1 / Ns), 4);
	roughnessValue = roughnessValue * roughnessValue;
	if (useSpecularTexture)
	{
		float v = texture(specularTexture, inVertex.texCoord).g;
		roughnessValue = 1 - pow(v, 4);
		roughnessValue = roughnessValue * roughnessValue;
	}
            
	// calculate intermediary values
	float NdotL = max(dot(N, L), 0.0);
	float NdotH = max(dot(N, H), 0.0); 
	float NdotV = max(dot(N, V), 0.0);
	float VdotH = max(dot(V, H), 0.0);
	float roughness2 = roughnessValue * roughnessValue;

	vec3 F = Fresnel_Schlick(VdotH, vec3(F0));
	float D = GGX_NDF(roughness2, NdotH);
	float G = Geo_Implicit(NdotL, NdotV);
	vec3 specular = ( (F * D * G) / (4.0 * NdotL * NdotV) );
    	
	vec3 finalLight = Ka*La + Kd*NdotL*Ld + specular*Ls;

	fragColor = vec4(finalLight * texture(diffuseTexture, inVertex.texCoord).rgb, 1.0);
}