#version 430 core

//Applying small offsets to initial pixel location for ray to prevent jagged edges to geometry
#define DO_PIXEL_JITTERING 1
//Smooth shading will interpolate normals from vertices when tracing
#define DO_SMOOTH_SHADING 1
#define USE_BVH 1
//Will use the modified normal for environment lighting
#define USE_BENT_NORMAL_FOR_LIGHTING 1
//Will add lighting from the skybox onto the scene
#define ADD_SKYBOX_LIGHTING 0

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
uniform uint uUnmovingFrameCount;
uniform uint uEmissiveCount;
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
const float EPSILON_DET = EPSILON;
const float EPSILON_BARY = 1e-6;
const float EPSILON_T = 1e-5;

const float PI = 3.14159265359;
const uint MAX_STACK_SIZE = 128;

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

void EvaluateBSDF(vec3 normal, vec3 V, vec3 lightDir, vec3 albedo, float metallic, float roughness, out vec3 f, 
    out float diffusePdf, out float specularPdf, out float mixedPdf) {
    f = vec3(0.0);
    diffusePdf = 0.0;
    specularPdf = 0.0;

    float NdotL = max(dot(normal, lightDir), 0.0);
    float NdotV = max(dot(normal, V), 0.0);
    if (NdotL <= 0.0 || NdotV <= 0.0)
        return;

    vec3 H = normalize(V + lightDir);
    float NdotH = max(dot(normal, H), 0.0);
    float VdotH = max(dot(V, H), 0.0);

    float alpha = roughness * roughness;
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    float D = NormalDistributionGGX(NdotH, alpha);
    float k = (alpha + 1.0) * (alpha + 1.0) / 8.0;
    float G = GeometrySmith(NdotV, NdotL, k);
    vec3 F = FresnelSchlickReflectance(VdotH, F0);

    vec3 specular = (D * G * F) / max(4.0 * NdotV * NdotL, EPSILON);

    vec3 kd = (1.0 - F) * (1.0 - metallic);
    vec3 diffuse = kd * albedo / PI;

    f = diffuse + specular;
    diffusePdf = NdotL / PI;
    specularPdf = PDF_GGX(NdotH, alpha, VdotH);

    float specularWeight = clamp(max(F0.r, max(F0.g, F0.b)), 0.05, 0.95);
    mixedPdf= specularWeight * specularPdf + (1.0 - specularWeight) * diffusePdf;
}

void SampleBSDF(inout uint seed, vec3 normal, vec3 V, vec3 albedo, float metallic, float roughness, out vec3 L, out vec3 f, out float pdf) 
{
    float alpha = roughness * roughness;

    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    float specularWeight = clamp(max(F0.r, max(F0.g, F0.b)), 0.05, 0.95);
    bool chooseSpecular =  GenerateRandomNum(seed) < specularWeight;

    //Here we choose to do specular for pdf
    if (chooseSpecular) 
    {
        vec3 H = ImportanceSampleGGX(GenerateRandomNum(seed), 
                 GenerateRandomNum(seed), normal, roughness * roughness);
        L = reflect(-V, H);
    } 
    //Here we do diffuse pdf
    else 
    {
        L = CosineSampleHemisphere(GenerateRandomNum(seed), GenerateRandomNum(seed), normal);
    }

    float specularPdf = 0;
    float diffusePdf = 0;
    float mixedPdf = 0;
    EvaluateBSDF(normal, V, L, albedo, metallic, roughness, f, diffusePdf, specularPdf, mixedPdf);
    pdf = mixedPdf;
}

/* Moller–Trumbore ray–triangle intersection
 * Computes whether a ray at origin and dir hits a triangle defined by 3 vertices
 * where hitDistance is the scalar distance from ray origin along ray dir to the intersection point of the triangle
 * and triangle normal is the normal of the triangle
 */
