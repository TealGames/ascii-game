#include "Core/Rendering/RenderData.hpp"

namespace Engine::Rendering
{
    PointLightData::PointLightData() : PointLightData({}, {}, 0) {}
    PointLightData::PointLightData(const WorldPosition3D& pos, const ColHDR4& color, const float radius)
        : m_Pos(pos), m_Color(color), m_Radius(radius), _padding0(0), _padding1{}, m_ShadowMapIndex(-1) {}

    DirectionalLightData::DirectionalLightData() : DirectionalLightData({}, {}) {}
    DirectionalLightData::DirectionalLightData(const Vec3& dir, const ColHDR4& color)
        : m_Direction(dir), m_Color(color), _padding0(0) {}

    MaterialData::MaterialData() : MaterialData(Material{}, INVALID_TEXTURE_INDEX) {}
    MaterialData::MaterialData(const Material& material, const int albedoIndx)
        : m_BaseColor(material.GetBaseColor()), m_Alpha(material.GetAlpha()), m_Metallic(material.GetMatallic()),
        m_Roughness(material.GetRoughness()), m_EmissiveColor(material.GetEmissiveColor()), m_AlbedoIndex(albedoIndx) {}

    std::string MaterialData::ToString() const
    {
        return std::format("[MaterialData BaseColor:{} Alpha:{} EmissiveColor:{}]",
            m_BaseColor.ToString(), m_Alpha, m_EmissiveColor.ToString());
    }

    WorldPosition3D InstanceBoundsData::GetCenter() const { return m_RootWorldBounds.GetCenter(); }
    AABB3D InstanceBoundsData::GetAABB() const { return m_RootWorldBounds; }
    std::string InstanceBoundsData::ToString() const
    {
        return std::format("[RootWorldBounds:{} InstIdx:{}]",
            m_RootWorldBounds.ToString(), m_InstanceIndex);
    }

    template struct BVHNodeBase<STD_430_ALIGN>;
    template struct AABB3DBase<STD_430_ALIGN>;
}