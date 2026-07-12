#include "pch.hpp"
#include "Core/Asset/SpriteAsset.hpp"
#include "Core/Serialization/JsonSerializers.hpp"
#include "Utils/IOHandler.hpp"

namespace Engine::Rendering
{
	const std::array<std::string_view,1> SpriteAsset::EXTENSIONS = { ".json" };

	SpriteAsset::SpriteAsset(const std::filesystem::path& path)
		: Asset(path), m_visual()
	{
		ASSET_EXTENSION_CHECK
	}

	const VisualData& SpriteAsset::GetVisual() const
	{
		return m_visual;
	}
	VisualData& SpriteAsset::GetVisualMutable()
	{
		return m_visual;
	}

	void SpriteAsset::UpdateAssetFromFile()
	{
		Json json = Json::parse(::Utils::IO::TryReadFileFull(GetAbsolutePathCopy()));
		m_visual = json.get<VisualData>();
	}
	void SpriteAsset::SaveToPath(const std::filesystem::path& path)
	{
		Json json = m_visual;
		Assert(::Utils::IO::TryWriteFile(path, json.dump()),
			"Tried to save sprite asset at apth:'{}' but failed", path.string());
	}

	std::string SpriteAsset::ToString() const
	{
		return std::format("[SpriteAsset path:{} visual:{}]", GetAbsolutePathCopy().string(), m_visual.ToString());
	}
}

