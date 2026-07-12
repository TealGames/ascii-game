#include "pch.hpp"
#include "Core/Asset/FontAsset.hpp"
#include "Utils/Debug.hpp"
#include "Utils/IOHandler.hpp"
#include "Utils/HelperFunctions.hpp"
#include "Utils/ToStringFunctions.hpp"

namespace Engine::Rendering
{
	const std::array<std::string_view,1> FontAsset::EXTENSIONS = {".ttf"};

	FontAsset::FontAsset(const std::filesystem::path& path)
		: Asset(path), m_font()
	{
		ASSET_EXTENSION_CHECK
		const std::string pathString = path.string();
		//TODO: load fonts from a path
		//m_font = LoadFontEx(pathString.c_str(), 64, nullptr, 0);
	}

	FontAsset::FontAsset(const Rendering::Font& font)
		: Asset(""), m_font(font)
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
		if (!Assert(HasValidFont(), "Tried to get font MUTABLE but font is invalid"))
			throw std::invalid_argument("Invalid font state");

		return m_font;
	}

	const Rendering::Font& FontAsset::GetFont() const
	{
		if (!Assert(HasValidFont(), "Tried to get font but font is invalid"))
			throw std::invalid_argument("Invalid font state");

		return m_font;
	}

	void FontAsset::UpdateAssetFromFile()
	{
		const std::string pathString = GetAbsolutePathCopy().string();
		//TODO: load font
		//m_font = LoadFontEx(pathString.c_str(), 64, nullptr, 0);
	}
}
