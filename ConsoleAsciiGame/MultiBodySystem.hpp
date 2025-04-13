#pragma once
#include <vector>
#include "entt/entt.hpp"
#include "Entity.hpp"
#include "CameraData.hpp"

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
	requires ECS::IsComponent<T>
	void OperateOnActiveComponents(entt::registry& registry, const std::function<ECS::Entity*(const ECS::EntityID&)>& entityFromIDFunc,
		const std::function<void(T&, ECS::Entity&)>& action)
	{
		auto view = registry.view<T>();
		for (auto entityId : view)
		{
			ECS::Entity* entityPtr = entityFromIDFunc(entityId);
			if (!Assert(entityPtr != nullptr, std::format("Tried to operate on component type: {} "
				"but failed to retrieve entity with ID: {} (it probably does not exist in the scene)",
				typeid(T).name(), ECS::Entity::ToString(entityId))))
				return;

			if (!entityPtr->m_Active) continue;

			T* component = &(view.get<T>(entityId));
			ComponentData* componentBase = static_cast<ComponentData*>(component);
			if (!componentBase->m_IsEnabled) continue;

			action(*component, *entityPtr);
		}
	}

	template<typename T>
	requires ECS::IsComponent<T>
	void GetRegistryComponentsMutable(entt::registry& registry, const std::function<ECS::Entity*(const ECS::EntityID&)>& entityFromIDFunc, 
		std::vector<T*>& inputVector)
	{
		auto view = registry.view<T>();
		for (auto entityId : view)
		{
			ECS::Entity* entityPtr = entityFromIDFunc(entityId);
			if (!Assert(entityPtr != nullptr, std::format("Tried to get components of type: {} "
				"but failed to retrieve entity with ID: {} (it probably does not exist in the scene)",
				typeid(T).name(), ECS::Entity::ToString(entityId))))
				return;

			if (!entityPtr->m_Active) continue;

			T* component = &(view.get<T>(entityId));
			ComponentData* componentBase = static_cast<ComponentData*>(component);
			if (!componentBase->m_IsEnabled) continue;

			inputVector.push_back(component);
		}
	}
}
