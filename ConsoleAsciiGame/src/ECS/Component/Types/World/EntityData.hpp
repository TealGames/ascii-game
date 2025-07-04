#pragma once
#include "ECS/Component/Component.hpp"
#include "Core/Analyzation/Debug.hpp"
#include "ECS/Entity/EntityRegistry.hpp"
#include "ECS/Component/Types/World/TransformData.hpp"
#include "ECS/Component/GlobalComponentInfo.hpp"
#include <format>

class UIPanel;

class UITransformData;
namespace ECS { class EntityRegistry; }

class EntityData;
class EntityData : public Component
{
private:
	ECS::EntityRegistry* m_registry;
	ECS::EntityID m_id;
	/// <summary>
	/// Different from enable property on component. This controls the whole 
	/// entity so while some components may be disabled (including this), active
	/// entity state propogates recursively and can not be enabled until parent is enabled
	/// </summary>
	bool m_isActive;

	std::vector<Component*> m_components;

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
	requires std::is_base_of_v<Component, T>
	void ComponentRequirementCheck()
	{
		if (!GlobalComponentInfo::PassesComponentRequirementCheck(*this, typeid(T)))
		{
			Assert(false, std::format("Attempted to add a component of type:'{}' to entity:{} "
				"but it does not have the required components", Utils::GetTypeName<T>(), ToString()));
			throw std::invalid_argument("Missing required components");
		}
	}

	template<typename T>
	requires std::is_base_of_v<Component, T>
	bool TryInvokePostAddAction()
	{
		return GlobalComponentInfo::InvokePostAddAction(*this, typeid(T));
	}

