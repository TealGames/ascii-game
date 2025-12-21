#version 430 core

//Smooth shading will interpolate normals from vertices when tracing
#define DO_SMOOTH_SHADING 1
#define USE_BVH 1
//Will use the modified normal for environment lighting
#define USE_BENT_NORMAL_FOR_LIGHTING 1
#define DO_AMBIENT_OCCLUSION 1

struct BVHNode
{
    //Min pos, max pos
	vec3 bounds[2];

	int objectStartIndex;
	uint objectCount;

	int indexChild0;
	int indexChild1;
};
bool IsLeaf(BVHNode node)
{
    return node.objectCount > 0 && node.objectStartIndex >= 0;
}

struct Vertex 
{
    vec3 localPos;
    vec2 uvPos;
    vec3 normal;
};
struct Material
{
    vec4 baseColor;
    vec4 emission;
    float alpha;
    float metallic;
    float roughness;
    int albedoIndex;
};
struct Instance
{
    uint materialIndex;
    uint meshIndex;
    mat4 modelMatrix;
    mat4 inverseModelMatrix;
    mat3 normalModelMatrix;
};
struct InstanceMesh
{
    uint indexOffset;
    //The total number of indices to read for vertices
    uint numIndices;
    
    uint blasTreeOffset;
    uint blasTreeNodeCount;
};

struct PointLight 
{
    vec3 position;
    vec4 color;
    float radius;
    uint shadowMapIndex;
};

//The local size is the work group size -> how many threads there are per group 
//(the number of groups is determined with compute shader call)
layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

uniform ivec2 uScreenSize;
uniform uint uMaxBounces;
uniform uint uAmbientOcclusionSamples;
uniform uint uUnmovingFrameCount;
uniform uint uEmissiveMaterialCount;
uniform uint uInstanceCount;
uniform float uBloomThreshold;

uniform bool uHasSkybox;
uniform sampler2D uSkybox;
//TODO: eventaully use texture2darray type (all textures must be the same size and stuff them into array)
// or get a texture atlas for each type of texture (like normals, albedos, etc)
uniform sampler2D uTextures[TEXTURE_MAX_COUNT];

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

layout(std140) uniform LightsBlock
{
    vec3 directionalDir;
    vec4 directionalColor;
    int pointLightsCount;
    PointLight pointLights[MAX_POINT_LIGHTS];
} uLightsBlock;

//This is a shader storage buffer object -> similar to uniform buffer
//but allows more space, dynamic arrays (no compile time predefined size), and read + write
layout(std430) buffer Vertices { Vertex vertices[VERTEX_MAX_COUNT]; };
layout(std430) buffer Indices { uint indices[INDEX_MAX_COUNT]; };
layout(std430) buffer Instances { Instance instances[INSTANCE_MAX_COUNT]; };
layout(std430) buffer Materials { Material materials[MATERIAL_MAX_COUNT]; };
//The indices into the instance list that are emissive
layout(std430) buffer LightIndices { uint lightIndices[INSTANCE_MAX_COUNT]; };
layout(std430) buffer InstanceMeshes { InstanceMesh meshes[INSTANCE_MAX_COUNT]; };
//The tree which stores the world bvh tree where each object at leaves
//are the indices into the instances
layout(std430) buffer TLASTree { BVHNode tlasTree[TLAS_NODE_MAX_COUNT]; };
//The local bvh trees where each leaf object index are the indices into the vertex index buffer.
//NOTE: this contains all mesh bvh trees packed together
layout(std430) buffer BLASTrees { BVHNode blasTrees[BLAS_NODE_MAX_COUNT]; };

//The texure that was previosuly used for output and can be blended with current one to provide more detail
layout(rgba16f) uniform image2D uTextureInput;
layout(rgba16f) uniform writeonly image2D uTextureOutput;
uniform writeonly image2D uBrightnessTexture;

const float EPSILON = 1e-8;
const float PI = 3.14159265359;

/* Fresnel Reflectance: computes how much light is reflected or
   refracted based on the viewing angle
   NOTE: since true Fresnel equation is more complicated and performance intensive we use Schlick approximation
   F_schlick(v, h) = F0 + (1 - F0) * (1- <v * h>)^5
   <param cosTheta> the dot product of the surface to view direction vector and the halfway vector
                    (halfway vector: halfway normalized vector for direction from surface to light and direction from surface to camera) <param>
   <param F0> at the transition from air to this material at normal incidence, the percent of light reflected<param>

   EXAMPLE: if you think of a pool while looking at an angle, the closer part to viewer has more light transmitted
   through the water, so you can see to the pool's bottom, but the farther section from viewer has more light
   reflected so you just see reflection from other objects and can not see the pool's bottom */
vec3 FresnelSchlickReflectance(float cosTheta, vec3 F0) 
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

