#version 430 core

uniform sampler2D uSkybox;

in vec3 vViewSpaceDir;
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
    vec2 uv = SampleSphericalMap(normalize(vViewSpaceDir));
    vec3 color = texture(uSkybox, uv).rgb;
    //vec3 color = SampleEquirectangular(vViewSpaceDir);
    fragColor = vec4(color, 1.0);
}