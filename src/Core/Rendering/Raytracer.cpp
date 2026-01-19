#include <execution>
#include "Core/Rendering/Raytracer.hpp"
#include "Utils/Math/MathAdvanced.hpp"
#include "Core/Rendering/Texture.hpp"

static constexpr size_t MAX_STACK_SIZE = 100;

namespace Rendering
{
    Vec3 SampleRandomTrianglePoint(const Vec3& v0, const Vec3& v1, const Vec3& v2, std::uint32_t& seed)
    {
        //TODO: sqrt is slow
        float u = std::sqrt(Utils::FastRandom(seed));
        float v = Utils::FastRandom(seed);
        return (1.0f - u) * v0 + u * (1.0f - v) * v1 + u * v * v2;
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
    reflected so you just see reflection from other objects and can not see the pool's bottom 
    */
    Vec3 FresnelSchlickReflectance(float cosTheta, const Vec3& F0)
    {
        return F0 + (1.0f - F0) * powf(1.0f - cosTheta, 5.0f);
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
        float denom = (NdotH * NdotH) * (a2 - 1.0f) + 1.0f;
        denom = PI * denom * denom;
        return a2 / std::max(Utils::EPSILON_F, denom);
    }

    /*
       Fast approximation for a part of the geometry term equation (G1) for the GGX variant
       <param NdotV> the dot product of the surface normal and another direction (either light to surface, or surface to camera) <param>
       <param k> alpha / 2 (alpha: roughness^2) <param>
    */
    float GeometrySchlickGGX(float NdotV, float k)
    {
        return std::max(NdotV, Utils::EPSILON_F) / (NdotV * (1.0f - k) + k);
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

    // Builds tangent and bitangent from a unit normal N with no branches, no cross products, no normalization
    void BuildOrthonormalBasisFrisvad(const Vec3& N, Vec3& outTangent, Vec3& outBitangent)
    {
        float sign = std::copysign(1.0f, N.m_Z);
        float a = -1.0f / (sign + N.m_Z);
        float b = N.m_X * N.m_Y * a;

        outTangent = Vec3(1.0f + sign * N.m_X * N.m_X * a, sign * b, -sign * N.m_X);
        outBitangent = Vec3(b, sign + N.m_Y * N.m_Y * a, -N.m_Y);
    }

    // Importance sample GGX: sample half-vector H in tangent space then reflect view to get direction
    Vec3 ImportanceSampleGGX(float Xi1, float Xi2, const Vec3& N, float alpha)
    {
        //NOTE: alpha = roughness^2
        float phi = 2.0 * PI * Xi1;
        float cosTheta = std::sqrtf((1.0f - Xi2) / (1.0f + (alpha * alpha - 1.0f) * Xi2));
        float sinTheta = std::sqrt(std::max(0.0f, 1.0f - cosTheta * cosTheta));

        //Convert spherical coordinates to cartesian in tangent space
        Vec3 Ht = Vec3(std::cos(phi) * sinTheta, std::sin(phi) * sinTheta, cosTheta);

        Vec3 tangent, bitangent;
        BuildOrthonormalBasisFrisvad(N, tangent, bitangent);
        //Transform from tangent space (+Z up) to world space using the TBN basis 
        // (essentially by multiplying by a rotation-only mat3 to get back to world space)
        return (tangent * Ht.m_X + bitangent * Ht.m_Y + N * Ht.m_Z);
    }

    // Cosine-weighted hemisphere sample (for diffuse)
    Vec3 CosineSampleHemisphere(float Xi1, float Xi2, const Vec3& N)
    {
        float r = std::sqrt(Xi1);
        float theta = 2.0f * PI * Xi2;
        float x = r * std::cos(theta);
        float y = r * std::sin(theta);
        float z = std::sqrt(std::max(0.0f, 1.0f - x * x - y * y));

        Vec3 tangent, bitangent;
        BuildOrthonormalBasisFrisvad(N, tangent, bitangent);
        //Transform from tangent space (+Z up) to world space using the TBN basis 
        // (essentially by multiplying by a rotation-only mat3 to get back to world space)
        return (tangent * x + bitangent * y + N * z);
    }

    // PDF for GGX sample (half-vector H)
    float PDF_GGX(float NdotH, float alpha, float VdotH)
    {
        float D = NormalDistributionGGX(NdotH, alpha);
        // pdf for sampling H: D * NdotH
        // conversion to pdf over sample direction L (when reflecting V about H): pdfL = (D * NdotH) / (4 * VdotH)
        return D * NdotH / std::max(Utils::EPSILON_F, 4.0f * VdotH);
    }

    // PDF for cosine hemisphere sample
    float PDF_CosineHemisphere(float NdotL) { return NdotL / PI; }

    //The Bidrectional Scattering Distribution Function -> similar to Bidirectional Reflectance (BRDF)
    //but also including Bidrectional Transmission (BRDF) as BSDF = BRDF + BTDF
    //Simply, BRDF allows surface reflections, but BSDF also allows for light scattering as well (glass, water, subsurface scattering)
    BSDFEvaluationInfo EvaluateBSDF(const Vec3& normal, const Vec3& V, const Vec3& lightDir, const Vec3& albedo, float metallic, float roughness)
    {
        BSDFEvaluationInfo result;

        float NdotL = std::max(DotProduct(normal, lightDir), 0.0f);
        float NdotV = std::max(DotProduct(normal, V), 0.0f);
        if (NdotL <= 0.0f || NdotV <= 0.0f)
            return result;

        Vec3 H = (V + lightDir).Normalize();
        float NdotH = std::max(DotProduct(normal, H), 0.0f);
        float VdotH = std::max(DotProduct(V, H), 0.0f);

        float alpha = roughness * roughness;
        Vec3 F0 = Lerp(Vec3(0.04f), albedo, metallic);

        float D = NormalDistributionGGX(NdotH, alpha);
        float k = (alpha + 1.0f) * (alpha + 1.0f) / 8.0f;
        float G = GeometrySmith(NdotV, NdotL, k);
        Vec3 F = FresnelSchlickReflectance(VdotH, F0);

        Vec3 specular = (D * G * F) / std::max(4.0f * NdotV * NdotL, Utils::EPSILON_F);

        Vec3 kd = (1.0f - F) * (1.0f - metallic);
        Vec3 diffuse = kd * albedo / PI;

        result.m_F = diffuse + specular;
        result.m_DiffusePDF = PDF_CosineHemisphere(NdotL);
        result.m_SpecularPDF = PDF_GGX(NdotH, alpha, VdotH);

        float specularWeight = std::clamp(MaxVal(F0), 0.05f, 0.95f);
        result.m_MixedPDF = std::lerp(result.m_DiffusePDF, result.m_SpecularPDF, specularWeight);
        return result;
    }

    BSDFSampleInfo SampleBSDF(std::uint32_t& rngSeed, const Vec3& normal, const Vec3& V, const Vec3& albedo, float metallic, float roughness)
    {
        float alpha = roughness * roughness;
        BSDFSampleInfo result;

        Vec3 F0 = Lerp(Vec3(0.04f), albedo, metallic);
        float specularWeight = std::clamp(MaxVal(F0), 0.05f, 0.95f);
        bool chooseSpecular = Utils::FastRandom(rngSeed) < specularWeight;

        //Essentially we determine here if the point that we want to sample is going to be SPECULAR (bounce back at same as incoming angle)
        //OR diffuse (bounce in random direction) based on the probabiluty we choose before
        if (chooseSpecular)
        {
            Vec3 H = ImportanceSampleGGX(Utils::FastRandom(rngSeed), Utils::FastRandom(rngSeed), normal, roughness * roughness);
            result.m_L = Utils::ReflectAcrossNormal(-V, H);
        }
        else
        {
            result.m_L = CosineSampleHemisphere(Utils::FastRandom(rngSeed), Utils::FastRandom(rngSeed), normal);
        }

        BSDFEvaluationInfo bsdfInfo = EvaluateBSDF(normal, V, result.m_L, albedo, metallic, roughness);
        result.m_F = bsdfInfo.m_F;
        result.m_PDF = bsdfInfo.m_MixedPDF;
        return result;
    }

    HDRColor SampleEquirectangular(const Vec3 dir, const Texture& hdrMap)
    {
        float theta = std::atanf(dir.m_Z / dir.m_X);
        float phi = std::asinf(dir.m_Y);

        // Map theta from [-pi, pi] to [0,1]
        // Map phi from [-pi/2, pi/2] to [0,1]
        const Vec2 uv = Vec2((theta + PI) / (2.0 * PI), (phi + 1.57079633) / PI);

        return hdrMap.SampleHDRAtUV(uv);
    }

    RaytraceHitInfo Raytracer::TraceRayLocal(const Ray3D& localRay, const Instance& instance)
    {
        int stack[MAX_STACK_SIZE];
        int stackPtr = 0;

        const InstanceMesh& mesh = m_Meshes[instance.m_MeshIndex];
        int startNodeIndex = int(mesh.m_BLASTreesInterval.m_StartIndex);
        //NOTE: we only need the first node to be offset since all tree child indices
        //should be adjusted to be in terms of the full node array
        stack[stackPtr++] = startNodeIndex;

        const BLASNodeType* node = nullptr;
        float tEnter = 0.0f, tExit = 0.0f;
        float leafTEnter = 0.0f;
        float tEnterChild0 = 0.0f, tExitChild0 = 0.0f, tEnterChild1 = 0.0f, tExitChild1 = 0.0f;
        float localMinTEnter = MAX_HIT_DISTANCE;
        RaytraceHitInfo closestHit;
        closestHit.m_HitDistance = localMinTEnter;

        while (stackPtr > 0)
        {
            node = &m_BlasNodes[stack[--stackPtr]];
            if (!Utils::RayIntersectsBounds(node->m_Bounds, localRay, &tEnter, &tExit))
                continue;

            if (tEnter > localMinTEnter)
                continue;

            if (node->IsLeaf())
            {
                for (int i = 0; i < node->m_ObjectCount; i++)
                {
                    //NOTE: since the object indices are in terms of TRIANGLES, we must multiply by
                    //3 to get the corresponding index
                    const IndexType indexV0 = m_Indices[(node->m_ObjectStartIndex + i) * 3];
                    const IndexType indexV1 = m_Indices[(node->m_ObjectStartIndex + i) * 3 + 1];
                    const IndexType indexV2 = m_Indices[(node->m_ObjectStartIndex + i) * 3 + 2];

                    Vec3 triangleNormal;

                    if (Utils::RayIntersectsTriangleInterpolated(
                        m_Vertices[indexV0].m_LocalPos, m_Vertices[indexV1].m_LocalPos, m_Vertices[indexV2].m_LocalPos,
                        m_Vertices[indexV0].m_Normal, m_Vertices[indexV1].m_Normal, m_Vertices[indexV2].m_Normal,
                        localRay, &leafTEnter, triangleNormal))
                    {
                        //If the dir and normal > 0 -> same dir and thus 
                        //it means triangle is a backface and should be ignored
                        if (DotProduct(localRay.m_Dir, triangleNormal) > 0.0)
                            continue;

                        if (leafTEnter > Utils::EPSILON && leafTEnter < localMinTEnter)
                        {
                            localMinTEnter = leafTEnter;

                            closestHit.m_DidHit = true;
                            closestHit.m_HitPos = localRay.m_Origin + localRay.m_Dir * localMinTEnter;
                            //NOTE: since the ray dir is normalized, minT is the same as distance
                            closestHit.m_HitDistance = localMinTEnter;
                            closestHit.m_HitNormal = triangleNormal;
                            closestHit.m_HitMaterial = &m_Materials[instance.m_MaterialIndex];
                            closestHit.m_HitTriangleIndices = { indexV0, indexV1, indexV2 };
                            closestHit.m_HitTrianglePos = Vec3Triangle(
                                m_Vertices[closestHit.m_HitTriangleIndices.m_0].m_LocalPos,
                                m_Vertices[closestHit.m_HitTriangleIndices.m_1].m_LocalPos,
                                m_Vertices[closestHit.m_HitTriangleIndices.m_2].m_LocalPos);
                        }
                    }
                }
            }
            else
            {
                bool minHitChild0 = Utils::RayIntersectsBounds(m_BlasNodes[node->m_IndexChild0].m_Bounds, localRay,
                    &tEnterChild0, &tExitChild0) && tEnterChild0 <= localMinTEnter;
                bool minHitChild1 = Utils::RayIntersectsBounds(m_BlasNodes[node->m_IndexChild1].m_Bounds, localRay,
                    &tEnterChild1, &tExitChild1) && tEnterChild1 <= localMinTEnter;

                if (minHitChild0 && minHitChild1)
                {
                    if (tEnterChild0 < tEnterChild1)
                    {
                        stack[stackPtr++] = node->m_IndexChild1;
                        stack[stackPtr++] = node->m_IndexChild0;
                    }
                    else
                    {
                        stack[stackPtr++] = node->m_IndexChild0;
                        stack[stackPtr++] = node->m_IndexChild1;
                    }
                }
                else if (minHitChild0)
                    stack[stackPtr++] = node->m_IndexChild0;
                else if (minHitChild1)
                    stack[stackPtr++] = node->m_IndexChild1;
            }
        }

        /*
        if (closestHit.m_DidHit)
        {
            closestHit.m_HitTrianglePos = Vec3Triangle(
                (instance.m_ModelMatrix * Vec4(m_Vertices[closestHit.m_HitTriangleIndices.m_0].m_LocalPos, 1.0)).GetXYZ(),
                (instance.m_ModelMatrix * Vec4(m_Vertices[closestHit.m_HitTriangleIndices.m_1].m_LocalPos, 1.0)).GetXYZ(),
                (instance.m_ModelMatrix * Vec4(m_Vertices[closestHit.m_HitTriangleIndices.m_2].m_LocalPos, 1.0)).GetXYZ());
            closestHit.m_HitNormal = (instance.m_NormalModelMatrix * closestHit.m_HitNormal).GetNormalized();
            closestHit.m_HitPos = (instance.m_ModelMatrix * Vec4(localRay.m_Origin + localRay.m_Dir * localMinTEnter, 1)).GetXYZ();
        }
        */
        return closestHit;
    }

    RaytraceHitInfo Raytracer::TraceRayWorld(const Ray3D& worldRay)
    {
        //Vec3 inverseWorldRayDir = 1.0 / Max(Abs(worldRay.m_Dir), Vec3(1e-8)) * GetSign(worldRay.m_Dir);
        //ivec3 worldRayDirSign = ivec3(lessThan(inverseWorldRayDir, vec3(0.0)));
        int stack[MAX_STACK_SIZE];
        int stackPtr = 0;

        stack[stackPtr++] = 0;

        const TLASNodeType* node = nullptr;
        float tEnter = 0.0f, tExit = 0.0f;
        float tEnterChild0 = 0.0f, tExitChild0 = 0.0f, tEnterChild1 = 0.0f, tExitChild1 = 0.0f;
        RaytraceHitInfo closestHit;
        closestHit.m_HitDistance = MAX_HIT_DISTANCE;
        Ray3D rayLocal;

        while (stackPtr > 0)
        {
            node = &m_TlasNodes[stack[--stackPtr]];
            if (!Utils::RayIntersectsBounds(node->GetAABB(), worldRay, &tEnter, &tExit))
                continue;

            //TODO: check all occurences of DoesIntersectSceneWorld and if thye all use normalized ray dir, 
            //we can directly compare tEnter and minHitDistance
            if ((worldRay.m_Dir * tEnter).GetMagnitude() > closestHit.m_HitDistance)
                continue;

            if (node->IsLeaf())
            {
                //hitLeaf = max(0, hitLeaf);
                //NOTE: for the tlas tree, the objects are indices into the Instance buffer
                for (int i = 0; i < node->m_ObjectCount; i++)
                {
                    const Instance& instance = m_Instances[node->m_ObjectStartIndex + i];
                    rayLocal.m_Origin = (instance.m_InverseModelMatrix * Vec4(worldRay.m_Origin, 1.0)).GetXYZ();
                    rayLocal.m_Dir = (instance.m_InverseModelMatrix * Vec4(worldRay.m_Dir, 0.0)).GetXYZ().Normalize();

                    RaytraceHitInfo currHitLocalInfo = TraceRayLocal(rayLocal, instance);
                    if (!currHitLocalInfo.m_DidHit)
                        continue;

                    const Vec3 currWorldHitPos = (instance.m_ModelMatrix * Vec4(currHitLocalInfo.m_HitPos, 1)).GetXYZ();
                    float currMinHitDistanceWorld = (currWorldHitPos - worldRay.m_Origin).GetMagnitude();
                    if (currMinHitDistanceWorld >= closestHit.m_HitDistance)
                        continue;

                    closestHit.m_DidHit = true;
                    closestHit.m_HitPos = currWorldHitPos;
                    closestHit.m_HitDistance = currMinHitDistanceWorld;
                    closestHit.m_HitNormal = (instance.m_NormalModelMatrix * currHitLocalInfo.m_HitNormal).GetNormalized();
                    closestHit.m_HitMaterial = currHitLocalInfo.m_HitMaterial;
                    closestHit.m_HitTriangleIndices = currHitLocalInfo.m_HitTriangleIndices;
                    closestHit.m_HitTrianglePos = Vec3Triangle(
                        (instance.m_ModelMatrix * Vec4(currHitLocalInfo.m_HitTrianglePos.m_0, 1.0)).GetXYZ(),
                        (instance.m_ModelMatrix * Vec4(currHitLocalInfo.m_HitTrianglePos.m_1, 1.0)).GetXYZ(),
                        (instance.m_ModelMatrix * Vec4(currHitLocalInfo.m_HitTrianglePos.m_2, 1.0)).GetXYZ());
                }
            }
            else
            {
                bool minHitChild0 = Utils::RayIntersectsBounds(m_TlasNodes[node->m_IndexChild0].m_Bounds, worldRay,
                    &tEnterChild0, &tExitChild0) && (worldRay.m_Dir * tEnterChild0).GetMagnitude() <= closestHit.m_HitDistance;
                bool minHitChild1 = Utils::RayIntersectsBounds(m_TlasNodes[node->m_IndexChild1].m_Bounds, worldRay,
                    &tEnterChild1, &tExitChild1) && (worldRay.m_Dir * tEnterChild1).GetMagnitude() <= closestHit.m_HitDistance;

                if (minHitChild0 && minHitChild1)
                {
                    if (tEnterChild0 < tEnterChild1)
                    {
                        stack[stackPtr++] = node->m_IndexChild1;
                        stack[stackPtr++] = node->m_IndexChild0;
                    }
                    else
                    {
                        stack[stackPtr++] = node->m_IndexChild0;
                        stack[stackPtr++] = node->m_IndexChild1;
                    }
                }
                else if (minHitChild0)
                    stack[stackPtr++] = node->m_IndexChild0;
                else if (minHitChild1)
                    stack[stackPtr++] = node->m_IndexChild1;
            }
        }
        return closestHit;
    }

    RaytraceHitInfo Raytracer::TraceRay(const Ray3D& worldRay)
    {
        return TraceRayWorld(worldRay);
    }

    HDRColor Raytracer::RunPixel(const Vec2Int pixel)
    {
        float aspectRatio = float(m_ViewBlock->m_ScreenSize.m_X) / float(m_ViewBlock->m_ScreenSize.m_Y);
        float scale = std::tan(m_ViewBlock->m_FovY * 0.5f);

        //Radiance is the total color that gets accumulated for this ray
        Vec3 radiance;
        Vec3 averageRadiance;
        RaytraceHitInfo hitInfo;
        BSDFEvaluationInfo bsdfEvalInfo;
        BSDFSampleInfo bsdfSampleInfo;
        Ray3D worldRay;
        for (int s = 0; s < m_Settings.m_SamplesPerPixel; s++)
        {
            radiance = Vec3(0.0f);
            //Energy percent is the multiplier for rgb which decreases as ray hits an object
            //and energy decreases based on the color (how much light is absorbed)
            Vec3 throughput = Vec3(1.0f);

            uint32_t seed = uint32_t(pixel.m_X * 1973u + pixel.m_Y * 9277u)
                ^ (m_Settings.m_UnmovingFrameCount * m_Settings.m_SamplesPerPixel + s) * 26699u;

            //The following apply a small <1 jitter to the pixel coordinate in order to prevent aliasing (jagged edges)
            //If we always used the center of the pixel it would not appear smooth, so by doing this combined with accumulation
            //we get a nicer more-filled and less jagged look to edges
            float normalizedPixelX = (float(pixel.m_X) + Utils::FastRandom(seed)) / float(m_ViewBlock->m_ScreenSize.m_X);
            float normalizedPixelY = (float(pixel.m_Y) + Utils::FastRandom(seed)) / float(m_ViewBlock->m_ScreenSize.m_Y);

            //This is the jittered pixel coord in normalized device coordinate pos [-1, 1] 
            //(horizontal and vertical offset from center of screen)
            Vec2 ndcPos = Vec2(normalizedPixelX * 2.0f - 1.0f, normalizedPixelY * 2.0f - 1.0f);
            //NOTE: this is a shortcut for doing inverse projection and inverse view matrix multiplication
            //ASSUMING frustum is symmetric (meaning no offset between camera center and near plane rectangle center)
            worldRay.m_Dir = (m_ViewBlock->m_FowardDir + m_ViewBlock->m_RightDir * (ndcPos.m_X * aspectRatio * scale)
                + m_ViewBlock->m_UpDir * (ndcPos.m_Y * scale)).Normalize();
            worldRay.m_Origin = m_ViewBlock->m_WorldPos;

            for (int bounce = 0; bounce < m_Settings.m_MaxBounces; bounce++)
            {
                hitInfo = TraceRay(worldRay);
                if (!hitInfo.m_DidHit)
                {
                    HDRColor skyColor;
                    if (m_SkyboxTex != nullptr)
                    {
                        skyColor = SampleEquirectangular(worldRay.m_Dir, *m_SkyboxTex);
                    }

                    radiance += throughput * skyColor.GetRGB();
                    break;
                }
                
                Vec3 albedo = hitInfo.m_HitMaterial->m_BaseColor.GetRGB() * hitInfo.m_HitMaterial->m_BaseColor.m_A;
                if (hitInfo.m_HitMaterial->m_AlbedoIndex >= 0)
                {
                    Vec2 uvEdge0 = m_Vertices[hitInfo.m_HitTriangleIndices.m_0].m_UVPos;
                    Vec2 uvEdge1 = m_Vertices[hitInfo.m_HitTriangleIndices.m_1].m_UVPos;
                    Vec2 uvEdge2 = m_Vertices[hitInfo.m_HitTriangleIndices.m_2].m_UVPos;

                    //We compute barycentric weights of the hit triangle for uv coords in (u, v, w) so
                    //we get accurate texture coords at the hit point
                    Vec3 baryWeights = Utils::CalculateBarycentricWeight(hitInfo.m_HitPos, 
                        hitInfo.m_HitTrianglePos.m_0, hitInfo.m_HitTrianglePos.m_1, hitInfo.m_HitTrianglePos.m_2);
                    Vec2 uv = uvEdge0 * baryWeights.m_X + uvEdge1 * baryWeights.m_Y + uvEdge2 * baryWeights.m_Z;

                    const Texture& albedoTex = m_Textures[hitInfo.m_HitMaterial->m_AlbedoIndex];
                    HDRColor texColor = albedoTex.SampleHDRAtUV(uv);
                    albedo *= texColor.GetRGB();
                }
               
                float metallic = std::clamp(hitInfo.m_HitMaterial->m_Metallic, 0.0f, 1.0f);
                float roughness = std::clamp(hitInfo.m_HitMaterial->m_Roughness, 0.02f, 1.0f);

                worldRay.m_Origin = hitInfo.m_HitPos + hitInfo.m_HitNormal * Utils::EPSILON_F;
                Vec3 reflectedRayDirWorld = -worldRay.m_Dir;

                //--------------------------------------------------------------------------------------------------
                //                                  DIRECT LIGHTING (NEXT EVENT ESTIMATION)
                //--------------------------------------------------------------------------------------------------
                {
                    Vec3 lightVec, lightDir, shadowHitPos;
                    float hitDistanceToLight = 0.0f;
                    if (m_Settings.m_EmissiveCount > 0)
                    {
                        // --------------------------------- Stochastic light sampling ----------------------------------
                        // Here we pick a random triangle on the light to see if hit object gets affected by this light
                        // NOTE: because we assume every 3 is a triangle, we have to divide by 3 to find the triangle index
                        // and we multiple by 3 to convert the triangle index to a vertex index
                        uint32_t randomLightInstanceIndex = m_EmissiveInstanceIndices[uint32_t(Utils::FastRandom(seed) * float(m_Settings.m_EmissiveCount))];
                        const Instance& lightInstance = m_Instances[randomLightInstanceIndex];
                        const InstanceMesh& lightMeshInstance = m_Meshes[randomLightInstanceIndex];
                        uint32_t randomTriangle = uint32_t(Utils::FastRandom(seed) * float(lightMeshInstance.m_NumIndices / 3));
                        uint32_t baseIndex = lightMeshInstance.m_IndexOffset + randomTriangle * 3u;
                        IndexType lightIndexV0 = m_Indices[baseIndex + 0];
                        IndexType lightIndexV1 = m_Indices[baseIndex + 1];
                        IndexType lightIndexV2 = m_Indices[baseIndex + 2];

                        Vec3 lightVertex0 = (lightInstance.m_ModelMatrix * Vec4(m_Vertices[lightIndexV0].m_LocalPos, 1.0f)).GetXYZ();
                        Vec3 lightVertex1 = (lightInstance.m_ModelMatrix * Vec4(m_Vertices[lightIndexV1].m_LocalPos, 1.0f)).GetXYZ();
                        Vec3 lightVertex2 = (lightInstance.m_ModelMatrix * Vec4(m_Vertices[lightIndexV2].m_LocalPos, 1.0f)).GetXYZ();
                        // Here we find the sample of the light using the random triangle we choose above
                        Vec3 randomLightTriangleWorldPoint = SampleRandomTrianglePoint(lightVertex0, lightVertex1, lightVertex2, seed);
                        float lightTriangleArea = Utils::CalculateTriangleArea(lightVertex0, lightVertex1, lightVertex2);
                        Vec3 lightNormalWorld = CrossProduct(lightVertex1 - lightVertex0, lightVertex2 - lightVertex0).Normalize();

                        lightVec = randomLightTriangleWorldPoint - hitInfo.m_HitPos;
                        hitDistanceToLight = lightVec.GetMagnitude();
                        lightDir = lightVec / hitDistanceToLight;

                        float NdotL = std::max(0.0f, DotProduct(hitInfo.m_HitNormal, lightDir));
                        float NlDot = std::max(0.0f, DotProduct(lightNormalWorld, -lightDir));

                        if (NdotL > Utils::EPSILON_F && NlDot > Utils::EPSILON_F)
                        {
                            hitInfo = TraceRay(Ray3D{ worldRay.m_Origin, lightDir });
                            bool occluded = hitInfo.m_DidHit && (shadowHitPos - worldRay.m_Origin).GetMagnitude() < hitDistanceToLight - Utils::EPSILON_F;
                            if (!occluded)
                            {
                                float lightTrianglePdfArea = 1.0 / (lightTriangleArea * float(m_Settings.m_EmissiveCount));
                                float lightPdf = lightTrianglePdfArea * hitDistanceToLight * hitDistanceToLight / std::max(NlDot, Utils::EPSILON_F);

                                bsdfEvalInfo = EvaluateBSDF(hitInfo.m_HitNormal, (-worldRay.m_Dir).Normalize(), lightDir, albedo, metallic, roughness);

                                // MIS power heuristic (more stable that balance heuristic)
                                float w = (lightPdf * lightPdf) / (lightPdf * lightPdf + bsdfEvalInfo.m_MixedPDF * bsdfEvalInfo.m_MixedPDF);
                                HDRColor materialEmission = m_Materials[lightInstance.m_MaterialIndex].m_EmissiveColor;
                                Vec3 lightRadiance = materialEmission.GetRGB() * materialEmission.m_A;

                                radiance += throughput * bsdfEvalInfo.m_F * lightRadiance * NdotL * w / lightPdf;
                            }
                        }

                    }
                    
                    // ----------------------------------- POINT LIGHTS ------------------------------
                    // In physics based rendering we can not just give point lights an area and glow, 
                    // we must do the same process as sampling an emissive texture and this also
                    // means the point light emits light in all directions infinitely with a cutoff function
                    for (int i = 0; i < m_LightBlock->m_PointLightsCount; i++)
                    {
                        const PointLightData& light = m_LightBlock->m_PointLights[i];
                        lightVec = light.m_Pos - hitInfo.m_HitPos;
                        hitDistanceToLight = lightVec.GetMagnitude();
                        lightDir = lightVec / hitDistanceToLight;
                        float NdotL = std::max(0.0f, DotProduct(hitInfo.m_HitNormal, lightDir));

                        if (NdotL <= Utils::EPSILON_F)
                            continue;

                        hitInfo = TraceRay(Ray3D{ worldRay.m_Origin, lightDir });
                        bool occluded = hitInfo.m_DidHit && (shadowHitPos - worldRay.m_Origin).GetMagnitude() < hitDistanceToLight - Utils::EPSILON_F;
                        if (!occluded)
                        {
                            Vec3 lightIntensity = light.m_Color.GetRGB() * light.m_Color.m_A * std::clamp(1.0f - (hitDistanceToLight / light.m_Radius), 0.0f, 1.0f);
                            if (lightIntensity.GetMagnitude() < 1e-5)
                                continue;

                            bsdfEvalInfo = EvaluateBSDF(hitInfo.m_HitNormal, (-worldRay.m_Dir).Normalize(), lightDir, albedo, metallic, roughness);
                            //Since point lights are single points with infinite directions 
                            //we do not use pdf and only the f value for lighting the surface
                            radiance += throughput * bsdfEvalInfo.m_F * lightIntensity * NdotL;
                        }
                    }
                }

                //-----------------------------------------------------------------------------------------
                //                                     INDIRECT LIGHTING   
                //-----------------------------------------------------------------------------------------
                bsdfSampleInfo = SampleBSDF(seed, hitInfo.m_HitNormal, reflectedRayDirWorld, albedo, metallic, roughness);
                if (bsdfSampleInfo.m_PDF < Utils::EPSILON_F)
                    break;

                float NdotL = std::max(DotProduct(hitInfo.m_HitNormal, bsdfSampleInfo.m_L), 0.0f);
                throughput *= bsdfSampleInfo.m_F * NdotL / bsdfSampleInfo.m_PDF;

                if (bounce > 3)
                {
                    float p = std::clamp(MaxVal(throughput), 0.05f, 0.95f);
                    if (Utils::FastRandom(seed) > p)
                        break;
                    throughput /= p;
                }

                worldRay.m_Dir = bsdfSampleInfo.m_L;
            }
            averageRadiance += radiance;
        }
        averageRadiance /= m_Settings.m_SamplesPerPixel;

        //Progressive temporal accumulation using texture input
        HDRColor previousColor;
        if (m_Settings.m_UnmovingFrameCount != 0)
        {
            //NOTE: this only works because the textures we sample are FLOATS 
            previousColor = m_InputTex->SampleHDRAtTexel(pixel);
        }
        //NOTE: we can NOT just use an average here because as samples increase new ones would have a smaller weight
        //and errors from previous frames will acumulate
        //float blendFactor = clamp(1.0 / max(float(uUnmovingFrameCount + 1u), 1.0), 0.01, 1.0);
        //vec3 blended = mix(previousColor.rgb, averageRadiance, blendFactor);


        size_t maxHistory = 64u;
        float history = float(std::min(m_Settings.m_UnmovingFrameCount, maxHistory));

        Vec3 blended = (previousColor.GetRGB() * history + averageRadiance) / (history + 1.0f);
        Vec4 fragColor = Vec4(blended, 1);

        //imageStore(uTextureOutput, pixel, vec4(1.0, 0, 0, 1.0));
        //TODO: transparency is not supported yet
        if (m_OutputTex0 != nullptr) m_OutputTex0->WriteTexel(pixel, reinterpret_cast<std::byte*>(&fragColor));
        if (m_OutputTex1 != nullptr) m_OutputTex1->WriteTexel(pixel, reinterpret_cast<std::byte*>(&fragColor));

        if (m_BrightnessTex != nullptr)
        {
            float luminance = DotProduct(fragColor, Vec4(0.2126f, 0.7152f, 0.0722f, 1.0f));
            HDRColor brightnessColor = luminance >= m_Settings.m_BloomThreshold ? fragColor : Vec4::Zero();
            m_BrightnessTex->WriteTexel(pixel, reinterpret_cast<std::byte*>(&brightnessColor));
        }
        return fragColor;
        //LogWarning(std::format("Finished pixel:{}", pixel.ToString()));
    }

    void Raytracer::Run()
    {
        ENGINE_ASSERT(m_OutputTex0 != nullptr, "Attempted to run CPU raytracer but primary output texture is NULL");

        const Vec2Int primaryTexOutputSize = m_OutputTex0->GetInfo().m_TexelSize;
        ENGINE_ASSERT(primaryTexOutputSize == m_ViewBlock->m_ScreenSize, 
            "Attempted to run CPU raytracer but primary texture has size:{} that does not match screen size:{}", 
            primaryTexOutputSize.ToString(), m_ViewBlock->m_ScreenSize.ToString());

        if (m_OutputTex1 != nullptr)
        {
            ENGINE_ASSERT(m_OutputTex0->GetInfo().m_TexelSize == m_OutputTex1->GetInfo().m_TexelSize, 
                "Attempted to run CPU raytracer but non-NULL secondary output texture does not have same output size as primary one");
        }

        const HDRColor* inputTexMemPtr = m_OutputTex0->GetCPUMemPtr<HDRColor>();
        const Vec2Int textureSize = primaryTexOutputSize;
        const size_t totalTexels = m_InputTex->CalculateTotalTexels();
        std::for_each(std::execution::par, inputTexMemPtr, inputTexMemPtr + totalTexels,
            [this, textureSize, inputTexMemPtr, totalTexels](const HDRColor& color) -> void
            {
                const size_t texel = &color - inputTexMemPtr;
                HDRColor outputColor = RunPixel(Vec2Int(texel % textureSize.m_X, texel / textureSize.m_X));
                //LogWarning(std::format("Finsihed: {}", float(texel) / totalTexels));
                LogWarning(std::format("color:{} Finsihed: {} at texel:{}/{} (pos: {})",outputColor.ToString(), float(texel) / totalTexels, texel, totalTexels, 
                    Vec2Int(texel % textureSize.m_X, texel / textureSize.m_X).ToString()));
            });
        //LogWarning(std::format("Emtpy:{}", m_OutputTex0->HasEmptyData()));
        //LogError(std::format("Texture data:{}", m_OutputTex0->ToStringBytes(false)));
    }
}