/* Computes the GGX normal distribution function for Cook-Torrance BRDF equation
   D_ggx(h) = a^2 / (pi * (<n * h>^2 * (a^2 - 1) + 1)^2)
   <param alpha> material roughness^2 <param>
   <param NdotH> the dot product of the normal and halfway vector 
                (halfway vector: halfway normalized vector for direction from surface to light and direction from surface to camera) <param>
 
   EXPLANATIOn: the more roughness an object has, the more variation there is for the reflected vector.
   If an object has no roughness (polished object) then the light vector shares the same angle with the surface normal
   as the reflected vector. However, the more roughness an object has, the more microfacets it contains, and therefore
   has more variation of resulting reflection vectors. The normal distribution function computes the distribution of 
   possible reflected vectors around the ideal perfectly polished reflection vector
 */
float NormalDistributionGGX(float NdotH, float alpha) 
{
    float a2 = alpha * alpha;
    float denom = (NdotH * NdotH) * (a2 - 1.0) + 1.0;
    denom = PI * denom * denom;
    return a2 / max(EPSILON, denom);
}

/*
   Fast approximation for a part of the geometry term equation (G1) for the GGX variant
   <param NdotV> the dot product of the surface normal and another direction (either light to surface, or surface to camera) <param>
   <param k> alpha / 2 (alpha: roughness^2) <param>
*/
float GeometrySchlickGGX(float NdotV, float k) 
{
    return max(NdotV, EPSILON) / (NdotV * (1.0 - k) + k);
}
/* Computes the geometry term in the Cook-Torrance BRDF equation
   G_smith(l,v) = G1(v) * G1(l)
   <param NdotV> the dot product of normal surface vector and viewing vector from surface to camera<param>
   <param NdotL> the dot product of normal surface vector and surface to light vector<param>
   
   EXPLANATION: Cook Torrance BRDF equation assumes microfacets are v-shaped and 
   thus depending on the direction of incoming light and the viewing direction of
   the observer, shadowing and masking effects may occur. The goemtry term derives
   a geometry factor [0,1] for this effect
*/
float GeometrySmith(float NdotV, float NdotL, float k) 
{
    return GeometrySchlickGGX(NdotV, k) * GeometrySchlickGGX(NdotL, k);
}

// Importance sample GGX: sample half-vector H in tangent space then reflect view to get direction
vec3 ImportanceSampleGGX(float Xi1, float Xi2, vec3 N, float alpha) 
{
    // alpha = roughness^2
    float phi = 2.0 * PI * Xi1;
    float cosTheta = sqrt((1.0 - Xi2) / (1.0 + (alpha*alpha - 1.0) * Xi2));
    float sinTheta = sqrt(max(0.0, 1.0 - cosTheta * cosTheta));

    // spherical coords to cartesian in tangent space
    vec3 Ht = vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);

    // build tangent space (any stable orthonormal basis)
    vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangentX = normalize(cross(up, N));
    vec3 tangentY = cross(N, tangentX);

    // transform Ht to world
    vec3 H = normalize(tangentX * Ht.x + tangentY * Ht.y + N * Ht.z);
    return H;
}

// Cosine-weighted hemisphere sample (for diffuse)
vec3 CosineSampleHemisphere(float Xi1, float Xi2, vec3 N) 
{
    float r = sqrt(Xi1);
    float theta = 2.0 * PI * Xi2;
    float x = r * cos(theta);
    float y = r * sin(theta);
    float z = sqrt(max(0.0, 1.0 - x*x - y*y));

    // tangent basis
    vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangentX = normalize(cross(up, N));
    vec3 tangentY = cross(N, tangentX);
    return normalize(tangentX * x + tangentY * y + N * z);
}

// PDF for GGX sample (half-vector H)
float PDF_GGX(float NdotH, float alpha, float VdotH) 
{
    float D = NormalDistributionGGX(NdotH, alpha);
    // pdf for sampling H: D * NdotH
    // conversion to pdf over sample direction L (when reflecting V about H): pdfL = (D * NdotH) / (4 * VdotH)
    return D * NdotH / max(EPSILON, 4.0 * VdotH);
}

// PDF for cosine hemisphere sample
float PDF_CosineHemisphere(float NdotL) 
{
    return NdotL / PI;
}

// Evaluate Cook-Torrance microfacet BRDF (returns f and outputs specular)
vec3 EvaluateMicrofacetBRDF(vec3 N, vec3 V, vec3 L, vec3 albedo, float metallic, float roughness, out vec3 outSpec, out float outPdf) {
    vec3 H = normalize(V + L);
    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    float VdotH = max(dot(V, H), 0.0);

    float alpha = roughness * roughness;
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    float D = NormalDistributionGGX(NdotH, alpha);
    float k = (alpha + 1.0) * (alpha + 1.0) / 8.0; // UE4-style remap for G
    float G = GeometrySmith(NdotV, NdotL, k);
    vec3 F = FresnelSchlickReflectance(VdotH, F0);

    vec3 numerator = D * G * F;
    //NOTE: we use max to prevent division by 0
    float denominator = 4.0 * max(EPSILON, NdotV * NdotL);
    vec3 specular = numerator / denominator;

    // Energy-conserving diffuse (Lambert) factor only for non-metals
    vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);
    vec3 diffuse = kD * albedo / PI;

    outSpec = specular;

    // PDF mix is computed where sampling is performed; here return placeholder 0
    outPdf = 0.0;
    return diffuse + specular;
}

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
    if (abs(det) < EPSILON) 
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
    if (hitDistance < EPSILON) 
        return false;

    return true;
}

