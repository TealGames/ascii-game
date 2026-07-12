#include "pch.hpp"
#include "UIElementTemplates.hpp"
#include "Core/Asset/AssetManager.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "ECS/Component/Types/UI/UITransformComponent.hpp"
#include "ECS/Component/Types/UI/UIToggleComponent.hpp"
#include "ECS/Component/Types/UI/UIPanelComponent.hpp"
#include "Editor/EditorStyles.hpp"
#include "Core/Asset/TextureAsset.hpp"
#include "ECS/Component/Types/UI/UITextureComponent.hpp"

namespace Engine::UI::Templates
{
	static Assets::AssetManager* AssetManager = nullptr;
	void Init(Assets::AssetManager& assetManager)
	{
		AssetManager = &assetManager;
	}

	std::tuple<ECS::EntityData*, UITransformComponent*, UIToggleComponent*> CreateDropdownToggleTemplate(ECS::EntityData& parent, const std::string& name)
	{
		auto [toggleEntity, toggleTransform] = parent.CreateChildUI(name);
		UIPanelComponent& panel = toggleEntity->AddComponent(UIPanelComponent());
		UIToggleComponent& toggle = toggleEntity->AddComponent(UIToggleComponent(false, Editor::Styles::GetToggleStyle(), nullptr, nullptr, &panel));

		if (AssetManager == nullptr)
		{
			LogError(std::format("Attempted to create checkbox template named:'{}' but asset manager is null", name));
			throw std::invalid_argument("Invalid asset manager state");
		}

		auto [onTexEntity, onTexTransform] = toggleEntity->CreateChildUI("OnStateTexture");
		UITextureComponent& onTex = onTexEntity->AddComponent(UITextureComponent(*AssetManager->TryGetTypeAssetFromPathMutable
			<Rendering::TextureAsset>("textures/dropdown_icon_toggled.png")));
		toggle.m_OnValueSet.AddListener([onTexEntity](bool isChecked)-> void
			{
				onTexEntity->GetTransformMutable().GetLocalScaleMutable().m_Y *= -1;
			});

		return std::make_tuple(toggleEntity, toggleTransform, &toggle);
	}

	std::tuple<ECS::EntityData*, UITransformComponent*, UIToggleComponent*> CreateCheckboxTemplate(ECS::EntityData& parent, const std::string& name)
	{
		auto [toggleEntity, toggleTransform] = parent.CreateChildUI(name);
		UIPanelComponent& panel = toggleEntity->AddComponent(UIPanelComponent());
		UIToggleComponent& toggle = toggleEntity->AddComponent(UIToggleComponent(false, Editor::Styles::GetToggleStyle(), nullptr, nullptr, &panel));

		if (AssetManager == nullptr)
		{
			LogError(std::format("Attempted to create checkbox template named:'{}' but asset manager is null", name));
			throw std::invalid_argument("Invalid asset manager state");
		}

		auto [onTexEntity, onTexTransform] = toggleEntity->CreateChildUI("OnStateTexture");
		UITextureComponent& onTex = onTexEntity->AddComponent(UITextureComponent(*AssetManager->
			TryGetTypeAssetFromPathMutable<Rendering::TextureAsset>("textures/x_icon.png")));
		toggle.m_OnValueSet.AddListener([onTexEntity](bool isChecked)-> void
			{
				onTexEntity->GetTransformMutable().GetLocalScaleMutable().m_Y *= -1;
			});

		toggle.SetStateTextures(&onTex, nullptr);
		return std::make_tuple(toggleEntity, toggleTransform, &toggle);
	}
}
