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
//These are instanced per object
layout(location=5) in mat4 aModelMatrix;

void main()
{
    gl_Position= uViewerBlock.projectionMatrix * uViewerBlock.viewMatrix * aModelMatrix * vec4(aPosition, 1.0);
};