bool DoesIntersectTriangleInterpolated(vec3 rayWorldOrigin, vec3 rayWorldDir, vec3 v0, vec3 v1, vec3 v2,
    vec3 n0, vec3 n1, vec3 n2, out float hitDistance, out vec3 triangleNormal)
{
    vec3 e1 = v1 - v0;
    vec3 e2 = v2 - v0;

    vec3 pvec = cross(rayWorldDir, e2);
    float det = dot(e1, pvec);
    if (abs(det) < EPSILON) 
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
    if (hitDistance < EPSILON) 
        return false;

    // Compute barycentric coordinates
    float w = 1.0 - u - v;

    // Interpolate vertex normals
    triangleNormal = normalize(w * n0 + u * n1 + v * n2);

    return true;
}

bool DoesIntersectBounds(vec3 bounds[2], vec3 rayOrigin, vec3 inverseRayDir, ivec3 rayDirSign, 
                         out float outTMin, out float outTMax)
{
/*
	//NOTE: we use inverse dir since multiply is faster than divide
	vec3 tMin = (bounds.boundsMin - rayOrigin) * inverseRayDir;
	vec3 tMax = (bounds.boundsMax - rayOrigin) * inverseRayDir;

    float temp = 0;
	if (inverseRayDir.x < 0)
    {
        temp = tMin.x;
        tMin.x = tMax.x;
        tMax.x = temp;
    }
	if (inverseRayDir.y < 0)
    {
        temp = tMin.y;
        tMin.y = tMax.y;
        tMax.y = temp;
    }
	if (inverseRayDir.z < 0)
    {
        temp = tMin.z;
        tMin.z = tMax.z;
        tMax.z = temp;
    }

	float tEnter = max(tMin.x, max(tMin.y, tMin.z));
	float tExit = min(tMax.x, min(tMax.y, tMax.z));

	return tEnter <= tExit && tExit >= 0;
*/
    float tMin  = (bounds[rayDirSign.x].x - rayOrigin.x) * inverseRayDir.x;
    float tMax  = (bounds[1 - rayDirSign.x].x - rayOrigin.x) * inverseRayDir.x;

    float tMinY = (bounds[rayDirSign.y].y - rayOrigin.y) * inverseRayDir.y;
    float tMaxY = (bounds[1 - rayDirSign.y].y - rayOrigin.y) * inverseRayDir.y;

    // Slab separation check
    if (tMin > tMaxY || tMinY > tMax)
        return false;

    // Merge Y slab
    if (tMinY > tMin) tMin = tMinY;
    if (tMaxY < tMax) tMax = tMaxY;

    float tMinZ = (bounds[rayDirSign.z].z - rayOrigin.z) * inverseRayDir.z;
    float tMaxZ = (bounds[1 - rayDirSign.z].z - rayOrigin.z) * inverseRayDir.z;

    // Slab separation check
    if (tMin > tMaxZ || tMinZ > tMax)
        return false;

    // Merge Z slab
    if (tMinZ > tMin) tMin = tMinZ;
    if (tMaxZ < tMax) tMax = tMaxZ;

    outTMin = tMin;
    outTMax = tMax;
    return true;
}

/*
   Calculates the weight of the targetPos based on the 3 positions
   in normalized position where (1, 0, 0) would be barycentric weight of
   v0; (0, 1, 0) of v1; (0, 0, 1) of v2
*/
vec3 CalculateBarycentricWeight(vec3 targetPos, vec3 v0, vec3 v1, vec3 v2)
{
    vec3 e1 = v1 - v0;
    vec3 e2 = v2 - v0;
    vec3 p = targetPos - v0;
    float area = length(cross(e1, e2));
    float a0 = length(cross(v1 - targetPos, v2 - targetPos)) / area;
    float a1 = length(cross(v2 - targetPos, v0 - targetPos)) / area;
    float a2 = 1.0 - a0 - a1;
    return vec3(a0, a1, a2);
}

vec3 SampleEquirectangular(vec3 dir, sampler2D hdrMap)
{
    float theta = atan(dir.z, dir.x); 
    float phi   = asin(dir.y); 

    // Map theta from [-pi, pi] to [0,1]
    float u = (theta + 3.14159265) / (2.0 * 3.14159265);
    // Map phi from [-pi/2, pi/2] to [0,1]
    float v = (phi + 1.57079633) / 3.14159265;

    return texture(hdrMap, vec2(u, v)).rgb;
}

