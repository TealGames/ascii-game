#include "pch.hpp"
#include "Core/Asset/TestAsset.hpp"
#include "Utils/HelperFunctions.hpp"
#include "Utils/ToStringFunctions.hpp"
#include "Utils/IOHandler.hpp"

namespace Engine
{
	const std::array<std::string_view,1> TestAsset::EXTENSIONS = {""};

	TestAsset::TestAsset(const std::filesystem::path& path) 
		: Asset(path) 
	{
		ASSET_EXTENSION_CHECK
	}

	TestAsset::~TestAsset()
	{
	}

	void TestAsset::UpdateAssetFromFile()
	{
	}
}