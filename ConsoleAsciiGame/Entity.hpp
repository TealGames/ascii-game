#pragma once
#include <map>
#include <string>
#include <optional>
#include <unordered_map>
#include <vector>
#include <memory>
#include <type_traits>
#include <algorithm>
#include <tuple>
#include "HelperFunctions.hpp"
#include "ComponentType.hpp"
#include "Debug.hpp"
#include "IValidateable.hpp"

class TransformData;
class EntityData;

namespace ECS
{
	class EntityRegistry;

	template<typename T>
	requires std::is_base_of_v<Component, T>
	struct EntityComponentPair
	{
		//Note: the types are pointers so this object
		//can easily be copied into things like optional, vector, etc

		EntityData* m_Entity;
		T* m_Data;

		EntityComponentPair(EntityData& entity, T& data) :
			m_Entity(&entity), m_Data(&data) {}
	};

	template<typename ...Args>
	requires Utils::HasAtLeastOneArg<Args...> 
	&& Utils::AllSameBaseType<Component, Args...>
		struct EntityComponents
	{
		EntityData* m_Entity;

		using TupleType = std::tuple<typename Utils::ToPointerType<Args>::Type...>;
		TupleType m_Data;

		EntityComponents(EntityData& entity, Args&... data)
			: m_Entity(&entity), m_Data(std::make_tuple(&data...))
		{
		}

		template<std::size_t index>
		auto GetAt() -> typename std::remove_pointer_t<std::tuple_element_t<index, TupleType>>&
		{
			return *(std::get<index>(m_Data));
		}
	};
}

