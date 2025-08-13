#version 330 core

//uniform mat4 uViewMatrix;
//uniform mat4 uProjectionMatrix;

layout(std140) uniform CameraBlock 
{
    mat4 viewMatrix;
    mat4 projectionMatrix;
    vec3 cameraPos;
} uCameraBlock;

layout(location=0) in vec3 aPosition;
layout(location=1) in vec2 aTexCoords;
//Note: 2 is reserved for normals

//These are instanced per object
layout(location=3) in vec4 aColor;
layout(location=4) in mat4 aModelMatrix;

//Passed to fragment shader
out vec4 vColor;
out vec2 vTexCoords;

void main()
{
    gl_Position= uCameraBlock.projectionMatrix * uCameraBlock.viewMatrix * aModelMatrix * vec4(aPosition, 1.0);
    //gl_Position= vec4(aPosition.xy, 0.5, 1);
    vColor= aColor;
    vTexCoords= aTexCoords;
};