#include "pch.hpp"
#include "GlobalInitializer.hpp"
#include "StaticReferenceGlobals.hpp"

GlobalInitializer::GlobalInitializer(AssetManagement::AssetManager& assetManager)
{
	StaticReferenceGlobals::Init(assetManager);
}