bool DoesIntersectTriangle(vec3 rayOrigin, vec3 rayDir, vec3 v0, vec3 v1, vec3 v2, out float tEnter, out vec3 triangleNormal, out int flag)
{
    vec3 rayDirNormalized = normalize(rayDir);

    vec3 edge1 = v1 - v0;
    vec3 edge2 = v2 - v0;
    //triangleNormal = normalize(cross(e1, e2));
    triangleNormal = normalize(cross(edge1, edge2));

    vec3 rayCrossE2 = cross(rayDirNormalized, edge2);
    float det = dot(edge1, rayCrossE2);
    //The ray is parallel to the triangle
    if (abs(det) < EPSILON_DET) 
    {
        flag = 1;
        return false;
    }

    float invDet = 1.0 / det;
    vec3 s = rayOrigin - v0;
    float u = dot(s, rayCrossE2) * invDet;
    if (u < -EPSILON_BARY || u > 1.0 + EPSILON_BARY)
    {
        flag = 2;
        return false;
    }

    vec3 sCrossE1 = cross(s, edge1);
    float v = dot(rayDirNormalized, sCrossE1) * invDet;
    if (v < -EPSILON_BARY || u + v > 1.0 + EPSILON_BARY)
    {
        flag = 3;
        return false;
    }

    tEnter = dot(edge2, sCrossE1) * invDet;
    if (tEnter < EPSILON_T) 
    {
        flag = 4;
        return false;
    }

    tEnter /= length(rayDir);
    flag = 0;
    return true;
}

/* Same as DoesIntersectTriangle, but instead uses barycentric weights to interpolate the out triangle normal
 * using the corresponding vertex normals
 */
bool DoesIntersectTriangleInterpolated(vec3 rayWorldOrigin, vec3 rayWorldDir, vec3 v0, vec3 v1, vec3 v2,
    vec3 n0, vec3 n1, vec3 n2, out float tEnter, out vec3 triangleNormal)
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

    tEnter = dot(e2, qvec) * invDet;
    if (tEnter < EPSILON) 
        return false;

    // Compute barycentric coordinates
    float w = 1.0 - u - v;

    // Interpolate vertex normals
    triangleNormal = normalize(w * n0 + u * n1 + v * n2);

    return true;
}

/*
 * Slab intersection test
 */
bool DoesIntersectBounds(vec3 bounds[2], vec3 rayOrigin, vec3 inverseRayDir, ivec3 rayDirSign, 
                         out float outTEnter, out float outTExit)
{
    float tMin  = (bounds[rayDirSign.x].x - rayOrigin.x) * inverseRayDir.x;
    float tMax  = (bounds[1 - rayDirSign.x].x - rayOrigin.x) * inverseRayDir.x;
    //If bounds behind the ray we early exit
    if (tMax < 0.0) 
        return false;

    float tMinY = (bounds[rayDirSign.y].y - rayOrigin.y) * inverseRayDir.y;
    float tMaxY = (bounds[1 - rayDirSign.y].y - rayOrigin.y) * inverseRayDir.y;

    if (tMin > tMaxY || tMinY > tMax)
        return false;

    // Merge Y slab
    if (tMinY > tMin) tMin = tMinY;
    if (tMaxY < tMax) tMax = tMaxY;

    float tMinZ = (bounds[rayDirSign.z].z - rayOrigin.z) * inverseRayDir.z;
    float tMaxZ = (bounds[1 - rayDirSign.z].z - rayOrigin.z) * inverseRayDir.z;

    if (tMin > tMaxZ || tMinZ > tMax)
        return false;

    // Merge Z slab
    if (tMinZ > tMin) tMin = tMinZ;
    if (tMaxZ < tMax) tMax = tMaxZ;

    outTEnter = tMin;
    outTExit = tMax;
    return true;
}

