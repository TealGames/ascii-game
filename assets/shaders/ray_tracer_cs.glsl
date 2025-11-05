#version 430 core

struct Vertex 
{
    vec3 localPos;
    vec2 uvPos;
    vec3 normal;
};
struct Material
{
    vec4 baseColor;
    float alpha;
    vec4 emission;
};
struct Instance
{
    uint materialIndex;
    mat4 modelMatrix;
    mat3 normalModelMatrix;
};
struct InstanceMesh
{
    uint indexOffset;
    //The total number of indices to read for vertices
    uint numIndices;
};

//The local size is the work group size -> how many threads there are per group 
//(the number of groups is determined with compute shader call)
layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

uniform ivec2 uScreenSize;
uniform uint uMaxBounces;
uniform uint uUnmovingFrameCount;
uniform uint uEmissiveMaterialCount;
uniform uint uInstanceCount;

layout(std140) uniform ViewerBlock
{
    mat4 viewMatrix;
    mat4 projectionMatrix;
    vec3 worldPos;
    vec3 forwardDir;
    vec3 rightDir;
    vec3 upDir;
    float yFov;
} uViewerBlock;

//This is a shader storage buffer object -> similar to uniform buffer
//but allows more space, dynamic arrays (no compile time predefined size), and read + write
layout(std430) buffer Vertices { Vertex vertices[VERTEX_MAX_COUNT]; };
layout(std430) buffer Indices { uint indices[INDEX_MAX_COUNT]; };
layout(std430) buffer Instances { Instance instances[INSTANCE_MAX_COUNT]; };
layout(std430) buffer Materials { Material materials[MATERIAL_MAX_COUNT]; };
//The indices into the instance list that are emissive
layout(std430) buffer LightIndices { uint lightIndices[INSTANCE_MAX_COUNT]; };
layout(std430) buffer InstanceMeshes { InstanceMesh meshes[INSTANCE_MAX_COUNT]; };

//The texure that was previosuly used for output and can be blended with current one to provide more detail
layout(rgba16f) uniform image2D uTextureInput;
layout(rgba16f) uniform writeonly image2D uTextureOutput;

//Computes whether a ray at origin and dir hits a triangle defined by 3 vertices
//where hitDistance is the scalar distance from ray origin along ray dir to the intersection point of the triangle
//and triangle normal is the normal of the triangle
bool DoesIntersectTriangle(vec3 rayWorldOrigin, vec3 rayWorldDir, vec3 v0, vec3 v1, vec3 v2, out float hitDistance, out vec3 triangleNormal)
{
    vec3 e1 = v1 - v0;
    vec3 e2 = v2 - v0;
    //triangleNormal = normalize(cross(e1, e2));
    triangleNormal = normalize(cross(e2, e1));

    vec3 pvec = cross(rayWorldDir, e2);
    float det = dot(e1, pvec);
    if (abs(det) < 1e-6) 
        return false;

    float invDet = 1.0 / det;
    vec3 tvec = rayWorldOrigin - v0;
    float u = dot(tvec, pvec) * invDet;
    if (u < 0.0 || u > 1.0) 
        return false;

    vec3 qvec = cross(tvec, e1);
    float v = dot(rayWorldDir, qvec) * invDet;
    if (v < 0.0 || u + v > 1.0) 
        return false;

    hitDistance = dot(e2, qvec) * invDet;
    if (hitDistance < 1e-6) 
        return false;

    return true;
}

