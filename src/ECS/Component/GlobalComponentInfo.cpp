#include "pch.hpp"
#include "ECS/Component/GlobalComponentInfo.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "ECS/Component/Component.hpp"
#include "ECS/Component/ComponentMacros.hpp"
#include "ECS/Component/AllComponentsHeader.hpp"

namespace Engine::ECS
{
	ComponentInfo::ComponentInfo(const DependencyType dependency, const TypeCollection& componentDependencies, 
		const ValidationAction& validationAction, const ComponentAddAction& postAddAction) :
		m_DependencyType(dependency), m_DependentComponents(componentDependencies), 
		m_ComponentRequirementCheck(validationAction), m_ComponentPostAddAction(postAddAction) {}

	ComponentInfo::ComponentInfo(const DependencyType dependency, const TypeCollection& componentDependencies, 
		const ComponentAddAction& postAddAction)
		: ComponentInfo(dependency, componentDependencies, nullptr, postAddAction) {}
	ComponentInfo::ComponentInfo(const ComponentAddAction& postAddAction)
		: ComponentInfo(DependencyType::None, {}, nullptr, postAddAction) {}

	ComponentInfo::ComponentInfo(const TypeCollection& dependentTypes, const ValidationAction& requiredAction, 
		const ComponentAddAction& postAddAction)
		: ComponentInfo(DependencyType::Component, dependentTypes, requiredAction, postAddAction) {}


	namespace GlobalComponentInfo
	{
		//NOTE: transform dependencies should not be added because we are guaranteed to have transform before any other component
		static ComponentInfoCollection m_ComponentInfo = { };

		void AddComponentInfo(const std::type_info& componentType, const ComponentInfo& info)
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
			if (infoIt == m_ComponentInfo.cend() || !infoIt->second.m_ComponentPostAddAction)
				return false;

			infoIt->second.m_ComponentPostAddAction(entity);
			return true;
		}

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
			if (infoIt == m_ComponentInfo.cend() || infoIt->second.m_DependencyType != DependencyType::Component) return true;

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

	namespace Utils
	{
#define ADD_OR_GET_COMPONENT_ACTION_ENTRY(Namespace, ComponentName) \
{ #ComponentName, [](EntityData& entity) -> Component* {\
	return static_cast<Component*>(&entity.GetOrAddComponentMutable(Namespace::ComponentName())); \
}}, \

		using AddOrGetComponentAction = std::function<ECS::Component*(ECS::EntityData&)>;
		static std::unordered_map<std::string, AddOrGetComponentAction> AddOrGetComponentActions =
		{
			ALL_COMPONENT_MACRO(ADD_OR_GET_COMPONENT_ACTION_ENTRY)
		};

		ECS::Component* AddOrGetComponentToEntityByName(EntityData& entity, const std::string& componentName)
		{
			auto componentActionIt = AddOrGetComponentActions.find(componentName);
			if (componentActionIt == AddOrGetComponentActions.end())
				return nullptr;
			return componentActionIt->second(entity);
		}
	}
}
