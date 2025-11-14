#version 430 core

in vec3 vWorldDir;
uniform sampler2D uSkybox;
layout(location=0) out vec4 fragColor;

const vec2 invArctan = vec2(0.1591, 0.3183);

vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invArctan;
    uv += 0.5;
    return uv;
}

void main()
{
    vec2 uv = SampleSphericalMap(normalize(vWorldDir));
    vec3 color = texture(uSkybox, uv).rgb;
    fragColor = vec4(color, 1.0);
}