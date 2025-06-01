#pragma once
#include <functional>

class EntityData;
class ComponentData;

/// <summary>
	/// In order for the component to be added, the type of relationship the required component must have in relation
	/// to the component that is being added for the add component operation to work
	/// </summary>
enum class RequiredComponentType
{
	None,
	/// <summary>
	/// The entity itself (Of the component) must contain the required component
	/// </summary>
	Self,
	/// <summary>
	/// It MUST contain a parent AND it must contain the component
	/// </summary>
	Parent,
	/// <summary>
	/// Either it has no parent OR if it does, it MUST have the component
	/// </summary>
	NoParentOrParent,
	/// <summary>
	/// The highest parent (meaning the farthest parent which itself has no parents 
	/// must have the component
	/// </summary>
	HighestParent,
};

/// <summary>
/// The type of object the component needs for it to work properly, but is NOT required for it to be added meaning
/// it may still retain some functionality even if this dependency is not there, but may be in an inactive or erroneous state
/// </summary>
enum class DeppendencyType
{
	None,
	/// <summary>
	/// Needs another SEPARATE entity to work fully/partially. Also, if the component
	/// requires a component on ANOTHER entity to work fully/partially, choose this option
	/// </summary>
	Entity,
	/// <summary>
	/// Needs another component ON THE SAME ENTITY to work fully/partially
	/// </summary>
	Component,
};

using TypeCollection = std::vector<const std::type_info*>;
using ValidationAction = std::function<bool(EntityData& entity)>;
struct ComponentInfo
{
	//RequiredComponentType m_RequiredType;

	DeppendencyType m_DependencyType;
	TypeCollection m_DependentComponents;

	ValidationAction m_ComponentRequirementCheck;

	/*ComponentInfo(const RequiredComponentType requiredType, const TypeCollection& required);
	ComponentInfo(const DeppendencyType dependencyType, const TypeCollection& dependentComponents = {});
	ComponentInfo(const RequiredComponentType requiredType, const TypeCollection& required,
		const DeppendencyType dependencyType, const TypeCollection& dependentComponents);*/
	ComponentInfo(const DeppendencyType dependency, const TypeCollection& componentDependencies, 
		const ValidationAction& validationAction);
	ComponentInfo(const DeppendencyType dependency, const TypeCollection& componentDependencies = {});
	ComponentInfo(const ValidationAction& validationAction);
};
using ComponentInfoCollection = std::unordered_map<const std::type_info*, ComponentInfo>;

namespace GlobalComponentInfo
{
	ComponentInfoCollection::const_iterator GetComponentInfo(const ComponentData* component);
	ComponentInfoCollection::const_iterator GetComponentInfo(const std::type_info& typeInfo);

	void AddComponentInfo(const std::type_info& componentType, const ComponentInfo& info);

	/// <summary>
	/// Return true if the entity has the required component types for the target component
	/// NOTE: this means the target component is not yet added and is the type component that is checked based on
	/// the already existing components on the entity
	/// <param name="component"></param>
	/// <returns></returns>
	bool PassesComponentRequirementCheck(EntityData& entity, const type_info& targetComponent);

	/// <summary>
	/// Return true if the component has the dependency type specified
	/// </summary>
	/// <param name="component"></param>
	/// <param name="type"></param>
	/// <returns></returns>
	bool DoesComponentHaveDependencyType(const ComponentData* component, const DeppendencyType type);
	bool DoesComponentHaveDependencies(const ComponentData* component);
	bool DoesComponentDependOnEntity(const ComponentData* component);
	bool DoesComponentDependOnComponent(const ComponentData* component);
	
	/// <summary>
	/// Return true if the component DOES NOT HAVE a component dependency OR if it does and all those dependencies are fulfilled
	/// </summary>
	/// <param name="component"></param>
	/// <returns></returns>
	bool DoesComponentHaveComponentDependencies(const ComponentData* component);
}

