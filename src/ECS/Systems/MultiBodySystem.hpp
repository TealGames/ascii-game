#pragma once
#include <vector>
#include "Utils/TemplateConcepts.hpp"
#include "ECS/Entity/EntityRegistry.hpp"
#include "ECS/Component/Types/World/CameraComponent.hpp"
#include "ECS/Component/Types/World/EntityComponent.hpp"

namespace ECS
{
	template<typename T, typename TInvocable>
	concept IsComponentInvocableType = Utils::IsInvocableType<void, TInvocable, T&>;

	template<typename T, typename TInvocable>
	requires (std::is_base_of_v<Component, T> && IsComponentInvocableType<T, TInvocable>)
	void OperateOnComponents(EntityRegistry& registry, const ComponentStateFlag flags, TInvocable&& action)
	{
		if (flags == ComponentStateFlag::None)
			return;

		auto view = registry.GetInternalRegistry().view<T>();
		for (auto entityId : view)
		{
			T* component = registry.TryGetComponentMutable<T>(entityId);
			Component* componentBase = static_cast<Component*>(component);
			if (componentBase == nullptr || !Utils::HasFlagAll(flags, componentBase->GetStateFlags())) 
				continue;

			action(*component);
		}
	}

	template<typename T>
	requires std::is_base_of_v<Component, T>
	void GetRegistryComponentsMutable(EntityRegistry& registry, const ComponentStateFlag flags, std::vector<T*>& inputVector)
	{
		return OperateOnComponents<T>(registry, flags,
			[&inputVector](T& component) -> void 
			{ 
				inputVector.push_back(&component);
			});
	}
}
