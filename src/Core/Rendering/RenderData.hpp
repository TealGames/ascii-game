#pragma once
#include <cstdint>
#include <string>
#include "Utils/Data/Color.hpp"
#include "Utils/Math/Matrix.hpp"
#include "Utils/Math/WorldPosition.hpp"
#include "Utils/DataStructure/BVH.hpp"
#include "Utils/Math/AABB.hpp"
#include "Core/Rendering/Material.hpp"

namespace Rendering
{
    struct PointLightData
    {
        WorldPosition3D m_Pos;
        //This is padding for vec3 since we can only have 2 or 4 floats
        float _padding0;
        HDRColor m_Color;
        float m_Radius;
        uint32_t m_ShadowMapIndex;
        //This is padding to round data to 16 byte alignment
        float _padding1[2];

        PointLightData();
        PointLightData(const WorldPosition3D& pos, const HDRColor& color, const float radius);
    };
    struct DirectionalLightData
    {
        Vec3 m_Direction = {};
        float _padding0 = 0;
        HDRColor m_Color = {};

        DirectionalLightData();
        DirectionalLightData(const Vec3& dir, const HDRColor& color);
    };
    constexpr size_t MAX_POINT_LIGHTS = 2;
    struct LightBlockData
    {
        DirectionalLightData m_DirLight = {};
        int m_PointLightsCount = 0;
        float _padding[3];
        PointLightData m_PointLights[MAX_POINT_LIGHTS] = {};
    };

    //NOTE: must be aligned to std::430 (members and struct at 16 byte alignment)
    constexpr int INVALID_TEXTURE_INDEX = -1;
    struct MaterialData
    {
        HDRColor m_BaseColor;
        HDRColor m_EmissiveColor;
        float m_Alpha;
        float m_Metallic;
        float m_Roughness;
        int m_AlbedoIndex;
        //float _padding;

        MaterialData();
        MaterialData(const Material& material, const int albedoIndx);

        std::string ToString() const;
    };

    struct ViewerBlockData
    {
        Mat4 m_ViewMatrix;
        Mat4 m_ProjectionMatrix;
        Vec3 m_WorldPos;
        Vec3 m_FowardDir;
        Vec3 m_RightDir;
        Vec3 m_UpDir;
        Vec2Int m_ScreenSize;
        float m_FovY;
    };

    struct InstanceBoundsData
    {
        AABB3D m_RootWorldBounds;
        /// <summary>
        /// The instance index into the renderer instance buffer
        /// which contains the mesh index and model matrices
        /// </summary>
        std::uint32_t m_InstanceIndex;

        WorldPosition3D GetCenter() const;
        AABB3D GetAABB() const;
        std::string ToString() const;
    };

    constexpr size_t STD_430_ALIGN = 16;
    using BVHNodeStd430 = BVHNodeBase<STD_430_ALIGN>;
    using AABB3DStd430 = AABB3DBase<STD_430_ALIGN>;

    extern template struct BVHNodeBase<STD_430_ALIGN>;
    extern template struct AABB3DBase<STD_430_ALIGN>;
}