bool DoesIntersectLocalObjectBVH(vec3 rayOriginLocal, vec3 rayDirLocal, Instance instance, out float minHitDistance, out vec3 hitWorldNormal, 
                        out Material hitMaterial, inout uint seed, out uint hitIndexV0, out uint hitIndexV1, out uint hitIndexV2, 
                        out vec3 hitVertexWorld0, out vec3 hitVertexWorld1, out vec3 hitVertexWorld2)
{
    vec3 inverseLocalRayDir = 1 / rayDirLocal;
    ivec3 localRayDirSign = ivec3(lessThan(inverseLocalRayDir, vec3(0.0)));

    uint nodeIndex = meshes[instance.meshIndex].blasTreeOffset;
    BVHNode node = blasTrees[nodeIndex];
    float tMin, tMax;
    //NOTE: technically, if everything done correct,
    //the index into the blas tree should never be outside the bounds of the instance tree
    while (!IsLeaf(node))
	{
		if (DoesIntersectBounds(blasTrees[node.indexChild0].bounds, rayOriginLocal, inverseLocalRayDir, localRayDirSign, tMin, tMax))
			nodeIndex = node.indexChild0;
		else if (DoesIntersectBounds(blasTrees[node.indexChild1].bounds, rayOriginLocal, inverseLocalRayDir, localRayDirSign, tMin, tMax))
			nodeIndex = node.indexChild1;
		else return false;

        node = blasTrees[nodeIndex];
	}

    bool hit = false;
    float hitDistanceMin = 1e20;
    //We iterate over all indices into the vertex index buffer
    for (int i = 0; i < node.objectCount; i+=3)
    {
        uint indexV0 = indices[node.objectStartIndex + i];
        uint indexV1 = indices[node.objectStartIndex + i + 1];
        uint indexV2 = indices[node.objectStartIndex + i + 2];

        float hitDistance;
        vec3 triangleNormal;

#if DO_SMOOTH_SHADING
        if (DoesIntersectTriangleInterpolated(
            rayOriginLocal, rayDirLocal,
            vertices[indexV0].localPos, vertices[indexV1].localPos, vertices[indexV2].localPos,
            vertices[indexV0].normal, vertices[indexV1].normal, vertices[indexV2].normal,
            hitDistance, triangleNormal))
#elif
        if (DoesIntersectTriangle(rayOriginWorld, rayDirWorld, vertices[indexV0].localPos, 
            vertices[indexV1].localPos, vertices[indexV2].localPos, hitDistance, triangleNormal))
#endif
        {
            if (dot(rayDirLocal, triangleNormal) > 0.0)
                continue;

            if (hitDistance > EPSILON && hitDistance < hitDistanceMin)
            {
                hitIndexV0 = indexV0;
                hitIndexV1 = indexV1;
                hitIndexV2 = indexV2;

                hitVertexWorld0 = vec3(instance.modelMatrix * vec4(vertices[indexV0].localPos, 1.0));
                hitVertexWorld1 = vec3(instance.modelMatrix * vec4(vertices[indexV1].localPos, 1.0));
                hitVertexWorld2 = vec3(instance.modelMatrix * vec4(vertices[indexV2].localPos, 1.0));

                minHitDistance = hitDistance;
                hitWorldNormal = instance.normalModelMatrix * normalize(triangleNormal);
                hitMaterial = materials[instance.materialIndex];
                hit = true;
            }
        }
    }
    return hit;
}

bool DoesIntersectSceneWorldBVH(vec3 rayOriginWorld, vec3 rayDirWorld, out vec3 hitPos, out vec3 hitWorldNormal, 
                        out Material hitMaterial, inout uint seed, out uint hitIndexV0, out uint hitIndexV1, out uint hitIndexV2, 
                        out vec3 hitVertexWorld0, out vec3 hitVertexWorld1, out vec3 hitVertexWorld2)
{
	vec3 inverseWorldRayDir = 1 / rayDirWorld;
    ivec3 worldRayDirSign = ivec3(lessThan(inverseWorldRayDir, vec3(0.0)));
    float tMin, tMax;
	if (!DoesIntersectBounds(tlasTree[0].bounds, rayOriginWorld, inverseWorldRayDir, worldRayDirSign, tMin, tMax))
		return false;

	uint nodeIndex = 0;
    BVHNode node = tlasTree[nodeIndex];
	while (!IsLeaf(node))
	{
		if (DoesIntersectBounds(tlasTree[node.indexChild0].bounds, rayOriginWorld, inverseWorldRayDir, worldRayDirSign, tMin, tMax))
			nodeIndex = node.indexChild0;
		else if (DoesIntersectBounds(tlasTree[node.indexChild1].bounds, rayOriginWorld, inverseWorldRayDir, worldRayDirSign, tMin, tMax))
			nodeIndex = node.indexChild1;
		else return false;

        node = tlasTree[nodeIndex];
	}

    //Here we iterate over all possible LOCAL OBJECT SPACE BLAS TREE ROOT NODES
    bool hit = false;
    vec3 rayOriginLocal, rayDirLocal;
    float minHitDistance;
	for (int i = 0; i < node.objectCount; i++)
	{
        Instance instance = instances[node.objectStartIndex + i];

        rayOriginLocal = (instance.inverseModelMatrix * vec4(rayOriginWorld, 1)).xyz;
        rayDirLocal = normalize((instance.inverseModelMatrix * vec4(rayDirWorld, 0)).xyz);
        hit= DoesIntersectLocalObjectBVH(rayOriginLocal, rayDirLocal, instance, minHitDistance, hitWorldNormal, hitMaterial, seed, 
                                         hitIndexV0, hitIndexV1, hitIndexV2, hitVertexWorld0, hitVertexWorld1, hitVertexWorld2);
        if (hit)
        {
            hitPos = rayOriginWorld + rayDirWorld * minHitDistance;
        }
	}
	return hit;
}