bool IsWithinBounds(vec3 bounds[2], vec3 pos, out float pastMax)
{
    if (pos.x > bounds[1].x || pos.y > bounds[1].y || pos.z > bounds[1].z)
    {
        pastMax = max(max(max(0, pos.x - bounds[1].x), pos.y- bounds[1].y), pos.z- bounds[1].z);
        return false;
    }
    if (pos.x < bounds[0].x || pos.y < bounds[0].y || pos.z < bounds[0].z)
    {
        pastMax = max(max(max(0, bounds[0].x - pos.x), bounds[0].y - pos.y), bounds[0].z - pos.z);
        return false;
    }
    return true;
}

bool IsFullyOutsideBounds(vec3 bounds[2], vec3 pos)
{
    if (pos.x > bounds[1].x && pos.y > bounds[1].y && pos.z > bounds[1].z &&
        pos.x > bounds[0].x && pos.y > bounds[0].y && pos.z > bounds[0].z)
        return true;

    if (pos.x < bounds[1].x && pos.y < bounds[1].y && pos.z < bounds[1].z &&
        pos.x < bounds[0].x && pos.y < bounds[0].y && pos.z < bounds[0].z)
        return true;

    return false;
}

float CalculateParallelogramArea(vec3 v0, vec3 v1, vec3 v2)
{
    return length(cross(v1- v0, v2- v0));
}
float CalculateTriangleArea(vec3 v0, vec3 v1, vec3 v2)
{
    return 0.5f * CalculateParallelogramArea(v0, v1, v2);
}

