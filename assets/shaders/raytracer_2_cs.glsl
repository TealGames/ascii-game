#version 430 core
//Adapted from video: https://www.youtube.com/watch?v=Qz0KTGYJtUk&t=1567s

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
uniform uint uSamplesPerPixel;
uniform uint uUnmovingFrameCount;
uniform uint uOutputTextureCount;
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

uniform writeonly image2D uBrightnessTexture;
layout(rgba16f) uniform readonly image2D uTextureInput;
layout(rgba16f) uniform writeonly image2D uTextureOutput0;
layout(rgba16f) uniform writeonly image2D uTextureOutput1;

const float EPSILON = 1e-8;
const float EPSILON_DET = EPSILON;
const float EPSILON_BARY = 1e-6;
const float EPSILON_T = 1e-5;

const float PI = 3.14159265359;
const uint MAX_STACK_SIZE = 128;

struct Ray
{
	vec3 origin;
	vec3 dir;
};
struct HitInfo
{
	bool didHit;
	float dst;
	vec3 hitPoint;
	vec3 normal;
	Material material;

	uint hitIndexV0;
	uint hitIndexV1;
	uint hitIndexV2;
	vec3 hitVertexWorld0;
	vec3 hitVertexWorld1;
	vec3 hitVertexWorld2;
};

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

                if (DoesIntersectTriangleInterpolated(
                    rayOriginLocal, rayDirLocal,
                    vertices[indexV0].localPos, vertices[indexV1].localPos, vertices[indexV2].localPos,
                    vertices[indexV0].normal, vertices[indexV1].normal, vertices[indexV2].normal,
                    leafTEnter, triangleNormal))
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
		
// Calculate the intersection of a ray with a sphere
HitInfo RaySphere(Ray ray, vec3 sphereCentre, float sphereRadius)
{
	HitInfo hitInfo;
	vec3 offsetRayOrigin = ray.origin - sphereCentre;
	// From the equation: sqrLength(rayOrigin + rayDir * dst) = radius^2
	// Solving for dst results in a quadratic equation with coefficients:
	float a = dot(ray.dir, ray.dir); // a = 1 (assuming unit vector)
	float b = 2 * dot(offsetRayOrigin, ray.dir);
	float c = dot(offsetRayOrigin, offsetRayOrigin) - sphereRadius * sphereRadius;
	// Quadratic discriminant
	float discriminant = b * b - 4 * a * c; 

	// No solution when d < 0 (ray misses sphere)
	if (discriminant >= 0) {
		// Distance to nearest intersection point (from quadratic formula)
		float dst = (-b - sqrt(discriminant)) / (2 * a);

		// Ignore intersections that occur behind the ray
		if (dst >= 0) {
			hitInfo.didHit = true;
			hitInfo.dst = dst;
			hitInfo.hitPoint = ray.origin + ray.dir * dst;
			hitInfo.normal = normalize(hitInfo.hitPoint - sphereCentre);
		}
	}
	return hitInfo;
}

// Calculate the intersection of a ray with a triangle using Möller–Trumbore algorithm
// Thanks to https://stackoverflow.com/a/42752998
/*
HitInfo RayTriangle(Ray ray, Triangle tri)
{
	vec3 edgeAB = tri.posB - tri.posA;
	vec3 edgeAC = tri.posC - tri.posA;
	vec3 normalVector = cross(edgeAB, edgeAC);
	vec3 ao = ray.origin - tri.posA;
	vec3 dao = cross(ao, ray.dir);

	float thisDeterminant = -dot(ray.dir, normalVector);
	float invDet = 1 / thisDeterminant;
				
	// Calculate dst to triangle & barycentric coordinates of intersection point
	float dst = dot(ao, normalVector) * invDet;
	float u = dot(edgeAC, dao) * invDet;
	float v = -dot(edgeAB, dao) * invDet;
	float w = 1 - u - v;
				
	// Initialize hit info
	HitInfo hitInfo;
	hitInfo.didHit = thisDeterminant >= 1E-6 && dst >= 0 && u >= 0 && v >= 0 && w >= 0;
	hitInfo.hitPoint = ray.origin + ray.dir * dst;
	hitInfo.normal = normalize(tri.normalA * w + tri.normalB * u + tri.normalC * v);
	hitInfo.dst = dst;
	return hitInfo;
}
*/

// Thanks to https://gist.github.com/DomNomNom/46bb1ce47f68d255fd5d
bool RayBoundingBox(Ray ray, vec3 boxMin, vec3 boxMax)
{
	vec3 invDir = 1 / ray.dir;
	vec3 tMin = (boxMin - ray.origin) * invDir;
	vec3 tMax = (boxMax - ray.origin) * invDir;
	vec3 t1 = min(tMin, tMax);
	vec3 t2 = max(tMin, tMax);
	float tNear = max(max(t1.x, t1.y), t1.z);
	float tFar = min(min(t2.x, t2.y), t2.z);
	return tNear <= tFar;
};

