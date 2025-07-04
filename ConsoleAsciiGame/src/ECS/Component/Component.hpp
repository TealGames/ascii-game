#pragma once
#include <vector>
#include "ECS/Component/ComponentField.hpp"
#include "Core/Serialization/IJsonSerializable.hpp"
#include "Core/IValidateable.hpp"
#include "ECS/Entity/EntityID.hpp"

enum class HighestDependecyLevel
{
	/// <summary>
	/// This means the component does not depend on any other outside data
	/// </summary>
	None,
	/// <summary>
	/// This means the component requires another SIBLING component 
	/// (a component on the same entity as this component)
	/// </summary>
	SiblingComponent,
	/// <summary>
	/// This means the component requires ether a component on ANOTHER entity
	/// or the full entity object
	/// </summary>
	Entity,
};

/// <summary>
/// This means the component requires ether a component on ANOTHER entity
/// or the full entity object
/// </summary>
constexpr const char* ENTITY_DEPENDENCY_FLAG = "Entity";

class EntityData;
class TransformData;

class Component
{
private:
	//Guaranteed to not be nullptr (but cant be a ref to allow it to be set
	//not on construction without the need to have it as constructor arg
	EntityData* m_entity;
	//HighestDependecyLevel m_dependencyLevel;
public:
	friend class EntityData;
	/// <summary>
	///	If true, it signifies that this component data has been mutatated this frame. 
	/// This flag is useful for component systems that may use last frame buffers for
	/// optimization and can be checked to make sure we do not use last frame data if it has changed 
	/// this past frame
	/// </summary>
	bool m_MutatedThisFrame;
	/// <summary>
	/// If true, component is enabled, otherwise it is disabled. 
	/// Note: this does not change anything directly as components need to check this 
	/// flag themselves
	/// </summary>
	bool m_IsEnabled;
	//TODO: the fields for a component should be placed into a registry
	std::vector<ComponentField> m_Fields;

private:
public:
	Component();
	virtual ~Component() = default;

	EntityData& GetEntityMutable();
	const EntityData& GetEntity() const;
	TransformData& GetTransformMutable();
	const TransformData& GetTransform() const;

	ECS::EntityID GetEntityID() const;

	std::vector<ComponentField>& GetFieldsMutable();
	/// <summary>
	/// A function that creates the fields that can be used by outside sources.
	/// Note: this must be separate from constructor because base class constructor defaults initializes them
	/// (to prevent derived setting it and being reverted by base class constructor) and because polymorphism
	/// is not established until AFTER constructors are done (therefore base class cannot invoke it)
	/// </summary>
	virtual void InitFields();
	const std::vector<ComponentField>& GetFields() const;
	ComponentField* TryGetFieldMutable(const std::string& name);
	const ComponentField* TryGetField(const std::string& name) const;
	std::string ToStringFields() const;

	//HighestDependecyLevel GetDependencyLevel() const;
	//virtual std::vector<std::string> GetDependencyFlags() const = 0;
	/*bool DependsOnEntity() const;
	bool DependsOnAnySiblingComponent() const;
	bool HasDependencies() const;
	bool DoesEntityHaveComponentDependencies() const;

	std::vector<std::string> GetComponentDependencies() const;*/

	/*template<typename T>
	bool DependsOnSiblingComponent() const
	{
		if (!DependsOnAnySiblingComponent()) return false;

		const std::string tTypeName = Utils::GetTypeName<T>();
		for (const auto& componentDependency : GetDependencyFlags())
		{
			if (tTypeName == componentDependency) 
				return true;
		}
		return false;
	}*/

	virtual void Deserialize(const Json& json) = 0;
	virtual Json Serialize() = 0;

	virtual bool Validate();

	virtual std::string ToString() const;
};

std::string FormatComponentName(const Component* component);
std::string FormatComponentName(const std::type_info& typeInfo);

template<typename... ComponentTs>
std::vector<const std::type_info*> CreateComponentTypes()
{
	return {(&typeid(ComponentTs))...};
}
	