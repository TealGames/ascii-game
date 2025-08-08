#version 330 core

layout(location=0) out vec4 color;
in vec4 vColor;

void main()
{
	//EMISSIVE CODE -> APPLIES COLOR OVERLAY
	/*
	vec4 emissiveColor = vec4(1.0, 0.5, 0.0, 1.0); // Orange glow
	float emissiveStrength = 0.5;

	color = vColor + emissiveColor * emissiveStrength;
	*/
	color=vColor;
};
