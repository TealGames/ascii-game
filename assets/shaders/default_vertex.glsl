#version 330 core

layout(location=0) in vec3 aPosition;
layout(location=1) in vec4 aColor;
//This is instanced per object not per vertex
layout(location=2) in mat4 aModelMatrix;

//Passed to fragment shader
out vec4 vColor;

uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

void main()
{
    gl_Position= uProjectionMatrix * uViewMatrix * aModelMatrix * vec4(aPosition, 1.0);
    vColor= aColor;
};