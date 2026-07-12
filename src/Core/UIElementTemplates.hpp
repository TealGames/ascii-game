#pragma once
#include <string>
#include <unordered_map>
#include <functional>
#include <tuple>

namespace Engine::Assets { class AssetManager; }
namespace Engine::ECS { class EntityData; }
namespace Engine::UI
{
	class UITransformComponent;
	class UIToggleComponent;
	namespace Templates
	{
		void Init(Assets::AssetManager& assetManager);

		std::tuple<ECS::EntityData*, UITransformComponent*, UIToggleComponent*> 
			CreateDropdownToggleTemplate(ECS::EntityData& parent, const std::string& name);

		std::tuple<ECS::EntityData*, UITransformComponent*, UIToggleComponent*> 
			CreateCheckboxTemplate(ECS::EntityData& parent, const std::string& name);
	}
}

