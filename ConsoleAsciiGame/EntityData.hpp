#pragma once
#include "ComponentData.hpp"
#include "Debug.hpp"
#include "EntityRegistry.hpp"
#include "TransformData.hpp"
#include "GlobalComponentInfo.hpp"

namespace ECS { class EntityRegistry; }

class EntityData : public ComponentData
{
private:
	ECS::EntityRegistry* m_registry;
	ECS::EntityID m_id;

	std::vector<ComponentData*> m_components;

	//TODO: for performance (if actually boosts), we can store pointers to entity data rather than the id
	ECS::EntityID m_parentId;
	//TODO: the children ids should be sorted so we can use binary search
	std::vector<ECS::EntityID> m_childrenIds;
public:
	//TODO: is this really the best location for something like this?
	static const char* GLOBAL_SCENE_NAME;
	/// <summary>
	/// If true, will force add missing required components with default values
	/// </summary>
	static constexpr bool ADD_REQUIRED_COMPONENTS = true;

	std::string m_SceneName;
	std::string m_Name;

	bool m_Active;
	/// <summary>
	/// If true, will be serialized in the scene asset so it can be deserialized
	/// again with the same data between gameplay sessions
	/// </summary>
	bool m_IsSerializable;

	Event<void, EntityData*> m_OnFarthestChildElementAttached;

private:
	EntityData(ECS::EntityRegistry* registry, const ECS::EntityID id,
		const std::string& name, const std::string& sceneName, ECS::EntityID parentId);

	template<typename T>
	requires std::is_base_of_v<ComponentData, T>
	void ComponentRequirementCheck()
	{
		if (!GlobalComponentInfo::PassesComponentRequirementCheck(*this, *typeid(T)))
		{
			Assert(false, std::format("Attempted to add a component of type:{} to entity:{} "
				"but it does not have the required components", Utils::GetTypeName<T>(), ToString()));
			throw std::invalid_argument("Missing required components");
		}
	}

	/// <summary>
	/// Adds the component WITHOUT any has component check. Note: add component on registry
	/// call can result in undefined behavior and/or exceptions if the component exists already
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <returns></returns>
	template<typename T>
	requires std::is_base_of_v<ComponentData, T>
	T& AddComponentUnsafe()
	{
		T& result = m_registry->AddComponent<T>(component);
		ComponentData* componentData = &result;
		componentData->m_entity = this;
		componentData->InitFields();

		m_components.emplace_back(componentData);
		return result;
	}

public:
	EntityData();
	EntityData(ECS::EntityRegistry& registry, const ECS::EntityID id, 
		const std::string& name, const std::string& sceneName="", EntityData* parent = nullptr);

	ECS::EntityID GetId() const;
	std::string ToStringId() const;

	bool IsGlobal() const;
	ECS::EntityRegistry& GetRegistryMutable();
	const ECS::EntityRegistry& GetRegistry() const;

	TransformData& GetTransformMutable();
	const TransformData& GetTransform() const;

	template<typename T, typename... Args>
	requires std::is_base_of_v<ComponentData, T>
	T& AddComponent(Args&&... args)
	{
		ComponentRequirementCheck<T>();

		T& result = m_registry->AddComponent<T>(m_id, std::forward<Args>(args)...);
		ComponentData* componentData = &result;
		componentData->m_entity = this;
		componentData->InitFields();

		m_components.emplace_back(componentData);
		return result;
	}
	/// <summary>
	/// Will add the component with data to this entity
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="component"></param>
	/// <returns></returns>
	template<typename T>
	requires std::is_base_of_v<ComponentData, T>
	T& AddComponent(const T& component)
	{
		ComponentRequirementCheck<T>();

		if (!Assert(this, !HasComponent<T>(), std::format("Tried to add component of type: {} to {} "
			"but it already has this type (and duplicates are not allowed)", typeid(T).name(),
			ToString()))) throw std::invalid_argument("Attempted to add duplicate component");

		AddComponentUnsafe<T>(component);
	}
	/// <summary>
	/// Will add the component of type to this entity using no argument/default constructor
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <returns></returns>
	template<typename T>
	requires std::is_base_of_v<ComponentData, T>&& std::is_default_constructible_v<T>
	T& AddComponent() { return AddComponent<T>(T()); }

	/// <summary>
	/// If the component already exists, will set its values to that of the component argument
	/// Otherwise, will return null
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="component"></param>
	/// <returns></returns>
	template<typename T>
	requires std::is_base_of_v<ComponentData, T>
	T* TrySetComponent(const T& component)
	{
		return m_registry.TrySetComponent<T>(m_id, component);
	}
	/// <summary>
	/// If the component exists, its value will be mutated to match the values of 
	/// tho component argument. Otherwise, the component will be added.
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="component"></param>
	/// <returns></returns>
	template<typename T>
	requires std::is_base_of_v<ComponentData, T>
	T* AddOrSetComponent(const T& component)
	{
		//Note: we do NOT want to use the registry version because we want to invoke entity data's 
		//add component with the extra steps/actions
		if (HasComponent<T>(m_id)) return TrySetComponent<T>(component);
		return AddComponentUnsafe<T>(component);
	}

