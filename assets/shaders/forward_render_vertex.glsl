#version 330 core

layout(std140) uniform CameraBlock 
{
    mat4 viewMatrix;
    mat4 projectionMatrix;
    vec3 cameraPos;
} uCameraBlock;

layout(location=0) in vec3 aPosition;
layout(location=1) in vec2 aTexCoords;
layout(location=2) in vec3 aNormal;

//These are instanced per object
layout(location=3) in vec4 aColor;
layout(location=4) in mat4 aModelMatrix;
layout(location=8) in mat3 aNormalModelMatrix;

//Passed to fragment shader
out vec2 vTexCoords;
out vec4 vColor;
out vec3 vWorldPos;
out vec3 vNormal;

void main()
{
    vec4 worldPos=  aModelMatrix * vec4(aPosition, 1.0);
    gl_Position= uCameraBlock.projectionMatrix * uCameraBlock.viewMatrix * worldPos;

    vTexCoords= aTexCoords;
    vWorldPos= worldPos.xyz;
    vColor= aColor;
    //Just like we use model matrix by pos -> world pos,
    //we do normals by normal model matrix -> world normals
    vNormal= normalize(aNormalModelMatrix * aNormal);
};