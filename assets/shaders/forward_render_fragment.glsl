#version 330 core

struct PointLight 
{
    vec3 position;
    vec4 color;
    float radius;
};

layout(std140) uniform LightsBlock
{
    vec3 directionalDir;
    vec4 directionalColor;
    int pointLightsCount;
    PointLight pointLights[2];
} uLightsBlock;

layout(std140) uniform CameraBlock 
{
    mat4 viewMatrix;
    mat4 projectionMatrix;
    vec3 cameraPos;
} uCameraBlock;

uniform sampler2D uAlbedo;
//uniform float uSpecularPower;

in vec2 vTexCoords;
in vec4 vColor;
in vec3 vWorldPos;
flat in vec3 vNormal;
in vec3 vCameraPos;

layout(location=0) out vec4 fragColor;

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

    vec3 viewDir = normalize(uCameraBlock.cameraPos - vWorldPos);
    vec4 albedo = texture(uAlbedo, vTexCoords);
    vec3 color = albedo.rgb * vColor.rgb;

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
    for (int i = 0; i < uLightsBlock.pointLightsCount; i++) 
    {
        PointLight pl = uLightsBlock.pointLights[i];
        thisToLightDist = pl.position - vWorldPos;
        dist = length(thisToLightDist);

        if (dist <= pl.radius) 
        {
            thisToLightDir = normalize(thisToLightDist);
            attenuation = 1.0 / (1.0 + (dist*dist)/(pl.radius*pl.radius));
            lightInNormalDir = max(dot(normal, thisToLightDir), 0.0);

            if (lightInNormalDir >= 0.0) 
            {
                thisToViewDir = normalize(thisToLightDir  + viewDir);
                spec = pow(max(dot(normal, thisToViewDir), 0.0), uSpecularPower);
                 //TODO: considering light strength from color alpha
                color += pl.color.rgb * attenuation * (albedo.rgb * lightInNormalDir + spec);
            }
        }
    }

    // gamma correction can be applied later; output linear color
    fragColor = vec4(color, vColor.a);
}