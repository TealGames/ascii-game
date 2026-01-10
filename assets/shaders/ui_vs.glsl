#version 430 core

layout(location=0) in vec2 aLocalRectPos;
layout(location=1) in vec2 aUV;

//These are instanced per object
layout(location=2) in vec4 aColor;
layout(location=3) in int aTextureIndex;
layout(location=4) in float aDepth;
layout(location=5) in mat3 aWorldRectPosMatrix;

//Passed to fragment shader
out vec4 vColor;
flat out int vTexIndex;
out vec2 vTexCoords;

void main()
{
    vColor = aColor;
	vTexIndex = aTextureIndex;
    vTexCoords = aUV;

    vec3 uiPos = vec3((aWorldRectPosMatrix * vec3(aLocalRectPos, 1)).xy, aDepth);
    //OpenGL NDC range for xyz is -1 to 1 while aLocalRecctPos and aDepth should be in [0,1]
    gl_Position = vec4(uiPos * 2.0 - 1.0, 1);
}