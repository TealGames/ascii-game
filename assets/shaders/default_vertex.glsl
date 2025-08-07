#version 330 core

layout(location=0) in vec3 aPosition;

//This is instanced per object
layout(location=1) in vec4 aColor;
//This is also instanced
layout(location=2) in mat4 aModelMatrix;

//Passed to fragment shader
out vec4 vColor;

uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

void main()
{
    gl_Position= uProjectionMatrix * uViewMatrix * aModelMatrix * vec4(aPosition, 1.0);
    //gl_Position= vec4(aPosition.xy, 0.5, 1);
    vColor= aColor;
};