bool DoesIntersectSceneWorldNaive(vec3 rayOriginWorld, vec3 rayDirWorld, out vec3 hitPos, out vec3 hitNormal, 
                        out Material hitMaterial, inout uint seed, out uint hitIndexV0, out uint hitIndexV1, 
                        out uint hitIndexV2, out vec3 hitVertexWorld0, out vec3 hitVertexWorld1, out vec3 hitVertexWorld2)
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

            vec3 normal0 = vertices[indexV0].normal;
            vec3 normal1 = vertices[indexV1].normal;
            vec3 normal2 = vertices[indexV2].normal;

            float hitDistance;
            vec3 triangleNormal;

#if DO_SMOOTH_SHADING
            if (DoesIntersectTriangleInterpolated(
                rayOriginWorld, rayDirWorld,
                vertexWorld0, vertexWorld1, vertexWorld2,
                normalize(mat3(instance.normalModelMatrix) * normal0),
                normalize(mat3(instance.normalModelMatrix) * normal1),
                normalize(mat3(instance.normalModelMatrix) * normal2),
                hitDistance, triangleNormal))
#elif
            if (DoesIntersectTriangle(rayOriginWorld, rayDirWorld, vertexWorld0, 
                                      vertexWorld1, vertexWorld2, hitDistance, triangleNormal))