	template<typename T>
	requires std::is_base_of_v<ComponentData, T>
	bool HasComponent() const
	{
		return m_registry->HasComponent<T>(m_id);
	}

	bool HasComponent(const std::string& targetComponentName) const;
	bool HasComponent(const std::type_info& typeInfo) const;

	template<typename T>
	requires std::is_base_of_v<ComponentData, T>
	T* TryGetComponentMutable() { return m_registry->TryGetComponentMutable(m_id); }

	template<typename T>
	requires std::is_base_of_v<ComponentData, T>
	const T* TryGetComponent() const { return m_registry->TryGetComponent(m_id); }

	/// <summary>
	/// Will return a list of all components of this entity that contain the base
	/// type T
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <returns></returns>
	template<typename T>
	requires std::is_base_of_v<ComponentData, T>
	std::vector<T*> TryGetComponentsWithBaseMutable()
	{
		if (m_components.empty()) return {};

		std::vector<T*> components = {};
		for (auto& component : m_components)
		{
			if (T* maybeTComponent = dynamic_cast<T*>(*component))
			{
				components.push_back(maybeTComponent);
			}
		}
		return components;
	}

	const ComponentData* TryGetComponentAtIndex(const size_t& index) const;
	ComponentData* TryGetComponentAtIndexMutable(const size_t& index);
	const ComponentData* TryGetComponentWithName(const std::string& name) const;
	ComponentData* TryGetComponentWithNameMutable(const std::string& name);

	size_t TryGetIndexOfComponent(const ComponentData* component) const;
	size_t TryGetIndexOfComponent(const std::string& componentName) const;

	std::string TryGetComponentName(const ComponentData* component) const;
	const std::type_info& GetComponentType(const ComponentData* component) const;

	/// <summary>
	/// Will return all components as base type for this entity. 
	/// Note: even thoguh it is const, since each element is a pointer, the component
	/// is modifiable, but no other components can be added/removed/changed
	/// </summary>
	/// <returns></returns>
	const std::vector<ComponentData*>& GetAllComponentsMutable() const;

	EntityData* GetParentMutable();
	const EntityData* GetParent() const;
	bool HasParent() const;
	EntityData* GetHighestParentMutable();
	const EntityData* GetHighestParent() const;
	size_t GetChildCount() const;

	EntityData& CreateChild(const std::string& name, const TransformData& transform);
	/// <summary>
	/// Adds the element as a child and returns its child index
	/// </summary>
	/// <param name="element"></param>
	/// <returns></returns>
	size_t PushChild(EntityData& entity);

	EntityData* TryPopChild(const ECS::EntityID id);
	template<typename T>
	requires std::is_base_of_v<ComponentData, T>
	T* TryPopChildAs(const ECS::EntityID id)
	{
		if (m_childrenIds.empty() == 0) return nullptr;

		//TDDO: if children are sorted, we could use binary search
		for (size_t i=0; i<m_childrenIds.size(); i++)
		{
			if (m_childrenIds[i] != id) continue;

			m_childrenIds.erase(m_childrenIds.begin() + i);
			return m_registry->TryGetComponentMutable<T>(id);
		}
		return nullptr;
	}

	EntityData* TryPopChildAt(const size_t index);
	template<typename T>
	requires std::is_base_of_v<ComponentData, T>
	T* TryPopChildAtAs(const size_t index)
	{
		if (index >= GetChildCount()) return nullptr;

		ECS::EntityID poppedId = m_childrenIds[index];
		m_childrenIds.erase(m_childrenIds.begin() + index);
		return m_registry->TryGetComponentMutable<T>(poppedId);
	}

	std::vector<EntityData*> TryPopChildren(const size_t& startIndex, const size_t& count);
	std::vector<EntityData*> PopAllChildren();

	std::vector<EntityData*> GetChildrenMutable();
	std::vector<const EntityData*> GetChildren() const;
	EntityData* TryGetChildEntityAtMutable(const size_t index);
	EntityData* TryGetChildEntityMutable(const ECS::EntityID id);

	template<typename T>
	requires std::is_base_of_v<ComponentData, T>
	T* TryGetChildComponentAtMutable(const size_t index)
	{
		if (index >= m_childrenIds.size()) return nullptr;
		return m_registry->TryGetComponentMutable<T>(m_childrenIds[index]);
	}
	template<typename T>
	requires std::is_base_of_v<ComponentData, T>
	T* TryGetChildComponentMutable(const ECS::EntityID id)
	{
		if (m_childrenIds.empty()) return nullptr;

		for (const auto& childId : m_childrenIds)
		{
			if (childId == id)
				return m_registry->TryGetComponentMutable<T>(id);
		}
		return nullptr;
	}

