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
	//TODOL: also add a static flag so we can optimize some components
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

	virtual void InitFields();
	const std::vector<ComponentField>& GetFields() const;

	HighestDependecyLevel GetDependencyLevel() const;

	virtual void Deserialize(const Json& json) = 0;
	virtual Json Serialize() = 0;

	virtual bool Validate() override;

	virtual std::string ToString() const;
};
	