/*
   Calculates the weight of the targetPos based on the 3 positions
   in normalized position where (1, 0, 0) would be barycentric weight of
   v0; (0, 1, 0) of v1; (0, 0, 1) of v2
*/
vec3 CalculateBarycentricWeight(vec3 targetPos, vec3 v0, vec3 v1, vec3 v2)
{
    /* 
     * Since 0.5 * cross product(v0, v1) is the area of a triangle 
     * we can use that to determine how big the area of the triangle formed between the targetPos
     * and two adjacent vertices in relation to the FULL AREA to get ratios of how close
     * a point is (closer to vertices -> smaller triangle area -> dividing smaller value -> greater fraction)
     */

    //(NOTE: we use parallogram area (just doing cross(v0, v1, v2)) because we use ratios of small area over full area
    //the 0.5 would cancel out
    float area = CalculateParallelogramArea(v0, v1, v2);
    float a0 = length(cross(v1 - targetPos, v2 - targetPos)) / area;
    float a1 = length(cross(v2 - targetPos, v0 - targetPos)) / area;
    //NOTE: we skip the last area because we know all 3 values must equal 1
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

bool DoesIntersectLocalObjectBVH(vec3 rayOriginLocal, vec3 rayDirLocal, Instance instance, out vec3 hitPosWorld, out vec3 hitNormalWorld, 
                        out Material hitMaterial, inout uint seed, out uint hitIndexV0, out uint hitIndexV1, out uint hitIndexV2, 
                        out vec3 hitVertexWorld0, out vec3 hitVertexWorld1, out vec3 hitVertexWorld2)
{
    vec3 inverseLocalRayDir = 1.0 / max(abs(rayDirLocal), vec3(1e-8)) * sign(rayDirLocal);
    ivec3 localRayDirSign = ivec3(lessThan(inverseLocalRayDir, vec3(0.0)));
	int stack[MAX_STACK_SIZE];
    int stackPtr = 0;

    InstanceMesh mesh = meshes[instance.meshIndex];
    int startNodeIndex = int(mesh.blasTreeOffset);
    //NOTE: we only need the first node to be offset since all tree child indices
    //should be adjusted to be in terms of the full node array
	stack[stackPtr++] = startNodeIndex;

	BVHNode node;
	float tEnter = 0, tExit = 0;
    float leafTEnter = 0;
    float tEnterChild0 = 0, tExitChild0 = 0, tEnterChild1 = 0, tExitChild1 = 0;
	bool hit = false;
    float localMinTEnter = 1e20;
    vec3 localHitNormal;
	while (stackPtr > 0)
	{
		node = blasTrees[stack[--stackPtr]];
        if (!DoesIntersectBounds(node.bounds, rayOriginLocal, inverseLocalRayDir, localRayDirSign, tEnter, tExit))
			continue;

		if (tEnter > localMinTEnter)
	        continue;

		if (IsLeaf(node))
		{
			for (int i = 0; i < node.objectCount; i++)
			{
                //NOTE: since the object indices are in terms of TRIANGLES, we must multiply by
                //3 to get the corresponding index
				uint indexV0 = indices[(node.objectStartIndex + i) * 3];
                uint indexV1 = indices[(node.objectStartIndex + i) * 3 + 1];
                uint indexV2 = indices[(node.objectStartIndex + i) * 3 + 2];

                vec3 triangleNormal;

#if DO_SMOOTH_SHADING
                if (DoesIntersectTriangleInterpolated(
                    rayOriginLocal, rayDirLocal,
                    vertices[indexV0].localPos, vertices[indexV1].localPos, vertices[indexV2].localPos,
                    vertices[indexV0].normal, vertices[indexV1].normal, vertices[indexV2].normal,
                    leafTEnter, triangleNormal))
#else
                if (DoesIntersectTriangle(rayOriginLocal, rayDirLocal, vertices[indexV0].localPos, 
                    vertices[indexV1].localPos, vertices[indexV2].localPos, leafTEnter, triangleNormal, currFlag))
#endif
                {
                    //If the dir and normal > 0 -> same dir and thus 
                    //it means triangle is a backface and should be ignored
                    if (dot(rayDirLocal, triangleNormal) > 0.0)
                        continue;

                    if (leafTEnter > EPSILON && leafTEnter < localMinTEnter)
                    {
                        hitIndexV0 = indexV0;
                        hitIndexV1 = indexV1;
                        hitIndexV2 = indexV2;

                        localMinTEnter = leafTEnter;
                        localHitNormal = triangleNormal;
                        hitMaterial = materials[instance.materialIndex];
                        hit = true;
                    }
                }
            }
		}
		else
		{
			bool minHitChild0 = DoesIntersectBounds(blasTrees[node.indexChild0].bounds, rayOriginLocal, inverseLocalRayDir, 
                                                    localRayDirSign, tEnterChild0, tExitChild0) && tEnterChild0 <= localMinTEnter;
            bool minHitChild1 = DoesIntersectBounds(blasTrees[node.indexChild1].bounds, rayOriginLocal, inverseLocalRayDir, 
                                                    localRayDirSign, tEnterChild1, tExitChild1) && tEnterChild1 <= localMinTEnter;

			if (minHitChild0 && minHitChild1)
			{
				if (tEnterChild0 < tEnterChild1)
				{
					stack[stackPtr++] = node.indexChild1;
					stack[stackPtr++] = node.indexChild0;
				}
				else
				{
					stack[stackPtr++] = node.indexChild0;
					stack[stackPtr++] = node.indexChild1;
				}
			}
			else if (minHitChild0)
				stack[stackPtr++] = node.indexChild0;
			else if (minHitChild1)
				stack[stackPtr++] = node.indexChild1;
		}
	}

    if (hit)
    {
        hitVertexWorld0 = vec3(instance.modelMatrix * vec4(vertices[hitIndexV0].localPos, 1.0));
        hitVertexWorld1 = vec3(instance.modelMatrix * vec4(vertices[hitIndexV1].localPos, 1.0));
        hitVertexWorld2 = vec3(instance.modelMatrix * vec4(vertices[hitIndexV2].localPos, 1.0));

        hitNormalWorld = normalize(instance.normalModelMatrix * localHitNormal);
        hitPosWorld = (instance.modelMatrix * vec4(rayOriginLocal + rayDirLocal * localMinTEnter, 1)).xyz;
    }
	return hit;
}

bool DoesIntersectSceneWorldBVH(vec3 rayOriginWorld, vec3 rayDirWorld, out vec3 hitPos, out vec3 hitWorldNormal, 
                        out Material hitMaterial, inout uint seed, out uint hitIndexV0, out uint hitIndexV1, out uint hitIndexV2, 
                        out vec3 hitVertexWorld0, out vec3 hitVertexWorld1, out vec3 hitVertexWorld2)
{
    vec3 inverseWorldRayDir =  1.0 / max(abs(rayDirWorld), vec3(1e-8)) * sign(rayDirWorld);
    ivec3 worldRayDirSign = ivec3(lessThan(inverseWorldRayDir, vec3(0.0)));
	int stack[MAX_STACK_SIZE];
    int stackPtr = 0;

	stack[stackPtr++] = 0;

	BVHNode node;
	float tEnter = 0, tExit = 0;
    float tEnterChild0 = 0, tExitChild0 = 0, tEnterChild1 = 0, tExitChild1 = 0;
    float hitDistance = 0, minHitDistance = 1e20;
    vec3 rayOriginLocal, rayDirLocal;
	bool hit = false;

	while (stackPtr > 0)
	{
		node = tlasTree[stack[--stackPtr]];
        if (!DoesIntersectBounds(node.bounds, rayOriginWorld, inverseWorldRayDir, worldRayDirSign, tEnter, tExit))
			continue;

        //TODO: check all occurences of DoesIntersectSceneWorld and if thye all use normalized ray dir, 
        //we can directly compare tEnter and minHitDistance
		if (length(tEnter * rayDirWorld) > minHitDistance)
            continue;

		if (IsLeaf(node))
		{
            //hitLeaf = max(0, hitLeaf);
            //NOTE: for the tlas tree, the objects are indices into the Instance buffer
			for (int i = 0; i < node.objectCount; i++)
			{
				Instance instance = instances[node.objectStartIndex + i];
                rayOriginLocal = vec3(instance.inverseModelMatrix * vec4(rayOriginWorld, 1.0));
                rayDirLocal = normalize(vec3(instance.inverseModelMatrix * vec4(rayDirWorld, 0.0)));
                vec3 thisHitPosWorld;

                if (DoesIntersectLocalObjectBVH(rayOriginLocal, rayDirLocal, instance, thisHitPosWorld, hitWorldNormal, hitMaterial, seed, 
                                                    hitIndexV0, hitIndexV1, hitIndexV2, hitVertexWorld0, hitVertexWorld1, hitVertexWorld2))
                {
                    float thisMinHitDistanceWorld = length(thisHitPosWorld - rayOriginWorld);
                    if (thisMinHitDistanceWorld < minHitDistance)
                    {
                        minHitDistance= thisMinHitDistanceWorld;
                        hitPos = thisHitPosWorld;
                        hit = true;
                    }
                }
            }
		}
		else
		{
			bool minHitChild0 = DoesIntersectBounds(tlasTree[node.indexChild0].bounds, rayOriginWorld, inverseWorldRayDir, 
                                                    worldRayDirSign, tEnterChild0, tExitChild0) && length(tEnterChild0 * rayDirWorld) <= minHitDistance;
            bool minHitChild1 = DoesIntersectBounds(tlasTree[node.indexChild1].bounds, rayOriginWorld, inverseWorldRayDir, 
                                                    worldRayDirSign, tEnterChild1, tExitChild1) && length(tEnterChild1 * rayDirWorld) <= minHitDistance;

			if (minHitChild0 && minHitChild1)
			{
				if (tEnterChild0 < tEnterChild1)
				{
					stack[stackPtr++] = node.indexChild1;
					stack[stackPtr++] = node.indexChild0;
				}
				else
				{
					stack[stackPtr++] = node.indexChild0;
					stack[stackPtr++] = node.indexChild1;
				}
			}
			else if (minHitChild0)
				stack[stackPtr++] = node.indexChild0;
			else if (minHitChild1)
				stack[stackPtr++] = node.indexChild1;
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
    for (uint instanceIndex = 0u; instanceIndex < 4; instanceIndex++)
    {
        Instance instance = instances[instanceIndex];
        InstanceMesh mesh = meshes[instanceIndex];

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
#else
            float dummyFlag = 0;
            if (DoesIntersectTriangle(rayOriginWorld, rayDirWorld, vertexWorld0, 
                                      vertexWorld1, vertexWorld2, hitDistance, triangleNormal, dummyFlag))
#endif
            {
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


vec3 SampleRandomTrianglePoint(vec3 v0, vec3 v1, vec3 v2, inout uint seed) 
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

    //NOTE: these are only placeholders to keep blas/tlas uniforms when BVH use is FALSE
    float num = tlasTree[0].bounds[0].x;
    float num2 = blasTrees[0].bounds[0].x;
  

    float aspectRatio = float(uScreenSize.x) / float(uScreenSize.y);
    float scale = tan(uViewerBlock.yFov * 0.5);
    
#if DO_PIXEL_JITTERING
    //The following apply a small <1 jitter to the pixel coordinate in order to prevent aliasing (jagged edges)
    //If we always used the center of the pixel it would not appear smooth, so by doing this combined with accumulation
    //we get a nicer more-filled and less jagged look to edges
    float normalizedPixelX = (float(pixel.x) + GenerateRandomNum(seed)) / float(uScreenSize.x);
    float normalizedPixelY = (float(pixel.y) + GenerateRandomNum(seed)) / float(uScreenSize.y);
#else
    //Normalized pixels are [0, 1] of screen size
    float normalizedPixelX = pixel.x / float(uScreenSize.x);
    float normalizedPixelY = pixel.y / float(uScreenSize.y);
#endif

    //This is the jittered pixel coord in normalized device coordinate pos [-1, 1] 
    //(horizontal and vertical offset from center of screen)
    vec2 ndcPos = vec2(normalizedPixelX * 2.0 - 1.0, normalizedPixelY * 2.0 - 1.0);
    //NOTE: this is a shortcut for doing inverse projection and inverse view matrix multiplication
    //ASSUMING frustum is symmetric (meaning no offset between camera center and near plane rectangle center)
    vec3 rayDirWorld = normalize(uViewerBlock.forwardDir + uViewerBlock.rightDir * (ndcPos.x * aspectRatio * scale) 
                                 + uViewerBlock.upDir * (ndcPos.y * scale));
    vec3 rayOriginWorld = uViewerBlock.worldPos;

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
                sky = SampleEquirectangular(rayDirWorld, uSkybox);
            }
            
            radiance += throughput * sky;
            break;
        }

        vec3 albedo = hitMaterial.baseColor.rgb * hitMaterial.baseColor.a;
        if (hitMaterial.albedoIndex >= 0) 
        {
            vec2 uvEdge0 = vertices[hitIndexV0].uvPos;
            vec2 uvEdge1 = vertices[hitIndexV1].uvPos;
            vec2 uvEdge2 = vertices[hitIndexV2].uvPos;

            //We compute barycentric weights of the hit triangle for uv coords in (u, v, w) so
            //we get accurate texture coords at the hit point
            vec3 baryWeights = CalculateBarycentricWeight(hitPos, hitWorldV0, hitWorldV1, hitWorldV2);
            vec2 uv = baryWeights.x * uvEdge0 + baryWeights.y * uvEdge1 + baryWeights.z * uvEdge2;

            albedo *= texture(uTextures[hitMaterial.albedoIndex], uv).rgb;
        }
        float metallic = clamp(hitMaterial.metallic, 0.0, 1.0);
        float roughness = clamp(hitMaterial.roughness, 0.02, 1.0);

        rayOriginWorld = hitPos + hitNormalWorld * EPSILON;
        vec3 reflectedRayDirWorld = -rayDirWorld;

        //--------------------------------------------------------------------------------------------------
        //                                  DIRECT LIGHTING (NEXT EVENT ESTIMATION)
        //--------------------------------------------------------------------------------------------------
        {
            // --------------------------------- Stochastic light sampling ----------------------------------
            // Here we pick a random triangle on the light to see if hit object gets affected by this light
            // NOTE: because we assume every 3 is a triangle, we have to divide by 3 to find the triangle index
            // and we multiple by 3 to convert the triangle index to a vertex index
            uint randomLightInstanceIndex = lightIndices[uint(GenerateRandomNum(seed) * float(uEmissiveCount))];
            Instance lightInstance= instances[randomLightInstanceIndex];
            InstanceMesh lightMeshInstance = meshes[randomLightInstanceIndex];
            uint randomTriangle = uint(GenerateRandomNum(seed) * float(lightMeshInstance.numIndices / 3));
            uint baseIndex = lightMeshInstance.indexOffset + randomTriangle * 3u;
            uint lightIndexV0 = indices[baseIndex + 0];
            uint lightIndexV1 = indices[baseIndex + 1];
            uint lightIndexV2 = indices[baseIndex + 2];
        
            vec3 lightVertex0 = vec3(lightInstance.modelMatrix * vec4(vertices[lightIndexV0].localPos, 1.0));
            vec3 lightVertex1 = vec3(lightInstance.modelMatrix * vec4(vertices[lightIndexV1].localPos, 1.0));
            vec3 lightVertex2 = vec3(lightInstance.modelMatrix * vec4(vertices[lightIndexV2].localPos, 1.0));
            // Here we find the sample of the light using the random triangle we choose above
            vec3 randomLightTriangleWorldPoint = SampleRandomTrianglePoint(lightVertex0, lightVertex1, lightVertex2, seed);
            float lightTriangleArea = CalculateTriangleArea(lightVertex0, lightVertex1, lightVertex2);
            vec3 lightNormalWorld = normalize(cross(lightVertex1 - lightVertex0, lightVertex2 - lightVertex0));

            vec3 lightVec = randomLightTriangleWorldPoint - hitPos;
            float hitDistanceToLight = length(lightVec);
            vec3 lightDir = lightVec / hitDistanceToLight;

            float NdotL = max(0.0, dot(hitNormalWorld, lightDir));
            float NlDot = max(0.0, dot(lightNormalWorld, -lightDir));

            vec3 shadowHitPos, shadowHitNormal;
            Material shadowHitMaterial;
            uint dummyIndex0, dummyIndex1, dummyIndex2;
            vec3 dummyV0, dummyV1, dummyV2;
            if (NdotL > EPSILON && NlDot > EPSILON)
            {
                bool occluded = DoesIntersectSceneWorld(rayOriginWorld, lightDir, shadowHitPos, shadowHitNormal, shadowHitMaterial, seed, 
                                                        dummyIndex0, dummyIndex1, dummyIndex2, dummyV0, dummyV1, dummyV2)
                                && length(shadowHitPos - rayOriginWorld) < hitDistanceToLight - EPSILON;
                if (!occluded)
                {
                    float lightTrianglePdfArea = 1.0 / (lightTriangleArea * float(uEmissiveCount));
                    float lightPdf = lightTrianglePdfArea * hitDistanceToLight * hitDistanceToLight / max(NlDot, EPSILON);

                    vec3 thisF = vec3(0);
                    float pdfDiffuse = 0;
                    float pdfSpecular = 0;
                    float mixedBsdfPdf =0;
                    EvaluateBSDF(hitNormalWorld, normalize(-rayDirWorld), lightDir, albedo, metallic, roughness, thisF, pdfDiffuse, pdfSpecular, mixedBsdfPdf);

                    // MIS power heuristic (more stable that balance heuristic)
                    float w = (lightPdf * lightPdf) / (lightPdf * lightPdf + mixedBsdfPdf * mixedBsdfPdf);
                    vec4 materialEmission = materials[lightInstance.materialIndex].emission;
                    vec3 lightRadiance = materialEmission.rgb * materialEmission.a;

                    radiance += throughput * thisF * lightRadiance * NdotL * w / lightPdf;
                }
            }

            // ----------------------------------- POINT LIGHTS ------------------------------
            // In physics based rendering we can not just give point lights an area and glow, 
            // we must do the same process as sampling an emissive texture and this also
            // means the point light emits light in all directions infinitely with a cutoff function
            for (int i=0; i<uLightsBlock.pointLightsCount; i++)
            {
                PointLight light = uLightsBlock.pointLights[i];
                lightVec = light.position - hitPos;
                hitDistanceToLight = length(lightVec);
                lightDir = lightVec / hitDistanceToLight;
                float NdotL = max(0.0, dot(hitNormalWorld, lightDir));

                if (NdotL <= EPSILON)
                    continue;
                
                bool occluded = DoesIntersectSceneWorld(rayOriginWorld, lightDir, shadowHitPos, shadowHitNormal, shadowHitMaterial, seed, 
                                                        dummyIndex0, dummyIndex1, dummyIndex2, dummyV0, dummyV1, dummyV2)
                                    && length(shadowHitPos - rayOriginWorld) < hitDistanceToLight - EPSILON;
                if (!occluded)
                {
                    vec3 lightIntensity = light.color.rgb * light.color.a * clamp(1 - (hitDistanceToLight / light.radius), 0.0f, 1.0f);
                    if (length(lightIntensity) < 1e-5)
                        continue;

                    vec3 pointLightF = vec3(0);
                    float dummy0, dummy1, dummy2;
                    EvaluateBSDF(hitNormalWorld, normalize(-rayDirWorld), lightDir, albedo, metallic, roughness, pointLightF, dummy0, dummy1, dummy2);

                    //Since point lights are single points with infinite directions 
                    //we do not use pdf and only the f value for lighting the surface
                    radiance += throughput * pointLightF * lightIntensity * NdotL;
                }
            }
        }

        //-----------------------------------------------------------------------------------------
        //                                     INDIRECT LIGHTING   
        //-----------------------------------------------------------------------------------------
        vec3 f= vec3(0);
        vec3 l = vec3(0);
        float pdf= 0;
        SampleBSDF(seed, hitNormalWorld, reflectedRayDirWorld, albedo, metallic, roughness, l, f, pdf);
        if (pdf < EPSILON)
            break;

        float NdotL = max(dot(hitNormalWorld, l), 0.0);
        throughput *= f * NdotL / pdf;

        if (bounce > 3) 
        {
            float p = clamp(max(throughput.r, max(throughput.g, throughput.b)), 0.05, 0.95);
            if (GenerateRandomNum(seed) > p) 
                break;
            throughput /= p;
        }

        rayDirWorld = l;
    }

    // Progressive accumulation using texture input
    vec4 previousColor = (uUnmovingFrameCount == 0u) ? vec4(0.0) : imageLoad(uTextureInput, pixel);
    vec3 blended = (previousColor.rgb * float(uUnmovingFrameCount) + radiance) / float(uUnmovingFrameCount + 1u);
    vec4 fragColor = vec4(blended, 1);

    //imageStore(uTextureOutput, pixel, vec4(1.0, 0, 0, 1.0));
    //TODO: transparency is not supported yet
    imageStore(uTextureOutput, pixel, fragColor);
    //imageStore(uTextureInput, pixel, fragColor);

    float luminance = dot(fragColor, vec4(0.2126, 0.7152, 0.0722, 1.0));
    imageStore(uBrightnessTexture, pixel, luminance >= uBloomThreshold ? fragColor : vec4(0.0));
}