#pragma once
#include <vector>
#include "EntityRegistry.hpp"
#include "CameraData.hpp"
#include "EntityData.hpp"

class Scene;
namespace ECS
{
	class MultiBodySystem
	{
	private:
	public:

	private:
	public:
		virtual void SystemUpdate(Scene& scene, CameraData& mainCamera, const float& deltaTime) = 0;
	};

	/// <summary>
	/// Will iterate throguh every component in a registry and complete the action only if the 
	/// component and entity is active
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="registry"></param>
	/// <param name="entityFromIDFunc"></param>
	/// <param name="action"></param>
	template<typename T>
	requires std::is_base_of_v<Component, T>
	void OperateOnActiveComponents(EntityRegistry& registry, const std::function<void(T&)>& action)
	{
		auto view = registry.GetInternalRegistry().view<T>();
		for (auto entityId : view)
		{
			T* component = registry.TryGetComponentMutable<T>(entityId);
			Component* componentBase = static_cast<Component*>(component);
			if (componentBase == nullptr || !componentBase->m_IsEnabled) continue;

			EntityData& entity = componentBase->GetEntityMutable();
			if (!entity.IsEntityActive()) continue;

			action(*component);
		}
	}

	template<typename T>
	requires std::is_base_of_v<Component, T>
	void GetRegistryComponentsMutable(EntityRegistry& registry, std::vector<T*>& inputVector)
	{
		auto view = registry.GetInternalRegistry().view<T>();
		for (auto entityId : view)
		{
			T* component = registry.TryGetComponentMutable<T>(entityId);
			Component* componentBase = static_cast<Component*>(component);
			if (componentBase==nullptr || !componentBase->m_IsEnabled) continue;

			EntityData& entity = componentBase->GetEntityMutable();
			if (!entity.IsEntityActive()) continue;

			inputVector.push_back(component);
		}
	}
}
