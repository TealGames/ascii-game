#include "pch.hpp"
#include "ECS/Systems/Types/UI/UITextureSystem.hpp"
#include "ECS/Component/GlobalComponentInfo.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "ECS/Component/Types/UI/UITextureComponent.hpp"
#include "ECS/Component/Types/UI/UIRendererComponent.hpp"

namespace Engine::UI
{
	UITextureSystem::UITextureSystem() {}
	void UITextureSystem::Init()
	{
		ECS::GlobalComponentInfo::AddComponentInfo(typeid(UITextureComponent),
			ECS::ComponentInfo(ECS::CreateComponentTypeInfo<UIRendererComponent>(), CreateRequiredComponentFunction(UIRendererComponent()),
				[](ECS::EntityData& entity)-> void
				{
					//if (entity.m_Name == "OnStateTexture") LogError(std::format("Entity has renderer:{}", entity.HasComponent<UIRendererComponent>()));
					entity.TryGetComponentMutable<UITextureComponent>()->m_renderer = entity.TryGetComponentMutable<UIRendererComponent>();
				}));
	}
}
