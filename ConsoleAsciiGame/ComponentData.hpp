#pragma once
#include <vector>
#include "ComponentField.hpp"
#include "IJsonSerializable.hpp"
#include "IValidateable.hpp"

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

namespace ECS
{
	class Entity;
}

class ComponentData: public IJsonSerializable, public IValidateable
{
private:
	HighestDependecyLevel m_dependencyLevel;
public:

private:
public:
	
	/// <summary>
	///	If true, it signifies that this component data has been mutatated this frame. 
	/// This flag is useful for component systems that may use last frame buffers for
	/// optimization and can be checked to make sure we do not use last frame data if it has changed 
	/// this past frame
	/// </summary>
	bool m_MutatedThisFrame;

	//Guaranteed to not be nullptr (but cant be a ref to allow it to be set
	//not on construction without the need to have it as constructor arg
	ECS::Entity* m_Entity;

	std::vector<ComponentField> m_Fields;

	ComponentData(const HighestDependecyLevel& dependency);
	virtual ~ComponentData() = default;
	ECS::Entity& GetEntitySafeMutable();
	const ECS::Entity& GetEntitySafe() const;

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

	HighestDependecyLevel GetDependencyLevel() const;

	virtual void Deserialize(const Json& json) = 0;
	virtual Json Serialize() = 0;

	virtual bool Validate() override;

	virtual std::string ToString() const;
};
	