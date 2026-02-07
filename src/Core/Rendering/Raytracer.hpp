#pragma once
#include "Utils/Math/Ray.hpp"
#include "Core/Rendering/RenderData.hpp"
#include "Core/Rendering/Vertex.hpp"
#include "Core/Rendering/Material.hpp"
#include "Core/Rendering/Texture.hpp"

namespace Rendering
{
    constexpr float MAX_HIT_DISTANCE = 1e20;
    struct RaytraceHitInfo
    {
        bool m_DidHit = false;
        Vec3 m_HitPos = {};
        float m_HitDistance = 0.0f;
        Vec3 m_HitNormal = {};
        const MaterialData* m_HitMaterial = nullptr;
        IndexTriangle m_HitTriangleIndices = {};
        Vec3Triangle m_HitTrianglePos = {};
    };

    struct RaytraceSettings
    {
        std::uint32_t m_SamplesPerPixel = 0;
        std::uint32_t m_MaxBounces = 0;
        std::uint32_t m_EmissiveCount = 0;
        size_t m_UnmovingFrameCount = 0;
        float m_BloomThreshold = 0.0f;
    };

    struct BSDFEvaluationInfo
    {
        Vec3 m_Specular;
        Vec3 m_Diffuse;

        /// <summary>
        /// The Probability Density Function (PDF) for DIFFUSE term
        /// Some directions have higher liklihood of getting selected, so for proper
        /// Monte Carlo Estimator likely of getting selected -> smaller contribution
        /// </summary>
        float m_DiffusePDF = 0.0f;
        /// <summary>
        /// The Probability Density Function (PDF) for SPECULAR term
        float m_SpecularPDF = 0.0f;
    };
    struct BSDFSampleInfo
    {
        Vec3 m_F;
        float m_PDF = 0.0f;
        Vec3 m_L;
    };

    struct Raytracer
    {
        RaytraceSettings m_Settings;

        using TLASNodeType = BVHNodeStd430;
        using BLASNodeType = BVHNodeStd430;
        //TODO: we dont need std430 for cpu so we should find a way to not use that aligned nodes
        //to reduce memory size
        const TLASNodeType* m_TlasNodes;
        const BLASNodeType* m_BlasNodes;
        const Vertex* m_Vertices;
        const IndexType* m_Indices;
        const Instance* m_Instances;
        const MaterialData* m_Materials;
        const InstanceMesh* m_Meshes;
        const IndexType* m_EmissiveInstanceIndices;
        const Texture* m_Textures;

        const Texture* m_SkyboxTex;
        Texture* m_BrightnessTex;
        const Texture* m_InputTex;
        Texture* m_OutputTex0;
        Texture* m_OutputTex1;

        const LightBlockData* m_LightBlock;
        const ViewerBlockData* m_ViewBlock;

    private:
        RaytraceHitInfo TraceRayLocal(const Ray3D& localRay, const Instance& instance);
        RaytraceHitInfo TraceRayWorld(const Ray3D& worldRay);
    public:
        RaytraceHitInfo TraceRay(const Ray3D& worldRay);

        HDRColor RunPixel(const Vec2Int pixel);
        void Run();
    };
}