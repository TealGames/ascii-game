#version 430 core

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

layout(location=0) in vec3 aPosition;
layout(location=1) in vec2 aTexCoords;
layout(location=2) in vec3 aNormal;

//These are instanced per object
layout(location=3) in uint aMaterialIndex;
layout(location=4) in uint aMeshIndex;
layout(location=5) in mat4 aModelMatrix;
//NOTE: 5-8 for modelMatrix, 9-12 for inverseMOdelMatrix
layout(location=13) in mat3 aNormalModelMatrix;

//Passed to fragment shader
out vec2 vTexCoords;
flat out uint vMaterialIndex;
out vec3 vWorldPos;
out vec3 vNormal;

void main()
{
    vec4 worldPos=  aModelMatrix * vec4(aPosition, 1.0);
    gl_Position= uViewerBlock.projectionMatrix * uViewerBlock.viewMatrix * worldPos;

    vTexCoords= aTexCoords;
    vWorldPos= worldPos.xyz;
    vMaterialIndex= aMaterialIndex;
    //Just like we use model matrix by pos -> world pos,
    //we do normals by normal model matrix -> world normals
    vNormal= normalize(aNormalModelMatrix * aNormal);
};