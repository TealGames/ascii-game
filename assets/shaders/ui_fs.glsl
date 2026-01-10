#version 430 core

in vec4 vColor;
flat in int vTexIndex;
in vec2 vTexCoords;

uniform sampler2D uTextures[TEXTURE_MAX_COUNT];

layout(location=0) out vec4 color;

void main()
{
	//TODO: right now we do not support transparency
	if (vTexIndex >= 0) color = vec4(texture(uTextures[vTexIndex], vTexCoords).rgb * vColor.rgb, 1);
    else color = vec4(vColor.rgb, 1);
}