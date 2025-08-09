#version 330 core

uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

layout(location=0) in vec3 aPosition;
layout(location=1) in vec2 aTexCoords;

//This is instanced per object
layout(location=2) in vec4 aColor;
//This is also instanced
layout(location=3) in mat4 aModelMatrix;

//Passed to fragment shader
out vec4 vColor;
out vec2 vTexCoords;

void main()
{
    gl_Position= uProjectionMatrix * uViewMatrix * aModelMatrix * vec4(aPosition, 1.0);
    //gl_Position= vec4(aPosition.xy, 0.5, 1);
    vColor= aColor;
    vTexCoords= aTexCoords;
};