bool DoesIntersectTriangleInterpolated(
    vec3 rayWorldOrigin,
    vec3 rayWorldDir,
    vec3 v0, vec3 v1, vec3 v2,
    vec3 n0, vec3 n1, vec3 n2,
    out float hitDistance,
    out vec3 triangleNormal
)
{
    vec3 e1 = v1 - v0;
    vec3 e2 = v2 - v0;

    vec3 pvec = cross(rayWorldDir, e2);
    float det = dot(e1, pvec);
    if (abs(det) < 1e-6) 
        return false;

    float invDet = 1.0 / det;
    vec3 tvec = rayWorldOrigin - v0;

    float u = dot(tvec, pvec) * invDet;
    if (u < 0.0 || u > 1.0) 
        return false;

    vec3 qvec = cross(tvec, e1);
    float v = dot(rayWorldDir, qvec) * invDet;
    if (v < 0.0 || u + v > 1.0) 
        return false;

    hitDistance = dot(e2, qvec) * invDet;
    if (hitDistance < 1e-6) 
        return false;

    // Compute barycentric coordinates
    float w = 1.0 - u - v;

    // Interpolate vertex normals
    triangleNormal = normalize(w * n0 + u * n1 + v * n2);

    return true;
}

bool DoesIntersectSceneWorld(vec3 rayOriginWorld, vec3 rayDirWorld, out vec3 hitPos, out vec3 hitNormal, 
                        out Material hitMaterial, inout uint seed)
{
    bool hit = false;
    float hitDistanceMin = 1e20;
    for (uint instanceIndex = 0u; instanceIndex < uInstanceCount; instanceIndex++)
    {
        Instance instance = instances[instanceIndex];
        InstanceMesh mesh = meshes[instanceIndex];
        float modelMatDeterminant = determinant(mat3(instance.modelMatrix));

        // Loop through all triangles in this mesh
        for (uint indexIndex = 0u; indexIndex < mesh.numIndices; indexIndex += 3)
        {
            uint indexV0 = indices[mesh.indexOffset + indexIndex + 0];
            uint indexV1 = indices[mesh.indexOffset + indexIndex + 1];
            uint indexV2 = indices[mesh.indexOffset + indexIndex + 2];

            vec3 vertexWorld0 = vec3(instance.modelMatrix * vec4(vertices[indexV0].localPos, 1.0));
            vec3 vertexWorld1 = vec3(instance.modelMatrix * vec4(vertices[indexV1].localPos, 1.0));
            vec3 vertexWorld2 = vec3(instance.modelMatrix * vec4(vertices[indexV2].localPos, 1.0));

            float hitDistance;
            vec3 triangleNormal;

            if (DoesIntersectTriangle(rayOriginWorld, rayDirWorld, vertexWorld0, 
                                      vertexWorld1, vertexWorld2, hitDistance, triangleNormal))
            /*
            if (DoesIntersectTriangleInterpolated(rayOriginWorld, rayDirWorld, vertices[indexV0].localPos, 
               vertices[indexV1].localPos, vertices[indexV2].localPos, vertices[indexV0].normal, 
               vertices[indexV1].normal, vertices[indexV2].normal, hitDistance, triangleNormal))
            */
            {
                //if (modelMatDeterminant < 0)
                // triangleNormal = -triangleNormal;

                if (dot(rayDirWorld, triangleNormal) > 0.0)
                    continue;

                //if (dot(triangleNormal, rayDirWorld) > 0.0)
                //triangleNormal = -triangleNormal;

                // compute world-space hit position
                vec3 candidateHitPos = rayOriginWorld + rayDirWorld * hitDistance;

                // use a small positive epsilon along the normal to avoid self-intersection for the next ray
                const float eps = 1e-4; // try 1e-4 or 1e-3 depending on scene scale
                vec3 offsetOrigin = candidateHitPos + triangleNormal * eps;

                if (hitDistance > 1e-6 && hitDistance < hitDistanceMin)
                {
                    hitDistanceMin = hitDistance;
                    hitPos = candidateHitPos;
                    hitNormal = normalize(triangleNormal);
                    hitMaterial = materials[instance.materialIndex];
                    hit = true;
                }

                /*
                if (hitDistance > 1e-6 && hitDistance < hitDistanceMin)
                {
                    hitDistanceMin = hitDistance;
                    hitPos = rayOriginWorld + rayDirWorld * hitDistance;
                    hitNormal = normalize(triangleNormal);
                    hitMaterial = materials[instance.materialIndex];
                    hit = true;
                }
                */
            }
        }
    }

    return hit;
}

