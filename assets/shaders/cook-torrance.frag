#version 420 core

out vec4 fragColor;

in VertexInfo
{
	vec3 position;
	vec3 normal;
	vec2 texCoord0;
} inVertex;

struct LightInfo
{
	vec3 position;
	vec3 ambientIntensity;
	vec3 diffuseIntensity;
	vec3 specularIntensity;
};

LightInfo light;

uniform sampler2D tex0;
uniform vec3 u_viewPos;

uniform vec3 u_matAmbientReflectance;
uniform vec3 u_matDiffuseReflectance;
uniform vec3 u_matSpecularReflectance;
uniform float u_matShininess;

void main()
{
	const float Ka = 0.2;
	const float Kd = 0.6;
	const float Ks = 1.0;

	light.position = vec3(0.0, 20.0, 0.0);
	light.ambientIntensity = vec3(0.0);
	light.diffuseIntensity = vec3(1.0);
	light.specularIntensity = vec3(1);

	const vec3 L = normalize(light.position - inVertex.position);
	const vec3 N = normalize(inVertex.normal);
	const vec3 V = normalize(u_viewPos - inVertex.position);
	const vec3 H = normalize(L + V);

	// set important material values
    float roughnessValue = 0.6; // 0 : smooth, 1: rough
    float F0 = 0.2; // fresnel reflectance at normal incidence
    float k = 0.2; // fraction of diffuse reflection (specular reflection = 1 - k)
    
    // interpolating normals will change the length of the normal, so renormalize the normal.
    vec3 normal = normalize(inVertex.normal);
    
    // do the lighting calculation for each fragment.
    float NdotL = max(dot(N, L), 0.0);
    
    float specular = 0.0;
    if (NdotL > 0.0)
    {
        // calculate intermediary values
        float NdotH = max(dot(N, H), 0.0); 
        float NdotV = max(dot(N, V), 0.0);	// note: this could also be NdotL, which is the same value
        float VdotH = max(dot(V, H), 0.0);
        float mSquared = roughnessValue * roughnessValue;
        
        // geometric attenuation
        float NH2 = 2.0 * NdotH;
        float g1 = (NH2 * NdotV) / VdotH;
        float g2 = (NH2 * NdotL) / VdotH;
        float geoAtt = min(1.0, min(g1, g2));
     
        // roughness (or: microfacet distribution function)
        // beckmann distribution function
        float r1 = 1.0 / ( 4.0 * mSquared * pow(NdotH, 4.0));
        float r2 = (NdotH * NdotH - 1.0) / (mSquared * NdotH * NdotH);
        float roughness = r1 * exp(r2);
        
        // fresnel
        // Schlick approximation
        float fresnel = pow(1.0 - VdotH, 5.0);
        fresnel *= (1.0 - F0);
        fresnel += F0;
        
        specular = (fresnel * geoAtt * roughness) / (NdotV * NdotL * 3.14);
    }
    
	const vec3 color0 = texture(tex0, inVertex.texCoord0).rgb;
	const vec3 light0 = vec3(1.0) * min(Ka + Kd*NdotL + Ks*(k + specular * (1.0 - k)), 1.0);

	fragColor = vec4(color0 * light0, 1.0);
}