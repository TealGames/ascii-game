#include "pch.hpp"
#include "ECS/Component/GlobalComponentInfo.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "ECS/Component/Component.hpp"

ComponentInfo::ComponentInfo(const DependencyType dependency, const TypeCollection& componentDependencies, const ValidationAction& validationAction, const ComponentAddAction& postAddAction) :
	m_DependencyType(dependency), m_DependentComponents(componentDependencies), m_ComponentRequirementCheck(validationAction), m_ComponentPostAddAction(postAddAction) {}

ComponentInfo::ComponentInfo(const DependencyType dependency, const TypeCollection& componentDependencies, const ComponentAddAction& postAddAction)
	: ComponentInfo(dependency, componentDependencies, nullptr, postAddAction) {}
ComponentInfo::ComponentInfo(const ComponentAddAction& postAddAction)
	: ComponentInfo(DependencyType::None, {}, nullptr, postAddAction) {}

ComponentInfo::ComponentInfo(const TypeCollection& dependentTypes, const ValidationAction& requiredAction, const ComponentAddAction& postAddAction)
	: ComponentInfo(DependencyType::Component, dependentTypes, requiredAction, postAddAction) {}


namespace GlobalComponentInfo
{
	//NOTE: transform dependencies should not be added because we are guaranteed to have transform before any other component
	ComponentInfoCollection m_ComponentInfo = { };

	/*ComponentInfo::ComponentInfo(const RequiredComponentType requiredType, const TypeCollection& required) 
		: ComponentInfo(requiredType, required, DeppendencyType::None, {}) {}
	ComponentInfo::ComponentInfo(const DeppendencyType dependencyType, const TypeCollection& dependentComponents) 
		: ComponentInfo(RequiredComponentType::None, {}, dependencyType, dependentComponents) {}
	ComponentInfo::ComponentInfo(const RequiredComponentType requiredType, const TypeCollection& required,
		const DeppendencyType dependencyType, const TypeCollection& dependentComponents) 
		: m_RequiredType(requiredType), m_RequiredComponents(required), m_DependencyType(dependencyType), m_DependentComponents(dependentComponents) 
	{
	}*/

	void AddComponentInfo(const std::type_info& componentType,const ComponentInfo& info)
	{
		m_ComponentInfo.emplace(&componentType, info);
	}

	ComponentInfoCollection::const_iterator GetComponentInfo(const Component* component)
	{
		return m_ComponentInfo.find(&typeid(*component));
	}
	ComponentInfoCollection::const_iterator GetComponentInfo(const std::type_info& typeInfo)
	{
		return m_ComponentInfo.find(&typeInfo);
	}

	bool PassesComponentRequirementCheck(EntityData& entity, const type_info& targetComponent)
	{
		auto infoIt = GetComponentInfo(targetComponent);
		if (infoIt == m_ComponentInfo.cend()) return true;
		if (!infoIt->second.m_ComponentRequirementCheck) return true;

		return infoIt->second.m_ComponentRequirementCheck(entity);
	}

	bool InvokePostAddAction(EntityData& entity, const type_info& targetComponent)
	{
		auto infoIt = GetComponentInfo(targetComponent);
		if (infoIt == m_ComponentInfo.cend() || !infoIt->second.m_ComponentPostAddAction) return false;
	
		infoIt->second.m_ComponentPostAddAction(entity);
	}

	/*bool DoesEntityHaveRequiredComponentsForComponent(const EntityData& entity, const type_info& targetComponent, 
		std::vector<const std::type_info*>* outMissingComponents)
	{
		auto infoIt = GetComponentInfo(targetComponent);
		if (infoIt == m_ComponentInfo.cend() || infoIt->second.m_RequiredType== RequiredComponentType::None) return true;

		for (const auto& requiredComponent : infoIt->second.m_RequiredComponents)
		{
			if (requiredComponent == nullptr) continue;

			if (infoIt->second.m_RequiredType == RequiredComponentType::Self && !entity.HasComponent(*requiredComponent))
			{
				if (outMissingComponents == nullptr) return false;
				else outMissingComponents->push_back(requiredComponent);
			}
			else if (infoIt->second.m_RequiredType == RequiredComponentType::Parent &&
				(!entity.HasParent() || !entity.GetParent()->HasComponent(*requiredComponent)))
			{
				if (outMissingComponents == nullptr) return false;
				else outMissingComponents->push_back(requiredComponent);
			}
			else if (infoIt->second.m_RequiredType == RequiredComponentType::NoParentOrParent &&
				entity.HasParent() && !entity.GetParent()->HasComponent(*requiredComponent))
			{
				if (outMissingComponents == nullptr) return false;
				else outMissingComponents->push_back(requiredComponent);
			}
			else if (infoIt->second.m_RequiredType == RequiredComponentType::HighestParent)
			{
				const EntityData* highestParent = entity.GetHighestParent();
				if (highestParent == nullptr || highestParent->HasComponent(*requiredComponent))
					continue;

				if (outMissingComponents == nullptr) return false;
				else outMissingComponents->push_back(requiredComponent);
			}
		}
		return true;
	}*/
	bool DoesComponentHaveDependencyType(const Component* component, const DependencyType type)
	{
		auto infoIt = m_ComponentInfo.find(&typeid(*component));
		//If we can not find a profile and the depdency type is None, we can return true 
		//otherwise we say it is false because by default we assume no profile == no depdenencies
		if (infoIt == m_ComponentInfo.cend())
		{
			if (type == DependencyType::None) return true;
			else return false;
		}
		return infoIt->second.m_DependencyType == type;
	}
	bool DoesComponentHaveDependencies(const Component* component)
	{
		if (component == nullptr) return false;
		auto infoIt = GetComponentInfo(component);
		return infoIt != m_ComponentInfo.cend() && infoIt->second.m_DependencyType != DependencyType::None;
	}
	bool DoesComponentDependOnEntity(const Component* component)
	{
		if (component == nullptr) return false;
		auto infoIt = GetComponentInfo(component);
		return infoIt != m_ComponentInfo.cend() && infoIt->second.m_DependencyType == DependencyType::Entity;
	}
	bool DoesComponentDependOnComponent(const Component* component)
	{
		if (component == nullptr) return false;
		auto infoIt = GetComponentInfo(component);
		return infoIt != m_ComponentInfo.cend() && infoIt->second.m_DependencyType == DependencyType::Component;
	}

	bool DoesComponentHaveComponentDependencies(const Component* component)
	{
		if (component == nullptr) return true;

		auto infoIt = GetComponentInfo(component);
		if (infoIt== m_ComponentInfo.cend() || infoIt->second.m_DependencyType != DependencyType::Component) return true;

		const EntityData& entity = component->GetEntity();
		for (const auto& componentDependency : infoIt->second.m_DependentComponents)
		{
			if (componentDependency == nullptr) continue;
			if (!entity.HasComponent(*componentDependency))
				return false;
		}
		return true;
	}
}