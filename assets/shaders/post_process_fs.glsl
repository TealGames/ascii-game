#version 330 core

layout(location=0) out vec4 fragColor;

uniform bool uDoBloom;
uniform sampler2D uBrightnessTexture;

uniform sampler2D uHdrTexture;
uniform ivec2 uScreenSize;
uniform float uExposure;

//More cinematic tone mapping that preserves mid-tones better than reinhard
vec3 AcesFilmicToneMapping(vec3 color)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((color*(a*color+b))/(color*(c*color+d)+e), 0.0, 1.0);
}

//Reinhard tone mapping + increased exposure to convert hdr color to 8 bit range
vec3 ReinhardToneMapping(vec3 color)
{
    float exposure = 1.5;
    return 1.0 - exp(-color * exposure);
}

void main()
{
    vec2 uv= gl_FragCoord.xy / uScreenSize;
    vec3 ColHDR4 = texture(uHdrTexture, uv).rgb * uExposure;
    vec3 bloomColor = uDoBloom? texture(uBrightnessTexture, uv).rgb : vec3(0, 0, 0);
    //fragColor= vec4(ColHDR4, 1);

    fragColor = vec4(AcesFilmicToneMapping(ColHDR4) + bloomColor, 1);
    
    //fragColor= vec4(0.5, 1, 1, 1);

    // Simple Reinhard tone mapping
    //vec3 mapped = ColHDR4 / (ColHDR4 + vec3(1.0));

    // Exposure tone mapping (optional)
    // vec3 mapped = vec3(1.0) - exp(-ColHDR4 * exposure);

    // Gamma correction (assuming sRGB output)
    //mapped = pow(mapped, vec3(1.0/2.2));

    //fragColor = vec4(mapped, 1.0);
}