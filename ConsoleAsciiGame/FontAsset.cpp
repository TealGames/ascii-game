#include "pch.hpp"
#include "FontAsset.hpp"
#include "Debug.hpp"
#include "RaylibUtils.hpp"
#include "IOHandler.hpp"

const std::string FontAsset::EXTENSION = ".txt";

FontAsset::FontAsset(const std::filesystem::path& path) 
	: Asset(path, false), m_font()
{
	if (!Assert(this, IO::DoesPathHaveExtension(path, EXTENSION), std::format("Tried to create a font asset from path:{}"
		"but it does not have required font extension:'{}'", path.string(), EXTENSION)))
		return;

	const std::string pathString = path.string();
	m_font = LoadFontEx(pathString.c_str(), 64, nullptr, 0);
}
FontAsset::~FontAsset()
{
	//Note: font uses gpu resources that may not be unloaded and must be done manually
	//and also, even though font is passed by value, font resources are loaded once and passed via pointerss
	UnloadFont(m_font);
}

Font& FontAsset::GetFontMutable()
{
	if (!Assert(this, RaylibUtils::IsValidFont(m_font), std::format("Tried to get font MUTABLE but font is invalid")))
		throw std::invalid_argument("Invalid font state");

	return m_font;
}

const Font& FontAsset::GetFont() const
{
	if (!Assert(this, RaylibUtils::IsValidFont(m_font), std::format("Tried to get font but font is invalid")))
		throw std::invalid_argument("Invalid font state");

	return m_font;
}

void FontAsset::UpdateAssetFromFile()
{
	const std::string pathString = GetPathCopy().string();
	m_font = LoadFontEx(pathString.c_str(), 64, nullptr, 0);
}