	template<typename T>
	requires std::is_base_of_v<ComponentData, T>
	std::vector<T*> GetChildrenOfTypeMutable()
	{
		if (m_childrenIds.empty()) return {};

		std::vector<T*> children = {};
		children.reserve(m_childrenIds.size());
		T* currentComponent = nullptr;
		for (const auto& childId : m_childrenIds)
		{
			currentComponent = m_registry->TryGetComponentMutable<T>(childId);
			if (currentComponent == nullptr) continue;
			children.emplace_back(currentComponent);
		}
		return children;
	}
	template<typename T>
	requires std::is_base_of_v<ComponentData, T>
	std::vector<const T*> GetChildrenOfType() const
	{
		if (m_childrenIds.empty()) return {};

		std::vector<const T*> children = {};
		children.reserve(m_childrenIds.size());
		const T* currentComponent = nullptr;
		for (const auto& childId : m_childrenIds)
		{
			currentComponent = m_registry->TryGetComponent<T>(childId);
			if (currentComponent == nullptr) continue;
			children.emplace_back(currentComponent);
		}
		return children;
	}

	/// <summary>
	/// Will attempt to find the element from ID starting with THIS id and searching
	/// recursively through all of the children
	/// </summary>
	/// <param name="id"></param>
	/// <returns></returns>
	EntityData* FindEntityRecursiveMutable(const ECS::EntityID id);
	/// <summary>
	/// Same as the entity version but is optimized for the component
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="id"></param>
	/// <returns></returns>
	template<typename T>
	requires std::is_base_of_v<ComponentData, T>
	T* FindComponentRecursiveMutable(const ECS::EntityID id)
	{
		if (m_id == id) return this;
		if (m_childrenIds.empty()) return nullptr;

		T* foundComponent = nullptr;
		T* currChild = nullptr;
		for (const auto& childId : m_childrenIds)
		{
			currChild = m_registry->TryGetComponentMutable<T>(childId);
			if (currChild == nullptr) continue;

			foundComponent = currChild->FindComponentRecursiveMutable<T>(id);
			if (foundComponent != nullptr) return foundComponent;
		}
		return nullptr;
	}

	/// <summary>
	/// Will attempt to find the element that contains a child with the id, beginning with this
	/// parent element. Child index is an out argument that will contain the index of the child of the parent (if found)
	/// </summary>
	/// <param name="id"></param>
	/// <param name="childIndex"></param>
	/// <returns></returns>
	EntityData* FindParentEntityRecursiveMutable(const ECS::EntityID id, size_t* childIndex = nullptr);
	/// <summary>
	/// Same as entity version but is optimized for component
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="id"></param>
	/// <param name="childIndex"></param>
	/// <returns></returns>
	template<typename T>
	requires std::is_base_of_v<ComponentData, T>
	T* FindParentComponentRecursiveMutable(const ECS::EntityID id, size_t* childIndex = nullptr)
	{
		if (m_childrenIds.empty()) return nullptr;

		T* foundComponent = nullptr;
		T* currChild = nullptr;
		for (size_t i = 0; i < m_childrenIds.size(); i++)
		{
			currChild = m_registry->TryGetComponentMutable<T>(m_childrenIds[i]);
			if (currChild == nullptr) continue;

			if (m_childrenIds[i] == id)
			{
				if (childIndex != nullptr) *childIndex = i;
				return this;
			}
			foundComponent = currChild->FindParentComponentRecursiveMutable<T>(id, childIndex);
			if (foundComponent != nullptr) return foundComponent;
		}
		return nullptr;
	}

	void InitFields() override;

	static std::string ToString(const ECS::EntityID& id);
	std::string ToString() const override;

	void Deserialize(const Json& json) override;
	Json Serialize() override;

	bool operator==(const EntityData& other) const;
};

template <typename... Types>
std::function<bool(EntityData&)> RequiredComponentCheck() {
	return [](EntityData& entity) -> bool
		{
			bool isMissingComponent = false;
			(([isMissingComponent]() -> void
				{
					if (isMissingComponent) return;
					if (entity.HasComponent<T>()) return;

					if (ADD_REQUIRED_COMPONENTS)
						entity.AddComponent<Types>();
					else
					{
						Assert(false, std::format("Entity:{} is missing required component of type:{}",
							entity.ToString(), Utils::GetTypeName<Types>()));
						isMissingComponent = true;
					}

				}()), ...);
			return isMissingComponent;
		};
}
