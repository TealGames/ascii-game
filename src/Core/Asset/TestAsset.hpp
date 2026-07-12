#pragma once
#include "Core/Asset/Asset.hpp"

namespace Engine
{
	class TestAsset : public Assets::Asset
	{
	private:
	public:
		static const std::array<std::string_view,1> EXTENSIONS;

	private:
	public:
		TestAsset(const std::filesystem::path& path);
		~TestAsset();

		void UpdateAssetFromFile() override;
	};
}