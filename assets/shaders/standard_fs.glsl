#version 420 core

uniform sampler2D tex0;

in VertexData
{
    vec3 normal;
    vec2 texCoord0;
} InVertex;

struct DirectionalLight
{
    vec3 color;
    vec3 direction;
    float ambientIntensity;
};

vec4 CalcDirectionLight(DirectionalLight light)
{
    const float diffuseIntensity = max(0.0, dot(normalize(InVertex.normal), -normalize(light.direction)));
    return vec4(light.color * (light.ambientIntensity + diffuseIntensity), 1.0);
}

out vec4 fragColor;

void main()
{
    DirectionalLight light;
    light.color = vec3(1.0, 1.0, 1.0);
    light.direction = vec3(-1.0, -1.0, -1.0);
    light.ambientIntensity = 0.2;

    fragColor = texture(tex0, InVertex.texCoord0) * CalcDirectionLight(light);
	//fragColor = vec4(1, 1, 0, 0);
}