// Intersect all instances
bool DoesIntersectScene(vec3 rayOriginWorld, vec3 rayDirWorld, out vec3 hitPos, 
                        out vec3 hitNormal, out Material hitMaterial, inout uint seed) 
{
    // Loop over all instances
    bool hit = false;
    float hitDistanceMin = 1e20;
    for (uint instanceIndex = 0u; instanceIndex < uInstanceCount; instanceIndex++) 
    {
        Instance instance = instances[instanceIndex];
        InstanceMesh mesh = meshes[instanceIndex];
        mat4 worldToLocalModelMatrix = inverse(instance.modelMatrix);

        vec3 rayOriginLocal = vec3(worldToLocalModelMatrix * vec4(rayOriginWorld, 1.0));
        vec3 rayDirLocal = normalize(vec3(worldToLocalModelMatrix * vec4(rayDirWorld, 0.0)));

        //NOTE: we assume that the 3 vertices are contiguous so we iterate over the 
        //over all possible indices into the index buffer for the mesh, and then
        //we get the actual indices into the vertex buffer and check the vertex local pos
        //for any intersections
        for (uint indexIndex = 0u; indexIndex < mesh.numIndices; indexIndex+=3) 
        {
            uint indexV0 = indices[mesh.indexOffset + indexIndex];
            uint indexV1 = indices[mesh.indexOffset + indexIndex + 1];
            uint indexV2 = indices[mesh.indexOffset + indexIndex + 2];

            float hitDistance = 0;
            vec3 triangleNormal = vec3(0.0);

            /*
            if (DoesIntersectTriangle(rayOriginLocal, rayDirLocal, vertices[indexV0].localPos, 
               vertices[indexV1].localPos, vertices[indexV2].localPos, hitDistance, triangleNormal)) 
               */
            if (DoesIntersectTriangleInterpolated(rayOriginLocal, rayDirLocal, vertices[indexV0].localPos, 
               vertices[indexV1].localPos, vertices[indexV2].localPos, vertices[indexV0].normal, 
               vertices[indexV1].normal, vertices[indexV2].normal, hitDistance, triangleNormal)) 
            {
                // after DoesIntersectTriangle yields hitDistance (in local space)
                if (hitDistance < hitDistanceMin) 
                {
                    hitDistanceMin = hitDistance;
                    hitPos = vec3(instance.modelMatrix * vec4(rayOriginLocal + rayDirLocal * hitDistance, 1.0));
                    //hitNormal = normalize(mat3(instance.modelMatrix) * triangleNormal);
                    hitNormal = normalize(instance.normalModelMatrix * triangleNormal);
                    hitMaterial = materials[instance.materialIndex];
                    hit = true;

                    //NOTE: we do not return so that we find the closest possible min distance that ray travels
                }
            }
        }
    }
    return hit;
}

uint CreateHash(uint x) 
{
    x ^= x >> 17; 
    x *= 0xed5ad4bbu;
    x ^= x >> 11; 
    x *= 0xac4c1b51u;
    x ^= x >> 15; 
    x *= 0x31848babu;
    x ^= x >> 14;
    return x;
}

//Generates a random number in range [0, 1)
float GenerateRandomNum(inout uint state) 
{ 
    state = CreateHash(state); 
    return float(state) / 4294967296.0; 
}

vec3 SamplePointOnTriangle(vec3 v0, vec3 v1, vec3 v2, inout uint seed) 
{
    float u = sqrt(GenerateRandomNum(seed));
    float v = GenerateRandomNum(seed);
    return (1.0 - u) * v0 + u * (1.0 - v) * v1 + u * v * v2;
}

