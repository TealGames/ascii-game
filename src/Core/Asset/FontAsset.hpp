#pragma once
#include "Core/Asset/Asset.hpp"
#include "Core/Rendering/Font.hpp"
#include <optional>

namespace Engine::Rendering
{
	class FontAsset : public Assets::Asset
	{
	private:
		Font m_font;

	public:
		static const std::array<std::string_view,1> EXTENSIONS;

	private:
	public:
		FontAsset(const std::filesystem::path& path);
		FontAsset(const Font& font);
		~FontAsset();

		Font& GetFontMutable();
		const Font& GetFont() const;
		bool HasValidFont() const;

		void UpdateAssetFromFile() override;
	};
}