	/// <summary>
	/// Adds the component WITHOUT any has component check. Note: add component on registry
	/// call can result in undefined behavior and/or exceptions if the component exists already
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <returns></returns>
	template<typename T>
	requires std::is_base_of_v<Component, T>
	T& AddComponentUnsafe(const T& component)
	{
		if (m_Name=="InputField" && FormatComponentName(typeid(T))== "UIPanel")
		{
			//LogWarning("Reached a point here");
		}

		ComponentRequirementCheck<T>();

		T& addedComponent = m_registry->AddComponent<T>(m_id, component);
		Component* componentData = &addedComponent;
		componentData->m_entity = this;
		componentData->InitFields();

		m_components.emplace_back(componentData);
		TryInvokePostAddAction<T>();
		return addedComponent;
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

	bool IsEntityActive() const;
	/// <summary>
	/// Will attempt to enable this entity. If the parent is not enabled, it means this 
	/// entity can not be enabled due to deactive state propagating to the children
	/// </summary>
	/// <returns></returns>
	bool TryActivateEntity();
	/// <summary>
	/// Will disable this entity (different from disabling certain component since all components are essentially 
	/// deactivated and will not be updated) and all children entities as well
	/// </summary>
	void DeactivateEntity();
	bool TrySetEntityActive(const bool active);

	/// <summary>
	/// Will add the component with data to this entity
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="component"></param>
	/// <returns></returns>
	template<typename T>
	requires std::is_base_of_v<Component, T>
	T& AddComponent(const T& component)
	{
		if (!Assert(!HasComponent<T>(), std::format("Tried to add component of type: '{}' to '{}' "
			"but it already has this type (and duplicates are not allowed)", FormatComponentName(typeid(T)), ToString())))
			throw std::invalid_argument("Attempted to add duplicate component");

		return AddComponentUnsafe<T>(component);
		
		//LogWarning(std::format("Created component is:{}", createdComponent.ToString()));
	}
	/// <summary>
	/// Will add the component of type to this entity using no argument/default constructor
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <returns></returns>
	template<typename T>
	requires std::is_base_of_v<Component, T>&& std::is_default_constructible_v<T>
	T& AddComponent() 
	{ 
		return AddComponent<T>(T()); 
		//LogWarning(std::format("Added component no arguments is:{}", addedComponent.ToString()));
	}

	/// <summary>
	/// If the component already exists, will set its values to that of the component argument
	/// Otherwise, will return null
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="component"></param>
	/// <returns></returns>
	template<typename T>
	requires std::is_base_of_v<Component, T>
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
	requires std::is_base_of_v<Component, T>
	T* AddOrSetComponent(const T& component)
	{
		//Note: we do NOT want to use the registry version because we want to invoke entity data's 
		//add component with the extra steps/actions
		if (HasComponent<T>(m_id)) return TrySetComponent<T>(component);
		return AddComponentUnsafe<T>(component);
	}

	template<typename T>
	requires std::is_base_of_v<Component, T>
	bool HasComponent() const
	{
		return m_registry->HasComponent<T>(m_id);
	}

	bool HasComponent(const std::string& targetComponentName) const;
	bool HasComponent(const std::type_info& typeInfo) const;

	template<typename T>
	requires std::is_base_of_v<Component, T>
	T* TryGetComponentMutable() { return m_registry->TryGetComponentMutable<T>(m_id); }

	template<typename T>
	requires std::is_base_of_v<Component, T>
	const T* TryGetComponent() const { return m_registry->TryGetComponent<T>(m_id); }

	template<typename T>
	requires std::is_base_of_v<Component, T>
	T& GetOrAddComponentMutable(const T& component)
	{
		T* maybeComponent = TryGetComponentMutable<T>();
		if (maybeComponent != nullptr) return *maybeComponent;
		return AddComponentUnsafe<T>(component);
	}
	template<typename T>
	requires std::is_base_of_v<Component, T>&& std::is_default_constructible_v<T>
	T& GetOrAddComponentMutable()
	{
		return GetOrAddComponentMutable(T());
	}

	/// <summary>
	/// Will return a list of all components of this entity that contain the base
	/// type T
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <returns></returns>
	template<typename T>
	requires std::is_base_of_v<Component, T>
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

	const Component* TryGetComponentAtIndex(const size_t& index) const;
	Component* TryGetComponentAtIndexMutable(const size_t& index);
	const Component* TryGetComponentWithName(const std::string& name) const;
	Component* TryGetComponentWithNameMutable(const std::string& name);

	size_t TryGetIndexOfComponent(const Component* component) const;
	size_t TryGetIndexOfComponent(const std::string& componentName) const;

	std::string TryGetComponentName(const Component* component) const;
	const std::type_info& GetComponentType(const Component* component) const;

	/// <summary>
	/// Will return all components as base type for this entity. 
	/// Note: even thoguh it is const, since each element is a pointer, the component
	/// is modifiable, but no other components can be added/removed/changed
	/// </summary>
	/// <returns></returns>
	std::vector<Component*>& GetAllComponentsMutable();

	EntityData* GetParentMutable();
	const EntityData* GetParent() const;
	bool HasParent() const;
	EntityData* GetHighestParentMutable();
	const EntityData* GetHighestParent() const;
	int GetChildCount() const;

	EntityData& CreateChild(const std::string& name, const TransformData& transform);
	EntityData& CreateChild(const std::string& name);

	/// <summary>
	/// Creates a child with name, transform and variable other comonents and returns a tuple of the entity and components created
	/// Note: while it would be better to return a tuple of refs, we do not do that since we want to allow for std::tie() for assigning existing
	/// component/entity vars, which most of the type are pointers due to some needing to be used in vectors/collections which cannot have references
	/// </summary>
	/// <typeparam name="...ComponentT"></typeparam>
	/// <param name="name"></param>
	/// <param name="transform"></param>
	/// <param name="...components"></param>
	/// <returns></returns>
	template<typename ...ComponentT>
	requires Utils::AllSameBaseType<Component, ComponentT...>
	std::tuple<EntityData*, ComponentT*...> CreateChild(const std::string& name, const TransformData& transform, const ComponentT&... components)
	{
		EntityData& entity = CreateChild(name, transform);
		return std::make_tuple<EntityData*, ComponentT*...>(&entity, (&entity.AddComponent<ComponentT>(components))...);
	}

	template<typename ...ComponentT>
	requires Utils::AllSameBaseType<Component, ComponentT...>
	std::tuple<EntityData*, UITransformData*, ComponentT*...> CreateChildUI(const std::string& name, const TransformData& transform, 
		const UITransformData& uitransform, const ComponentT&... components)
	{
		return CreateChild<UITransformData, ComponentT...>(name, transform, uitransform, components...);
	}
	template<typename ...ComponentT>
	requires Utils::AllSameBaseType<Component, ComponentT...>
	std::tuple<EntityData*, UITransformData*, ComponentT*...> CreateChildUI(const std::string& name, const ComponentT&... components)
	{
		return CreateChildUI<ComponentT...>(name, TransformData(), UITransformData(), components...);
	}

	/// <summary>
	/// Adds the element as a child and returns its child index
	/// </summary>
	/// <param name="element"></param>
	/// <returns></returns>
	size_t PushChild(EntityData& entity);

	EntityData* TryPopChild(const ECS::EntityID id);
	template<typename T>
	requires std::is_base_of_v<Component, T>
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
	requires std::is_base_of_v<Component, T>
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
	size_t GetChildIndex(const ECS::EntityID id) const;
	bool HasChild(const ECS::EntityID id) const;

	template<typename T>
	requires std::is_base_of_v<Component, T>
	T* TryGetChildComponentAtMutable(const size_t index)
	{
		if (index >= m_childrenIds.size()) return nullptr;
		return m_registry->TryGetComponentMutable<T>(m_childrenIds[index]);
	}
	template<typename T>
	requires std::is_base_of_v<Component, T>
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
	requires std::is_base_of_v<Component, T>
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
	requires std::is_base_of_v<Component, T>
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
	requires std::is_base_of_v<Component, T>
	T* FindComponentRecursiveMutable(const ECS::EntityID id)
	{
		if (m_id == id) return m_registry->TryGetComponentMutable<T>(m_id);
		if (m_childrenIds.empty()) return nullptr;

		T* foundComponent = nullptr;
		T* currChild = nullptr;
		for (const auto& childId : m_childrenIds)
		{
			foundComponent = currChild->GetEntityMutable().FindComponentRecursiveMutable<T>(id);
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
	requires std::is_base_of_v<Component, T>
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
				return m_registry->TryGetComponentMutable<T>(m_id);
			}
			foundComponent = currChild->GetEntityMutable().FindParentComponentRecursiveMutable<T>(id, childIndex);
			if (foundComponent != nullptr) return foundComponent;
		}
		return nullptr;
	}

	void InitFields() override;

	std::string ToString() const override;

	void Deserialize(const Json& json) override;
	Json Serialize() override;

	bool operator==(const EntityData& other) const;
};

template <typename... Types>
requires Utils::AllSameBaseType<Component, Types...>
ValidationAction CreateRequiredComponentFunction(const Types&... components) {
	return [components...](EntityData& entity) -> bool
		{
			bool passesRequiredCheck = true;
			(([&]() -> void
				{
					if (!passesRequiredCheck) return;
					if (entity.HasComponent<Types>()) return;

					if (EntityData::ADD_REQUIRED_COMPONENTS)
					{
						entity.AddComponent<Types>(components);
						/*LogWarning(std::format("Added missing component of type:{} to entity:{}", 
							Utils::GetTypeName<Types>(), entity.ToString()));*/
					}
					else
					{
						LogError(std::format("Entity:{} is missing required component of type:{}",
							entity.ToString(), Utils::GetTypeName<Types>()));
						passesRequiredCheck = false;
					}

				}()), ...);
			return passesRequiredCheck;
		};
}

template <typename... Types>
ValidationAction CreateRequiredComponentFunction()
{
	return CreateRequiredComponentFunction<Types...>(Types()...);
}

