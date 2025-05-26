#include "pch.hpp"
#include "SpriteAsset.hpp"
#include "JsonSerializers.hpp"
#include "IOHandler.hpp"

const std::string SpriteAsset::EXTENSION = ".json";

SpriteAsset::SpriteAsset(const std::filesystem::path& path) 
	: Asset(path, false), m_visual() {}

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
	Json json = Json::parse(IO::TryReadFileFull(GetPathCopy()));
	m_visual = json.get<VisualData>();
}
void SpriteAsset::SaveToPath(const std::filesystem::path& path)
{
	Json json = m_visual;
	Assert(this, IO::TryWriteFile(path, json.dump()),
		std::format("Tried to save sprite asset at apth:'{}' but failed", path.string()));
}

std::string SpriteAsset::ToString() const
{
	return std::format("[SpriteAsset path:{} visual:{}]", GetPathCopy().string(), m_visual.ToString());
}