// --- RNG Stuff ---
			
// PCG (permuted congruential generator). Thanks to:
// www.pcg-random.org and www.shadertoy.com/view/XlGcRh
uint NextRandom(inout uint state)
{
	state = state * 747796405 + 2891336453;
	uint result = ((state >> ((state >> 28) + 4)) ^ state) * 277803737;
	result = (result >> 22) ^ result;
	return result;
}

float RandomValue(inout uint state)
{
	return NextRandom(state) / 4294967295.0; // 2^32 - 1
}

// Random value in normal distribution (with mean=0 and sd=1)
float RandomValueNormalDistribution(inout uint state)
{
	// Thanks to https://stackoverflow.com/a/6178290
	float theta = 2 * 3.1415926 * RandomValue(state);
	float rho = sqrt(-2 * log(RandomValue(state)));
	return rho * cos(theta);
}

// Calculate a random direction
vec3 RandomDirection(inout uint state)
{
	// Thanks to https://math.stackexchange.com/a/1585996
	float x = RandomValueNormalDistribution(state);
	float y = RandomValueNormalDistribution(state);
	float z = RandomValueNormalDistribution(state);
	return normalize(vec3(x, y, z));
}

vec2 RandomPointInCircle(inout uint rngState)
{
	float angle = RandomValue(rngState) * 2 * PI;
	vec2 pointOnCircle = vec2(cos(angle), sin(angle));
	return pointOnCircle * sqrt(RandomValue(rngState));
}

vec2 mod2(vec2 x, vec2 y)
{
	return x - y * floor(x/y);
}

// Crude sky colour function for background light
vec3 GetEnvironmentLight(Ray ray)
{
	return vec3(0);

	/*
	vec3 SkyColourHorizon = vec3(0);
	vec3 SkyColourZenith = vec3(0);
	vec3 GroundColor = vec3(0);

	float SunFocus = 0.0f;
	float SunIntensity = 0.0f;
				
	float skyGradientT = pow(smoothstep(0, 0.4, ray.dir.y), 0.35);
	float groundToSkyT = smoothstep(-0.01, 0, ray.dir.y);
	vec3 skyGradient = mix(SkyColourHorizon, SkyColourZenith, skyGradientT);
	float sun = pow(max(0, dot(ray.dir, _WorldSpaceLightPos0.xyz)), SunFocus) * SunIntensity;
	// Combine ground, sky, and sun
	vec3 composite = mix(GroundColour, skyGradient, groundToSkyT) + sun * (groundToSkyT>=1);
	return composite;
	*/
}

// --- Ray Tracing Stuff ---
HitInfo CalculateRayCollision(inout uint seed, Ray ray)
{
	HitInfo info;
	
	bool didHit = DoesIntersectSceneWorldBVH(ray.origin, ray.dir, info.hitPoint, info.normal, info.material, seed, info.hitIndexV0, 
                                      info.hitIndexV1, info.hitIndexV2, info.hitVertexWorld0, info.hitVertexWorld1, info.hitVertexWorld2);
	info.didHit = didHit;
	info.dst = length(info.hitPoint - ray.origin);
	return info;
}


float CalculateParallelogramArea(vec3 v0, vec3 v1, vec3 v2)
{
    return length(cross(v1- v0, v2- v0));
}
float CalculateTriangleArea(vec3 v0, vec3 v1, vec3 v2)
{
    return 0.5f * CalculateParallelogramArea(v0, v1, v2);
}
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

