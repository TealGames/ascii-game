#include "pch.hpp"
#include "Core/Asset/FontAsset.hpp"
#include "Core/Analyzation/Debug.hpp"
//#include "Utils/RaylibUtils.hpp"
#include "Utils/IOHandler.hpp"

const char* FontAsset::EXTENSION = ".ttf";

FontAsset::FontAsset(const std::filesystem::path& path) 
	: Asset(path, false), m_font()
{
	if (!Assert(path.extension()== EXTENSION, std::format("Tried to create a font asset from path:{} (extension:{})"
		"but it does not have required font extension:'{}'", path.string(), path.extension().string(), EXTENSION)))
		return;

	const std::string pathString = path.string();
	//TODO: load fonts from a path
	//m_font = LoadFontEx(pathString.c_str(), 64, nullptr, 0);
}

FontAsset::FontAsset(const Rendering::Font& font)
	: Asset("", false), m_font(font)
{

}

FontAsset::~FontAsset()
{
	//Note: font uses gpu resources that may not be unloaded and must be done manually
	//and also, even though font is passed by value, font resources are loaded once and passed via pointerss
	//UnloadFont(m_font);
}

bool FontAsset::HasValidFont() const
{
	//TODO: check font validity
	return true;
	//return RaylibUtils::IsValidFont(m_font);
}

Rendering::Font& FontAsset::GetFontMutable()
{
	if (!Assert(HasValidFont(), std::format("Tried to get font MUTABLE but font is invalid")))
		throw std::invalid_argument("Invalid font state");

	return m_font;
}

const Rendering::Font& FontAsset::GetFont() const
{
	if (!Assert(HasValidFont(), std::format("Tried to get font but font is invalid")))
		throw std::invalid_argument("Invalid font state");

	return m_font;
}

void FontAsset::UpdateAssetFromFile()
{
	const std::string pathString = GetPathCopy().string();
	//TODO: load font
	//m_font = LoadFontEx(pathString.c_str(), 64, nullptr, 0);
}
