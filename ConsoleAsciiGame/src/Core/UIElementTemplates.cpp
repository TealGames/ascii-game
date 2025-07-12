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

	std::tuple<EntityData*, UITransformData*, UIToggleComponent*> CreateDropdownToggleTemplate(EntityData& parent, const std::string& name)
	{
		auto [toggleEntity, toggleTransform] = parent.CreateChildUI(name);
		UIToggleComponent& toggle = toggleEntity->AddComponent(UIToggleComponent(false, EditorStyles::GetToggleStyle()));
		UIPanel& panel = toggleEntity->AddComponent(UIPanel(EditorStyles::EDITOR_SECONDARY_BACKGROUND_COLOR));

		if (AssetManager == nullptr)
		{
			LogError(std::format("Attempted to create checkbox template named:'{}' but asset manager is null", name));
			throw std::invalid_argument("Invalid asset manager state");
		}

		auto [onTexEntity, onTexTransform] = toggleEntity->CreateChildUI("OnStateTexture");
		UITextureData& onTex = onTexEntity->AddComponent(UITextureData(*AssetManager->TryGetTypeAssetFromPathMutable<TextureAsset>("textures/dropdown_icon_toggled.png")));
		toggle.m_OnValueSet.AddListener([onTexEntity](bool isChecked)-> void
			{
				float oldScaleY = onTexEntity->GetTransformMutable().GetLocalScale().m_Y;
				onTexEntity->GetTransformMutable().SetLocalScale({ 1, -oldScaleY });
			});

		return std::make_tuple(toggleEntity, toggleTransform, &toggle);
	}

	std::tuple<EntityData*, UITransformData*, UIToggleComponent*> CreateCheckboxTemplate(EntityData& parent, const std::string& name)
	{
		auto [toggleEntity, toggleTransform] = parent.CreateChildUI(name);
		UIToggleComponent& toggle = toggleEntity->AddComponent(UIToggleComponent(false, EditorStyles::GetToggleStyle()));
		UIPanel& panel = toggleEntity->AddComponent(UIPanel(EditorStyles::EDITOR_SECONDARY_BACKGROUND_COLOR));

		if (AssetManager == nullptr)
		{
			LogError(std::format("Attempted to create checkbox template named:'{}' but asset manager is null", name));
			throw std::invalid_argument("Invalid asset manager state");
		}

		auto [onTexEntity, onTexTransform] = toggleEntity->CreateChildUI("OnStateTexture");
		UITextureData& onTex = onTexEntity->AddComponent(UITextureData(*AssetManager->TryGetTypeAssetFromPathMutable<TextureAsset>("textures/x_icon.png")));
		toggle.m_OnValueSet.AddListener([onTexEntity](bool isChecked)-> void
			{
				float oldScaleY = onTexEntity->GetTransformMutable().GetLocalScale().m_Y;
				onTexEntity->GetTransformMutable().SetLocalScale({ 1, -oldScaleY });
			});

		toggle.SetStateTextures(&onTex, nullptr);
		return std::make_tuple(toggleEntity, toggleTransform, &toggle);
	}
}