vec3 Trace(Ray ray, inout uint rngState)
{
	vec3 incomingLight = vec3(0.0);
	vec3 rayColour = vec3(1.0);

	for (int bounceIndex = 0; bounceIndex <= uMaxBounces; bounceIndex ++)
	{
		HitInfo hitInfo = CalculateRayCollision(rngState, ray);

		if (hitInfo.didHit)
		{
			Material material = hitInfo.material;
			vec3 albedo = material.baseColor.rgb * material.baseColor.a;
            if (material.albedoIndex >= 0) 
            {
                vec2 uvEdge0 = vertices[hitInfo.hitIndexV0].uvPos;
                vec2 uvEdge1 = vertices[hitInfo.hitIndexV1].uvPos;
                vec2 uvEdge2 = vertices[hitInfo.hitIndexV2].uvPos;

                //We compute barycentric weights of the hit triangle for uv coords in (u, v, w) so
                //we get accurate texture coords at the hit point
                vec3 baryWeights = CalculateBarycentricWeight(hitInfo.hitPoint, hitInfo.hitVertexWorld0, hitInfo.hitVertexWorld1, hitInfo.hitVertexWorld2);
                vec2 uv = baryWeights.x * uvEdge0 + baryWeights.y * uvEdge1 + baryWeights.z * uvEdge2;

                albedo *= texture(uTextures[material.albedoIndex], uv).rgb;
            }
            float metallic = clamp(material.metallic, 0.0, 1.0);
            float roughness = clamp(material.roughness, 0.02, 1.0);

			float alpha = roughness * roughness;
			vec3 F0 = mix(vec3(0.04), albedo, metallic);
			float specularWeight = clamp(max(F0.r, max(F0.g, F0.b)), 0.05, 0.95);
			bool isSpecularBounce =  RandomValue(rngState) < specularWeight;
					
			ray.origin = hitInfo.hitPoint;
			vec3 diffuseDir = normalize(hitInfo.normal + RandomDirection(rngState));
			vec3 specularDir = reflect(ray.dir, hitInfo.normal);
			ray.dir = normalize(mix(diffuseDir, specularDir, (1 - roughness) * (isSpecularBounce? 1 : 0)));

			// Update light calculations
			vec3 emittedLight = material.emission.rgb * material.emission.a;
			incomingLight += emittedLight * rayColour;
			rayColour *= mix(albedo, F0, isSpecularBounce ? 1.0 : 0.0);
					
			/*
			// Random early exit if ray colour is nearly 0 (can't contribute much to final result)
			float p = max(rayColour.r, max(rayColour.g, rayColour.b));
			if (RandomValue(rngState) >= p) {
				break;
			}
			rayColour *= 1.0f / p; 
			*/
		}
		else
		{
			//incomingLight += GetEnvironmentLight(ray) * rayColour;
			incomingLight += vec3(0.6, 0.7, 0.9) * rayColour;
			break;
		}
	}

	return incomingLight;
}

	
void main()
{
	uint something = uEmissiveCount +1;
	bool hasSkybox = uHasSkybox;

	ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);
	if (pixelCoord.x >= uScreenSize.x || pixelCoord.y >= uScreenSize.y)
		return;

	vec2 pixelCoordNormalized = pixelCoord / uScreenSize;
	ivec2 numPixels = uScreenSize;
	uint pixelIndex = pixelCoord.y * numPixels.x + pixelCoord.x;
	uint rngState = pixelIndex + uUnmovingFrameCount * 719393;

	mat4 CamLocalToWorldMatrix = mat4(
		vec4(uViewerBlock.rightDir, 0.0),
		vec4(uViewerBlock.upDir, 0.0),
		vec4(uViewerBlock.forwardDir, 0.0),
		vec4(uViewerBlock.worldPos, 1.0));

	float focusDistance = 0.2;
	float DefocusStrength = 0.02;
	float DivergeStrength = 1.0;
	vec2 screenPosCentered = pixelCoordNormalized - vec2(0.5, 0.5);
	float aspectRatio = float(uScreenSize.x) / float(uScreenSize.y);
	float planeHeight = 2.0 * focusDistance * tan(uViewerBlock.yFov * 0.5);
	float planeWidth = planeHeight * aspectRatio; 

	vec3 focusPointLocal = vec3(screenPosCentered.x * planeWidth, screenPosCentered.y * planeHeight, focusDistance);
	vec3 focusPoint = (CamLocalToWorldMatrix * vec4(focusPointLocal, 1)).xyz;

	vec3 camRight = uViewerBlock.rightDir;
	vec3 camUp = uViewerBlock.upDir;

	// Trace a bunch of rays and average the result
	Ray ray;
	vec3 totalIncomingLight = vec3(0.0);

	for (int rayIndex = 0; rayIndex < uSamplesPerPixel; rayIndex ++)
	{	
		// Calculate ray origin and direction
		vec2 defocusJitter = RandomPointInCircle(rngState) * DefocusStrength;
		ray.origin = uViewerBlock.worldPos + camRight * defocusJitter.x + camUp * defocusJitter.y;

		vec2 jitter = RandomPointInCircle(rngState) * DivergeStrength / numPixels.x;
		vec3 jitteredFocusPoint = focusPoint + camRight * jitter.x + camUp * jitter.y;
		ray.dir = normalize(jitteredFocusPoint - ray.origin);
		// Trace
		totalIncomingLight += Trace(ray, rngState);
	}

	vec3 col = totalIncomingLight / uSamplesPerPixel;
	vec4 colPrev = (uUnmovingFrameCount == 0u) ? vec4(0.0) : imageLoad(uTextureInput, pixelCoord);
	float weight = 1.0 / (uUnmovingFrameCount + 1);
	vec4 fragColor = vec4(colPrev.rgb * (1 - weight) + col * weight, 1.0);

	if (uOutputTextureCount >= 1) imageStore(uTextureOutput0, pixelCoord, fragColor);
    if (uOutputTextureCount >= 2) imageStore(uTextureOutput1, pixelCoord, fragColor);

    float luminance = dot(fragColor, vec4(0.2126, 0.7152, 0.0722, 1.0));
    imageStore(uBrightnessTexture, pixelCoord, luminance >= uBloomThreshold ? fragColor : vec4(0.0));
}