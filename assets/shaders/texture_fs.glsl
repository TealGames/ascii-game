#version 330 core

struct Material
{
    vec4 baseColor;
    vec4 emission;
    float alpha;
    float metallic;
    float roughness;
    int albedoIndex;
};

layout(std430) buffer Materials 
{ 
    Material materials[MATERIAL_MAX_COUNT]; 
};

uniform sampler2D uAlbedo;

flat in uint vMaterialIndex;
in vec2 vTexCoords;

layout(location=0) out vec4 color;

void main()
{
	vec4 texColor= texture(uAlbedo, vTexCoords);
	vec4 baseColor = materials[vMaterialIndex].baseColor;
	color=vec4(mix(texColor.rgb, baseColor.rgb, baseColor.a), texColor.a * materials[vMaterialIndex].alpha);
	//color=texColor;
};