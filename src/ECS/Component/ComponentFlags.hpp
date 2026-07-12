#pragma once
#include "Utils/HelperMacros.hpp"
#include "ECS/Component/ComponentMacros.hpp"

namespace Engine::ECS
{
#define COMPONENT_FLAG_TYPE std::uint64_t
	using ComponentFlagType = COMPONENT_FLAG_TYPE;

#define COMPONENT_NAME(Namespace, ComponentName) ComponentName,
	enum class ComponentFlagIndex : ComponentFlagType
	{
		ALL_COMPONENT_MACRO(COMPONENT_NAME)
	};
	
	constexpr ComponentFlagType operator<<(const ComponentFlagType lhs, const ComponentFlagIndex rhs)
	{
		return lhs << static_cast<ComponentFlagType>(rhs);
	}

#define FLAG_BIT_COMPONENT_NAME(Namespace, ComponentName) ComponentName = COMPONENT_FLAG_TYPE{1} << ComponentFlagIndex::ComponentName,
	
	enum class ComponentFlag : ComponentFlagType
	{
		None = 0,
		ALL_COMPONENT_MACRO(FLAG_BIT_COMPONENT_NAME)
		All = UINT64_MAX
	};
	FLAG_ENUM_OPERATORS(ComponentFlag)
}





