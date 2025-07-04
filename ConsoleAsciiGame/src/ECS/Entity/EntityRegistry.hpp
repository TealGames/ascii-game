#pragma once
#include "ECS/Entity/EntityID.hpp"
#include "Core/Analyzation/Debug.hpp"
#include "Utils/HelperFunctions.hpp"
#include "ECS/Component/Component.hpp"
#include <format>

class EntityData;
class TransformData;

namespace ECS
{
	class EntityRegistry
	{
	private:
		entt::registry m_registry;
	public:

	private:
		template<typename T>
		requires std::is_base_of_v<Component, T>
		bool PassesHasComponentCheck(const EntityID id) const
		{
			if (HasComponent<T>(id))
			{
				LogError(std::format("Tried to add component of type: {} to {} via EntityRegistry"
					"but it already has this type (and duplicates are not allowed)", typeid(T).name(),
					TryGetEntity(id)->ToString()));
				return false;
			}
			return true;
		}

	public:
		EntityRegistry();

		bool IsValidID(const EntityID& id) const;
		EntityData& CreateNewEntity(const std::string& name, const TransformData& transform);

		EntityData* TryGetEntityMutable(const EntityID id);
		const EntityData* TryGetEntity(const EntityID id) const;
		bool HasEntity(const EntityID& id) const;

		entt::registry& GetInternalRegistry();

		template<typename T, typename... Args>
		requires std::is_base_of_v<Component, T>
		T& AddComponent(const EntityID entityId, Args&&... args)
		{
			if (!Assert(IsValidID(entityId), std::format("Attempted to add component via constructor args:'{}' to invalid entity:'{}'",
				FormatComponentName(typeid(T)), ToString(entityId))))
				throw std::invalid_argument("Invalid entity id");

			if (!PassesHasComponentCheck<T>(entityId))
				throw std::invalid_argument("Attempted to add duplicate component");

			return m_registry.emplace<T>(entityId, std::forward<Args>(args)...);
		}
		/// <summary>
		/// Will add the component with data to this entity
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <param name="component"></param>
		/// <returns></returns>
		template<typename T>
		requires std::is_base_of_v<Component, T>
		T& AddComponent(const EntityID entityId, const T& component)
		{
			if (!Assert(IsValidID(entityId), std::format("Attempted to add component via complete object:'{}' to invalid entity:'{}'",
				FormatComponentName(typeid(T)), ToString(entityId))))
				throw std::invalid_argument("Invalid entity id");

			if (!PassesHasComponentCheck<T>(entityId))
				throw std::invalid_argument("Attempted to add duplicate component");

			return m_registry.emplace<T>(entityId, component);
		}

		/// <summary>
		/// Will add the component of type to this entity using no argument/default constructor
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <returns></returns>
		template<typename T>
		requires std::is_base_of_v<Component, T>&& std::is_default_constructible_v<T>
		T& AddComponent(const EntityID entityId) { return AddComponent<T>(entityId, T()); }

		/// <summary>
		/// If component type exists on the entity, will update its values with those of the component
		/// argument. Otherwise, returns null.
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <param name="entityId"></param>
		/// <returns></returns>
		template<typename T>
		requires std::is_base_of_v<Component, T>
		T* TrySetComponent(const EntityID entityId, const T& component)
		{
			if (!Assert(IsValidID(entityId), std::format("Attempted to set component:'{}' to invalid entity:'{}'",
				FormatComponentName(typeid(T)), ToString(entityId))))
				throw std::invalid_argument("Invalid entity id");

			T* component = TryGetComponentMutable<T>(entityId);
			if (component == nullptr) return nullptr;

			*component = component;
			return component;
		}
		template<typename T>
		requires std::is_base_of_v<Component, T>
		T* AddOrSetComponent(const EntityID entityId, const T& component)
		{
			if (!Assert(IsValidID(entityId), std::format("Attempted to add or set component:'{}' to invalid entity:'{}'",
				FormatComponentName(typeid(T)), ToString(entityId))))
				throw std::invalid_argument("Invalid entity id");

			return m_registry.emplace_or_replace(entityId, component);
		}

		template<typename T>
		requires std::is_base_of_v<Component, T>
		bool HasComponent(const EntityID entityId) const
		{
			if (!Assert(IsValidID(entityId), std::format("Attempted to check has component:'{}' to invalid entity:'{}'",
				FormatComponentName(typeid(T)), ToString(entityId))))
				throw std::invalid_argument("Invalid entity id");

			return m_registry.try_get<T>(entityId) != nullptr;
		}

		template<typename T>
		requires std::is_base_of_v<Component, T>
		T* TryGetComponentMutable(const EntityID entityId)
		{
			if (!Assert(IsValidID(entityId), std::format("Attempted to get component MUTABLE:'{}' to invalid entity:'{}'",
				FormatComponentName(typeid(T)), ToString(entityId))))
				throw std::invalid_argument("Invalid entity id");

			return m_registry.try_get<T>(entityId);
		}

		template<typename T>
		requires std::is_base_of_v<Component, T>
		const T* TryGetComponent(const EntityID entityId) const
		{
			if (!Assert(IsValidID(entityId), std::format("Attempted to get component:'{}' to invalid entity:'{}'",
				FormatComponentName(typeid(T)), ToString(entityId))))
				throw std::invalid_argument("Invalid entity id");

			return m_registry.try_get<T>(entityId);
		}
	};
}


