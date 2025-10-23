#version 330 core

struct PointLight 
{
    vec3 position;
    vec4 color;
    float radius;
    uint shadowMapIndex;
};

layout(std140) uniform LightsBlock
{
    vec3 directionalDir;
    vec4 directionalColor;
    int pointLightsCount;
    PointLight pointLights[2];
} uLightsBlock;

layout(std140) uniform ViewerBlock 
{
    mat4 viewMatrix;
    mat4 projectionMatrix;
    vec3 worldPos;
} uViewerBlock;

uniform sampler2D uAlbedo;
uniform float uBloomThreshold;
uniform samplerCube uShadowMaps[2];

in vec2 vTexCoords;
in vec4 vColor;
in vec3 vWorldPos;
in vec3 vNormal;
in vec3 vCameraPos;

layout(location=0) out vec4 fragColor;
//The color for determining bloom 
// (basically just color if past threshold, otherwise rgb 0)
layout(location=1) out vec4 fragBright;

vec3 GetNormal() 
{
    /*#ifdef USE_NORMAL_MAP
    vec3 n = texture(uNormalMap, vTex).xyz * 2.0 - 1.0;
    return normalize(vTBN * n);
    #else*/

    //Since vnormal is in param it gets interpolated between vertices
    //so we ensure that it is truly normalized to prevent slight unexpected differences
    return normalize(vNormal);
}

void main() 
{
    //TEMPORARY REPLACE WITH UNIFORM
    float uSpecularPower= 5;
    vec3 normal = GetNormal();

    //vec3 debugColor = 0.5 * (normal + vec3(1.0));
    //fragColor = vec4(debugColor, 1.0);
    //return;

    vec3 viewDir = normalize(uViewerBlock.worldPos - vWorldPos);
    vec4 albedo = texture(uAlbedo, vTexCoords);
    vec3 color= vec3(0, 0, 0);

    //Here we calculate directional light impact by adding directional light color
    //based on how much light there is coming towards the surface normal
    vec3 thisToLightDir = normalize(-uLightsBlock.directionalDir); 
    float lightInNormalDir = max(dot(normal, thisToLightDir), 0.0);
    float spec=0;
    vec3 thisToViewDir= vec3(0, 0, 0);
    if (lightInNormalDir > 0.0) 
    {
        thisToViewDir = normalize(thisToLightDir + viewDir);
        spec = pow(max(dot(normal, thisToViewDir), 0.0), uSpecularPower);
        //TODO: considering light strength from color alpha
        color += uLightsBlock.directionalColor.rgb * (albedo.rgb * lightInNormalDir + spec);
    }

    //Similar to dir light, we calculate how much is the normal
    //in the direction of the point light and if it is within its radius
    //we apply similar color formula
    vec3 thisToLightDist= vec3(0, 0, 0);
    float dist=0.0;
    float attenuation=0.0;

    float closestDepth= 0.0;
    float bias= 0;
    float shadow= 0;
    for (int i = 0; i < uLightsBlock.pointLightsCount; i++) 
    {
        PointLight pl = uLightsBlock.pointLights[i];
        thisToLightDist = pl.position - vWorldPos;
        dist = length(thisToLightDist);

        if (dist <= pl.radius) 
        {
            thisToLightDir = normalize(thisToLightDist);
            //attenuation = 1.0 / (1.0 + (dist*dist)/(pl.radius*pl.radius));
            //attenuation = clamp(1.0 / (dist * dist), 0.0, 1.0);
            attenuation = (1.0 - smoothstep(pl.radius * 0.75, pl.radius, dist)) / (1.0 + dist*dist);

            lightInNormalDir = max(dot(normal, thisToLightDir), 0.0);
            if (lightInNormalDir >= 0.0) 
            {
                thisToViewDir = normalize(thisToLightDir  + viewDir);
                spec = pow(max(dot(normal, thisToViewDir), 0.0), uSpecularPower);

#ifdef DO_SHADOWS
                closestDepth = texture(uShadowMaps[pl.shadowMapIndex], -thisToLightDir).r * pl.radius;
                bias = 0.05 * (1.0 - dot(normal, -thisToLightDir));
                shadow = (dist - bias > closestDepth) ? 1.0 : 0.0;
#else
                shadow= 0;
#endif

                 //TODO: considering light strength from color alpha
                color += pl.color.rgb * attenuation * (albedo.rgb * vColor.rgb * lightInNormalDir + spec) * (1.0-shadow);
            }
        }
    }

    fragColor = vec4(color, vColor.a);

    //Computes the luminance (perceived brightness) of the hdr color output -> it is weighted sum
    //since we perceive some colors, like green more than others
    float luminance = dot(fragColor, vec4(0.2126, 0.7152, 0.0722, 1.0));
    fragBright = (luminance >= uBloomThreshold) ? fragColor : vec4(0.0, 0.0, 0.0, 1.0);
}