#endif
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
                vec3 offsetOrigin = candidateHitPos + triangleNormal * EPSILON;

                if (hitDistance > EPSILON && hitDistance < hitDistanceMin)
                {
                    hitIndexV0= indexV0;
                    hitIndexV1 = indexV1;
                    hitIndexV2 = indexV2;

                    hitVertexWorld0 = vertexWorld0;
                    hitVertexWorld1 = vertexWorld1;
                    hitVertexWorld2 = vertexWorld2;

                    hitDistanceMin = hitDistance;
                    hitPos = candidateHitPos;
                    hitNormal = normalize(triangleNormal);
                    hitMaterial = materials[instance.materialIndex];
                    hit = true;
                }
            }
        }
    }

    return hit;
}
bool DoesIntersectSceneWorld(vec3 rayOriginWorld, vec3 rayDirWorld, out vec3 hitPos, out vec3 hitNormal, 
                        out Material hitMaterial, inout uint seed, out uint hitIndexV0, out uint hitIndexV1, 
                        out uint hitIndexV2, out vec3 hitVertexWorld0, out vec3 hitVertexWorld1, out vec3 hitVertexWorld2)
{
#if USE_BVH
    return DoesIntersectSceneWorldBVH(rayOriginWorld, rayDirWorld, hitPos, hitNormal, hitMaterial, seed, hitIndexV0, 
                                      hitIndexV1, hitIndexV2, hitVertexWorld0, hitVertexWorld1, hitVertexWorld2);
#else
    return DoesIntersectSceneWorldNaive(rayOriginWorld, rayDirWorld, hitPos, hitNormal, hitMaterial, seed, hitIndexV0, 
                                      hitIndexV1, hitIndexV2, hitVertexWorld0, hitVertexWorld1, hitVertexWorld2);
#endif
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

float ComputeAmbientOcclusion(vec3 P, vec3 N, float maxDist, uint aoSamples, inout uint seed) 
{
    float unoccluded = 0.0;
    for (int i = 0; i < aoSamples; i++)
    {
        float xi1 = GenerateRandomNum(seed);
        float xi2 = GenerateRandomNum(seed);

        // Cosine-weighted hemisphere sampling (physically correct)
        vec3 L = CosineSampleHemisphere(xi1, xi2, N);

        // Offset to avoid self-intersection
        vec3 origin = P + N * 0.001;

        // Trace shadow ray
        vec3 hp, hn;
        Material hm;
        uint a,b,c;
        vec3 w0,w1,w2;

        if (!DoesIntersectSceneWorld(origin, L, hp, hn, hm, seed, a,b,c,w0,w1,w2))
        {
            // No hit at all means definitely unoccluded
            unoccluded += 1.0;
        }
        else
        {
            float dist = length(hp - origin);
            if (dist >= maxDist)
                unoccluded += 1.0;
        }
    }

    return unoccluded / float(aoSamples);
}

/*
   Computes the bent normal for ambient occlusion. Default normals point perpendicular to surface
    but bent normal will point in the direction where light can reach surface when considering
    all the directions that are blocked by geometry in the hemisphere around a normal
 */
vec3 ComputeBentNormal(vec3 P, vec3 N, float maxDist, uint aoSamples, inout uint seed)
{
    vec3 avgDir = vec3(0);

    for (int i = 0; i < aoSamples; i++)
    {
        float xi1 = GenerateRandomNum(seed);
        float xi2 = GenerateRandomNum(seed);

        vec3 L = CosineSampleHemisphere(xi1, xi2, N);
        vec3 origin = P + N * 0.001;

        vec3 hp, hn;
        Material hm;
        uint a,b,c;
        vec3 w0,w1,w2;

        if (!DoesIntersectSceneWorld(origin, L, hp, hn, hm, seed, a,b,c,w0,w1,w2))
            avgDir += L;
    }

    if (length(avgDir) < 1e-5) return N;
    return normalize(avgDir);
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
    float tanHalfFov = tan(uViewerBlock.yFov * 0.5);
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
        vec3 hitPos, hitNormalWorld;
        Material hitMaterial;
        uint hitIndexV0, hitIndexV1, hitIndexV2;
        vec3 hitWorldV0, hitWorldV1, hitWorldV2;
        if (!DoesIntersectSceneWorld(rayOriginWorld, rayDirWorld, hitPos, hitNormalWorld, hitMaterial, seed, 
            hitIndexV0, hitIndexV1, hitIndexV2, hitWorldV0, hitWorldV1, hitWorldV2)) 
        {
            vec3 sky = vec3(0);
            if (uHasSkybox)
            {
                //sky = SampleEquirectangular(rayDirWorld, uSkybox);
                sky = SampleEquirectangular(rayDirWorld, uSkybox);
                //sky = texture(uSkybox, rayDirWorld).rgb;
            }
            else sky = mix(vec3(0.6, 0.7, 0.9), vec3(0.2, 0.35, 0.6), 0.5 * (jitteredPixelY + 1.0));
            
            //vec3 sky = vec3(0, 0, 0);
            radiance += throughput * sky;
            break;
        }

        if (hitMaterial.emission.a > 0) 
        {
            radiance += throughput * hitMaterial.emission.rgb;
        }
        vec3 albedo = hitMaterial.baseColor.rgb;
        if (hitMaterial.albedoIndex >= 0) 
        {
            // You’ll need the UVs from the hit triangle for texture lookup
            // Interpolate UVs at hit point (requires you to compute barycentrics)
            vec2 uv0 = vertices[hitIndexV0].uvPos;
            vec2 uv1 = vertices[hitIndexV1].uvPos;
            vec2 uv2 = vertices[hitIndexV2].uvPos;

            // Computes barycentric weights of the hit triangle for uv coords in (u, v, w)
            vec3 bary = CalculateBarycentricWeight(hitPos, hitWorldV0, hitWorldV1, hitWorldV2);
            vec2 uv = bary.x * uv0 + bary.y * uv1 + bary.z * uv2;
            albedo = texture(uTextures[hitMaterial.albedoIndex], uv).rgb * hitMaterial.baseColor.rgb * hitMaterial.baseColor.a;
            //vec3 unused = texture(uTextures[hitMaterial.albedoIndex], uv).rgb * hitMaterial.baseColor.rgb * hitMaterial.baseColor.a;
            //albedo = texture(uTextures[hitMaterial.albedoIndex], uv).rgb;
            //if (hitIndexV0 == 0) albedo = vec3(1, 0, 0);
            //else albedo = vec3(0, 1, 0);
            //albedo = vec3(hitIndexV0, hitIndexV1, hitIndexV2);
        }
        
        //-----------------------------------------------------------------------------------
        //                          LIGHTING SECTION
        //-----------------------------------------------------------------------------------
        // Environment / ambient lighting section
        float ambientOcclusion = 1;

#if DO_AMBIENT_OCCLUSION
        ambientOcclusion = ComputeAmbientOcclusion(hitPos, hitNormalWorld, 1.0, uAmbientOcclusionSamples, seed);
#endif

        vec3 envLight = vec3(0.0);
        if (uHasSkybox)
        {
            vec3 lightingSurfaceNormal = hitNormalWorld;
#if DO_AMBIENT_OCCLUSION && USE_BENT_NORMAL_FOR_LIGHTING
            lightingSurfaceNormal = ComputeBentNormal(hitPos, hitNormalWorld, 1.0, uAmbientOcclusionSamples, seed);
#endif
            envLight = SampleEquirectangular(normalize(lightingSurfaceNormal), uSkybox);
        }
        vec3 diffuseEnv = albedo * envLight * ambientOcclusion;
        radiance += throughput * diffuseEnv;

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
        
        uint triCount = lightMeshInstance.numIndices / 3u;
        vec3 v0w = vec3(lightInstance.modelMatrix * vec4(vertices[lightIndexV0].localPos, 1.0));
        vec3 v1w = vec3(lightInstance.modelMatrix * vec4(vertices[lightIndexV1].localPos, 1.0));
        vec3 v2w = vec3(lightInstance.modelMatrix * vec4(vertices[lightIndexV2].localPos, 1.0));
        // Here we find the sample of the light using the random triangle we choose above
        vec3 lightSampleWorld = SamplePointOnTriangle(v0w, v1w, v2w, seed);

        // We use world vertex triangle positions to get triangle area and world normal
        vec3 e1 = v1w - v0w;
        vec3 e2 = v2w - v0w;
        vec3 lightNormalWorld = normalize(cross(e1, e2));
        float triArea = 0.5 * length(cross(e1, e2)) + EPSILON;

        // direction from hit point toward light (correct sign)
        vec3 lightDir = normalize(lightSampleWorld - hitPos);
        float hitDistanceToLight = length(lightSampleWorld - hitPos);

        float NdotL = max(0.0, dot(hitNormalWorld, lightDir));
        float NlDot = max(0.0, dot(lightNormalWorld, -lightDir));

        //NOTE: the following are only for required scene intersection test out params and are not used for anything
        uint dummyIndex0, dummyIndex1, dummyIndex2;
        vec3 dummyV0, dummyV1, dummyV2;
        if (NdotL > 0.0 && NlDot > 0.0)
        {
            // pdf for: uniform emissive instance * uniform triangle index * uniform point on triangle
            float pdf_point = (1.0 / max(1.0, float(uEmissiveMaterialCount))) *
                                (1.0 / max(1.0, float(triCount))) *
                                (1.0 / triArea);

            // geometry term (including 1/r^2)
            float G = (NdotL * NlDot) / max(EPSILON, hitDistanceToLight * hitDistanceToLight);

            // light emission (material emission) scaled by multiplier uniform
            vec3 Le = materials[lightInstance.materialIndex].emission.rgb;

            vec3 outSpec;
            float dummyPdf;
            vec3 brdf = EvaluateMicrofacetBRDF(hitNormalWorld, -rayDirWorld, lightDir, albedo, hitMaterial.metallic, hitMaterial.roughness, outSpec, dummyPdf);

            // Monte Carlo estimator: Le * G * (area / pdf_point) * BRDF * cos term
            // NOTE: our BRDF evaluate already includes cosine multiplication when used for energy. We include NdotL here for correctness
            float weight = triArea / max(EPSILON, pdf_point);
            vec3 direct = Le * G * weight * brdf;

            //If the object we hit is affected by a light (meaning there are no objects blocking the path from a random 
            //light to this object), we can then add that light's intensity/color to ray -> gains energy
            //NOTE: it does not matter that we choose a random light because due to accumulation, this will eventually fill out all lights
            vec3 shadowOrigin = hitPos + hitNormalWorld * EPSILON;
            vec3 shadowHitPos, shadowHitNormal;
            Material shadowHitMaterial;
            bool blocked = DoesIntersectSceneWorld(shadowOrigin, lightDir, shadowHitPos, shadowHitNormal, shadowHitMaterial, seed, 
                                                    dummyIndex0, dummyIndex1, dummyIndex2, dummyV0, dummyV1, dummyV2);

            if (!blocked || length(shadowHitPos - hitPos) > hitDistanceToLight - 0.001)
                radiance += throughput * direct;
            //NOTE: we apply a small amount of ambient light to ensure objects with no light do not look flat
            else
            {
                vec3 ambientLight = vec3(0, 0, 0);
                radiance += throughput * ambientLight * ambientOcclusion;
            }
        }

        float metallic = clamp(hitMaterial.metallic, 0.0, 1.0);
        float roughness = clamp(hitMaterial.roughness, 0.02, 1.0);
        // ----- Directional light contribution -----
        {
            vec3 L = normalize(-uLightsBlock.directionalDir); // from surface toward directional light
            float NdotL = max(dot(hitNormalWorld, L), 0.0);
            if (NdotL > 0.0)
            {
                vec3 shadowOrigin = hitPos + hitNormalWorld * EPSILON;
                vec3 shadowHitPos, shadowHitNormal;
                Material shadowHitMaterial;
                bool blocked = DoesIntersectSceneWorld(shadowOrigin, L, shadowHitPos, shadowHitNormal, shadowHitMaterial, seed, 
                                                        dummyIndex0, dummyIndex1, dummyIndex2, dummyV0, dummyV1, dummyV2);

                if (!blocked) 
                {
                    vec3 spec;
                    float dummyPdf;
                    //NOTE: we have to use the hitMaterial NOT the shadow hit material (which is the material in the way blocking ray)
                    vec3 brdf = EvaluateMicrofacetBRDF(hitNormalWorld, -rayDirWorld, L, albedo, metallic, roughness, spec, dummyPdf);
                    radiance += throughput * uLightsBlock.directionalColor.rgb * uLightsBlock.directionalColor.a * brdf * NdotL;
                } 
            }
        }

        // --- BSDF sampling for next bounce (mixture specular/diffuse) ---
        // compute F0 and albedo
        
        vec3 F0 = mix(vec3(0.04), albedo, metallic);

        // compute a simple energy-based probability to sample specular vs diffuse
        float avgF0 = (F0.r + F0.g + F0.b) / 3.0;
        float specularProb = clamp(avgF0 + (1.0 - roughness) * 0.5, 0.0, 1.0); // metals & low roughness bias specular
        float diffuseProb = 1.0 - specularProb;

        // random choice
        float chooser = GenerateRandomNum(seed);

        vec3 nextDir;
        float pdf = 1.0;
        vec3 brdfValue = vec3(0.0);

        vec3 V = normalize(-rayDirWorld); // view direction pointing toward viewer
        vec3 N = hitNormalWorld;

        //This branche calculates specular/reflections
        if (chooser < specularProb) 
        {
            // sample microfacet specular lobe via GGX half-vector sampling
            float Xi1 = GenerateRandomNum(seed);
            float Xi2 = GenerateRandomNum(seed);
            float alpha = roughness * roughness;
            vec3 H = ImportanceSampleGGX(Xi1, Xi2, N, alpha);

            // reflect V about H to get outgoing L
            nextDir = normalize(2.0 * dot(V, H) * H - V);
            // ensure nextDir is in the hemisphere
            if (dot(nextDir, N) <= 0.0) 
            {
                // fallback to cosine hemisphere
                float x1 = GenerateRandomNum(seed);
                float x2 = GenerateRandomNum(seed);
                nextDir = CosineSampleHemisphere(x1, x2, N);
            }

            // Evaluate BRDF and PDF for this sample
            float NdotL = max(dot(N, nextDir), 0.0);
            float NdotH = max(dot(N, H), 0.0);
            float VdotH = max(dot(V, H), 0.0);

            // Evaluate microfacet BRDF components
            vec3 specular;
            float tmpPdf;
            brdfValue = EvaluateMicrofacetBRDF(N, V, nextDir, albedo, metallic, roughness, specular, tmpPdf);

            // pdf for sampling this L via H sampling:
            float pdf_spec = PDF_GGX(NdotH, alpha, VdotH);
            // account for mixture probability selection:
            pdf = specularProb * pdf_spec + diffuseProb * PDF_CosineHemisphere(max(dot(N, nextDir), 0.0));
        } 
        //This handles the diffuse part (indirect bounces so no specular)
        else 
        {
            // cosine-weighted sample for diffuse
            float x1 = GenerateRandomNum(seed);
            float x2 = GenerateRandomNum(seed);
            nextDir = CosineSampleHemisphere(x1, x2, N);

            // evaluate BRDF and pdf for diffuse sample
            vec3 specular;
            float tmpPdf;
            brdfValue = EvaluateMicrofacetBRDF(N, V, nextDir, albedo, metallic, roughness, specular, tmpPdf);

            float NdotL = max(dot(N, nextDir), 0.0);
            pdf = diffuseProb * PDF_CosineHemisphere(NdotL) + specularProb * EPSILON;
        }

        if (pdf < EPSILON) 
            break;

        // Update throughput using rendering equation: throughput *= (f * cos / pdf)
        float cosTheta = max(dot(N, nextDir), 0.0);
        throughput *= brdfValue * cosTheta / pdf;

        rayDirWorld = nextDir;
        rayOriginWorld = hitPos + hitNormalWorld * EPSILON;

        // Russian roulette termination:
        // if the max rgb channel with energy left is surpassed by random number
        // then we exit
        float maxThroughput = max(max(throughput.r, throughput.g), throughput.b);
        if (GenerateRandomNum(seed) > maxThroughput) 
            break;

        throughput /= max(maxThroughput, EPSILON);
    }

    // Progressive accumulation using texture input
    vec4 prev = (uUnmovingFrameCount == 0u) ? vec4(0.0) : imageLoad(uTextureInput, pixel);
    vec3 blended = (prev.rgb * float(uUnmovingFrameCount) + radiance) / float(uUnmovingFrameCount + 1u);
    vec4 fragColor= vec4(blended, 1.0);

    //imageStore(uTextureOutput, pixel, vec4(1.0, 0, 0, 1.0));
    //TODO: transparency is not supported yet
    imageStore(uTextureOutput, pixel, fragColor);
    imageStore(uTextureInput, pixel, fragColor);

    float luminance = dot(fragColor, vec4(0.2126, 0.7152, 0.0722, 1.0));
    imageStore(uBrightnessTexture, pixel, luminance >= uBloomThreshold ? fragColor : vec4(0.0, 0.0, 0.0, 1.0));
}