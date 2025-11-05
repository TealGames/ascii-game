#include "Core/Rendering/Material.hpp"

namespace Rendering
{
	std::string Material::ToString() const
	{
		return std::format("[Material Name:{} OverlayColor:{} Alpha:{} Emissive:{} Roughness:{} Metallic:{}]",
			m_Name, m_BaseColor.ToString(), m_Alpha, m_EmissiveColor.ToString(), m_Roughness, m_Metallic);
	}
}
