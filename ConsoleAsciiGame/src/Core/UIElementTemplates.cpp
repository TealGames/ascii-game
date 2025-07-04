#include "pch.hpp"
#include "UIElementTemplates.hpp"
#include "Core/Asset/AssetManager.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "ECS/Component/Types/UI/UITransformData.hpp"
#include "ECS/Component/Types/UI/UIToggleComponent.hpp"
#include "ECS/Component/Types/UI/UIPanel.hpp"
#include "Editor/EditorStyles.hpp"
#include "Core/Asset/TextureAsset.hpp"
#include "ECS/Component/Types/UI/UITextureData.hpp"

static AssetManagement::AssetManager* AssetManager = nullptr;

namespace Templates
{
	void Init(AssetManagement::AssetManager& assetManager)
	{
		AssetManager = &assetManager;
	}

	std::tuple<EntityData*, UITransformData*, UIToggleComponent*> CreateCheckboxTemplate(EntityData& parent, const std::string& name)
	{
		auto [toggleEntity, toggleTransform] = parent.CreateChildUI(name);
		UIToggleComponent& toggle = toggleEntity->AddComponent(UIToggleComponent(false, EditorStyles::GetToggleStyle()));
		UIPanel& panel = toggleEntity->AddComponent(UIPanel(EditorStyles::EDITOR_SECONDARY_BACKGROUND_COLOR));

		auto [onTexEntity, onTexTransform, onTex] = toggleEntity->CreateChildUI("OnStateTexture", 
			UITextureData(*AssetManager->TryGetTypeAssetMutable<TextureAsset>("textures/dropdown_icon_toggled.png")));

		auto [offTexEntity, offTexTransform, offTex] = toggleEntity->CreateChildUI("OffStateTexture",
			UITextureData(*AssetManager->TryGetTypeAssetMutable<TextureAsset>("textures/dropdown_icon_default.png")));

		toggle.SetStateTextures(onTex, offTex);
		return std::make_tuple(toggleEntity, toggleTransform, &toggle);
	}
}
