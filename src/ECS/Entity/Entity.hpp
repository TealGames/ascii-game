#pragma once
#include <type_traits>
#include <tuple>
#include "Utils/HelperFunctions.hpp"
#include "ECS/Component/Component.hpp"

namespace Engine { class TransformComponent; }

namespace Engine::ECS
{
	class EntityRegistry;
	class EntityData;

	template<typename T>
	requires std::is_base_of_v<Component, T>
	struct EntityDataPair
	{
		//Note: the types are pointers so this object
		//can easily be copied into things like optional, vector, etc

		EntityData* m_Entity;
		T* m_Data;

		EntityDataPair(EntityData& entity, T& data) :
			m_Entity(&entity), m_Data(&data) {}
	};

	template<typename ...Args>
	requires ::Utils::HasAtLeastOneArg<Args...> 
	&& ::Utils::AllSameBaseType<Component, Args...> 
	struct EntityDatas
	{
		EntityData* m_Entity;

		using TupleType = std::tuple<typename ::Utils::ToPointerType<Args>::Type...>;
		TupleType m_Data;

		EntityDatas(EntityData& entity, Args&... data)
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

