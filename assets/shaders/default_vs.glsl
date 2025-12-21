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

layout(std140) uniform ViewerBlock
{
    mat4 viewMatrix;
    mat4 projectionMatrix;
    vec3 worldPos;
    vec3 forwardDir;
    vec3 rightDir;
    vec3 upDir;
    float yFov;
} uViewerBlock;

layout(std430) buffer Materials 
{ 
    Material materials[MATERIAL_MAX_COUNT]; 
};

layout(location=0) in vec3 aPosition;
//NOTE: location 1 reserved for tex coords
//NOTE: location 2 reserved for normal

//These instanced per object
layout(location=3) in uint aMaterialIndex;
//NOTE: location 4 is reserved for mesh index
layout(location=5) in mat4 aModelMatrix;

//Passed to fragment shader
out vec4 vColor;

void main()
{
    gl_Position= uViewerBlock.projectionMatrix * uViewerBlock.viewMatrix * aModelMatrix * vec4(aPosition, 1.0);
    vColor= materials[aMaterialIndex].baseColor;
};