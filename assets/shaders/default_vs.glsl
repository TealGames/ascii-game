#version 330 core

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
//Note: location 1 reserved for tex coords
//Note: location 2 reserved for normal

//These instanced per object
layout(location=3) in vec4 aColor;
layout(location=4) in mat4 aModelMatrix;

//Passed to fragment shader
out vec4 vColor;

void main()
{
    gl_Position= uViewerBlock.projectionMatrix * uViewerBlock.viewMatrix * aModelMatrix * vec4(aPosition, 1.0);
    vColor= aColor;
};