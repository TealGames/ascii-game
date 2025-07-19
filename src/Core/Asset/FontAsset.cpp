#include "pch.hpp"
#include "Core/Asset/FontAsset.hpp"
#include "Core/Analyzation/Debug.hpp"
#include "Utils/RaylibUtils.hpp"
#include "Utils/IOHandler.hpp"

const std::string FontAsset::EXTENSION = ".ttf";

FontAsset::FontAsset(const std::filesystem::path& path) 
	: Asset(path, false), m_font()
{
	if (!Assert(IO::DoesPathHaveExtension(path, EXTENSION), std::format("Tried to create a font asset from path:{}"
		"but it does not have required font extension:'{}'", path.string(), EXTENSION)))
		return;

	const std::string pathString = path.string();
	m_font = LoadFontEx(pathString.c_str(), 64, nullptr, 0);
}

FontAsset::FontAsset(const Font& font)
	: Asset("", false), m_font(font)
{

}

FontAsset::~FontAsset()
{
	//Note: font uses gpu resources that may not be unloaded and must be done manually
	//and also, even though font is passed by value, font resources are loaded once and passed via pointerss
	UnloadFont(m_font);
}

bool FontAsset::HasValidFont() const
{
	return RaylibUtils::IsValidFont(m_font);
}

Font& FontAsset::GetFontMutable()
{
	if (!Assert(HasValidFont(), std::format("Tried to get font MUTABLE but font is invalid")))
		throw std::invalid_argument("Invalid font state");

	return m_font;
}

const Font& FontAsset::GetFont() const
{
	if (!Assert(HasValidFont(), std::format("Tried to get font but font is invalid")))
		throw std::invalid_argument("Invalid font state");

	return m_font;
}

void FontAsset::UpdateAssetFromFile()
{
	const std::string pathString = GetPathCopy().string();
	m_font = LoadFontEx(pathString.c_str(), 64, nullptr, 0);
}
