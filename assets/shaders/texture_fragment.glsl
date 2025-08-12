#version 330 core

uniform sampler2D uTexture;

in vec4 vColor;
in vec2 vTexCoords;

layout(location=0) out vec4 color;

void main()
{
	vec4 texColor= texture(uTexture, vTexCoords);
	color=vec4(mix(texColor.rgb, vColor.rgb, vColor.a), texColor.a);
	//color=texColor;
};