void main() 
{
	ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
	if (pixel.x >= uScreenSize.x || pixel.y >= uScreenSize.y)
		return;

    uint seed = uint(pixel.x * 1973u + pixel.y * 9277u) + uUnmovingFrameCount * 26699u;
  
    //The following apply a small <1 jitter to the pixel coordinate in order to prevent aliasing (jagged edges)
    //If we always used the center of the pixel it would not appear smooth, so by doing this combined with accumulation
    //we get a nicer more-filled and less jagged look to edges
    float jitteredPixelX = (float(pixel.x) + GenerateRandomNum(seed)) / float(uScreenSize.x);
    float jitteredPixelY = (float(pixel.y) + GenerateRandomNum(seed)) / float(uScreenSize.y);
    //This is the jittered pixel coord in normalized device coordinate pos [-1, 1] 
    //(horizontal and vertical offset from center of screen)
    vec2 jitteredNDC = vec2(jitteredPixelX * 2.0 - 1.0, jitteredPixelY * 2.0 - 1.0);
    float aspectRatio = float(uScreenSize.x) / float(uScreenSize.y);
    float scale = tan(uViewerBlock.yFov * 0.5);
    vec3 rayDirWorld = normalize(uViewerBlock.forwardDir + uViewerBlock.rightDir * (jitteredNDC.x * aspectRatio * scale) 
                    + uViewerBlock.upDir * (jitteredNDC.y * scale));
    vec3 rayOriginWorld = uViewerBlock.worldPos;

    /*
    vec3 debugDir = rayDirWorld * 0.5 + 0.5; 
    imageStore(uTextureOutput, pixel, vec4(debugDir, 1.0));
    imageStore(uTextureInput, pixel, vec4(debugDir, 1.0));
    return;
    */

    //Radiance is the total color that gets accumulated for this ray
    vec3 radiance = vec3(0.0);
    //Energy percent is the multiplier for rgb which decreases as ray hits an object
    //and energy decreases based on the color (how much light is absorbed)
    vec3 throughput = vec3(1.0);
    
    for (int bounce = 0; bounce < uMaxBounces; bounce++) 
    {   
        vec3 hitPos;
        vec3 hitNormalWorld;
        Material hitMaterial;
        if (!DoesIntersectSceneWorld(rayOriginWorld, rayDirWorld, hitPos, hitNormalWorld, hitMaterial, seed)) 
        {
            vec3 sky = mix(vec3(0.6, 0.7, 0.9), vec3(0.2, 0.35, 0.6), 0.5 * (jitteredPixelY + 1.0));
            //vec3 sky = vec3(0, 0, 0);
            radiance += throughput * sky;
            break;
        }
        /*
        else 
        {
         imageStore(uTextureOutput, pixel, vec4(hitNormalWorld * 0.5 + 0.5, 1.0));
         imageStore(uTextureInput, pixel, vec4(hitNormalWorld * 0.5 + 0.5, 1.0));
        }
        */
        /*
        else
        {
            radiance += vec3(hitNormalWorld * 0.5 + 0.5);
            break;
        } 
        */

        if (hitMaterial.emission.a > 0) 
        {
            radiance += throughput * hitMaterial.emission.rgb;
        }

        // ----- Stochastic light sampling -----
        // Here we pick a random triangle on the light to see if hit object gets affected by this light
        // NOTE: because we assume every 3 is a triangle, we have to divide by 3 to find the triangle index
        // and we multiple by 3 to convert the triangle index to a vertex index
        uint randomLightInstanceIndex = lightIndices[uint(GenerateRandomNum(seed) * float(uEmissiveMaterialCount))];
        Instance lightInstance= instances[randomLightInstanceIndex];
        InstanceMesh lightMeshInstance = meshes[randomLightInstanceIndex];
        uint randomTriangle = uint(GenerateRandomNum(seed) * float(lightMeshInstance.numIndices / 3));
        uint baseIndex = lightMeshInstance.indexOffset + randomTriangle * 3u;
        uint lightIndexV0 = indices[baseIndex + 0];
        uint lightIndexV1 = indices[baseIndex + 1];
        uint lightIndexV2 = indices[baseIndex + 2];
        //Here we find the sample of the light using random 
        vec3 lightSample = SamplePointOnTriangle(vertices[lightIndexV0].localPos, vertices[lightIndexV1].localPos, 
                           vertices[lightIndexV2].localPos, seed);
        
        vec3 lightDir = normalize(hitPos - lightSample);
        float hitDistanceToLight = length(hitPos - lightSample);
        float lightInNormalDir = max(0.0, dot(hitNormalWorld, lightDir));

        //If the object we hit is affected by a light (meaning there are no objects blocking the path from a random 
        //light to this object), we can then add that light's intensity/color to ray -> gains energy
        //NOTE: it does not matter that we choose a random light because due to accumulation, this will eventually fill out all lights
        vec3 shadowHitPos;
        vec3 shadowHitNormal; 
        Material shadowHitMaterial;
        //By scaling shadow ray origin by epsilon from scene, we avoid unwanted shadow artifacts
        const float epsilon = 1e-3 * max(1.0, length(hitPos));
        vec3 shadowOrigin = hitPos + hitNormalWorld * epsilon;

        bool blocked = DoesIntersectSceneWorld(shadowOrigin, lightDir, shadowHitPos, shadowHitNormal, shadowHitMaterial, seed);
        if (!blocked || length(shadowHitPos - hitPos) > hitDistanceToLight - 0.001) 
        {
            radiance += throughput * lightInNormalDir; 
        }

        // ----- Sample new diffuse direction -----
        // Cosine-weighted hemisphere sampling: 
        // generates random values and then uses it to determine new reflected direction
        // which gives a bias towards directions closer to the normal
        float random1 = GenerateRandomNum(seed);
        float random2 = GenerateRandomNum(seed);
        float phi = 2.0 * 3.14159265 * random1;
        float cosTheta = sqrt(1.0 - random2);
        float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
        vec3 u = normalize(cross(abs(hitNormalWorld.x) > 0.1 ? vec3(0,1,0) : vec3(1,0,0), hitNormalWorld));
        vec3 v = cross(hitNormalWorld, u);
        vec3 newRayDir = normalize(u * sinTheta * cos(phi) + v * sinTheta * sin(phi) + hitNormalWorld * cosTheta);

        rayDirWorld = newRayDir;
        rayOriginWorld = hitPos + hitNormalWorld * 0.001;
        throughput *= hitMaterial.baseColor;

        // Russian roulette termination:
        // if the max rgb channel with energy left is surpassed by random number
        // then we exit
        float maxThroughputChannel = max(max(throughput.r, throughput.g), throughput.b);
        if (GenerateRandomNum(seed) > maxThroughputChannel) 
            break;

        throughput /= maxThroughputChannel;
    }

    /*
    vec4 prev = imageLoad(uTextureInput, pixel);
    const float blendFactor = 0.1;
    vec3 blended = mix(prev.rgb, radiance, blendFactor);

    imageStore(uTextureOutput, pixel, vec4(blended, 1.0));
    imageStore(uTextureInput, pixel, vec4(blended, 1.0));
    */

    // Progressive accumulation using texture input
    vec4 prev = (uUnmovingFrameCount == 0u) ? vec4(0.0) : imageLoad(uTextureInput, pixel);
    vec3 blended = (prev.rgb * float(uUnmovingFrameCount) + radiance) / float(uUnmovingFrameCount + 1u);

    //imageStore(uTextureOutput, pixel, vec4(1.0, 0, 0, 1.0));
    //TODO: transparency is not supported yet
    imageStore(uTextureOutput, pixel, vec4(blended, 1.0));
    imageStore(uTextureInput, pixel, vec4(blended, 1.0));
}