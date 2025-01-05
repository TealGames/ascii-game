#pragma once
#include <map>
#include <string>
#include <optional>
#include <unordered_map>
#include <vector>
#include <memory>
#include <type_traits>
#include <algorithm>
#include "EntityMapper.hpp"
#include "HelperFunctions.hpp"
#include "TransformData.hpp"
#include "ComponentType.hpp"

namespace ECS
{
	using ComponentCollectionType = std::unordered_map<ComponentType, ComponentID>;
	class Entity
	{
	private:
		EntityMapper& m_entityMapper;
		ComponentCollectionType m_componentIDs;

		std::string m_name;

	public:
		const std::string& m_Name;
		//TODO: maybe replace id with GUID (UIUD is available in boost library)
		const EntityID m_Id;
		TransformData& m_Transform;

	private:
		TransformData& GetTransformRef(const TransformData& data);
		TransformData& GetTransformRef(TransformData&& data);

		ComponentCollectionType::iterator GetComponentIDIteratorMutable(const ComponentType& type);
		ComponentCollectionType::const_iterator GetComponentIDIterator(const ComponentType& type) const;

	public:
		/// <summary>
		/// 
		/// </summary>
		/// <param name="name"></param>
		/// <param name="objectId"></param>
		Entity(const std::string& name, EntityMapper& mapper, const TransformData& transform);
		Entity(const std::string& name, EntityMapper& mapper, TransformData&& transform);

		//We do not allow copying since the component data is stored in the entity mapper
		//and copying would delete obj and not data since deleting/removing data is not implemented
		//in the entity mapper yet
		//Entity(Entity&& entity) = delete;

		bool HasComponent(const ComponentType& type) const;

		/// <summary>
		/// Will add the specified type of component. To improve performance and prevent the need for
		/// pointers/indirection and performance lost, the component data is moved.
		/// Note: only rvalues are allowed since it helps performance since a component can not be added as a 
		/// reference to the structure that stores componnents
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <param name="component"></param>
		/// <param name="outComponent">Pointer to the component stored (since the actual componenent is moved) 
		/// since there is no point of keeping copies. Note: this should be a pointer to a null pointer (and not just null)
		/// if you want to retrieve the out value</param>
		/// <returns></returns>
		template<typename T>
		requires std::is_rvalue_reference_v<T&&>
		bool TryAddComponent(T&& component, T** outComponent)
		{
			if (!Utils::Assert(this, std::is_copy_assignable_v<T>, std::format("Tried to add component of type: {} "
				"to entity id: {} via ENTITY but that type does not have a valid move assignment operator to move the componnent",
				typeid(T).name(), m_Name))) return false;

			const ComponentType type = GetComponentFromType<T>();
			if (!Utils::Assert(this, !HasComponent(type), std::format("Tried to add the component type: {} to entity: {} "
				"but it already exists on this entity! Components: {}", 
				ToString(type), m_Name, Utils::ToStringIterable(m_componentIDs)))) return false;

			//Note: we forward it to preserve the rvalue reference (and not need for the function to deduce again)
			std::optional<ComponentID> compID = m_entityMapper.TryAddComponent<T>(m_Id, std::forward<T>(component), type, outComponent);
			if (!Utils::Assert(this, compID.has_value(), std::format("Tried to add the component type: {} to entity: {} "
				"but the entity mapper failed to add it!", ToString(type), m_Name))) return false;

			m_componentIDs.emplace(type, compID.value());
			//outComponent = &(m_entityMapper.TryGetComponent<T>(m_Id, type));
			return true;
		}

		template<typename T>
		requires std::is_rvalue_reference_v<T&&>
		T* TryAddComponent(T&& component)
		{
			T* singleOutPtr = nullptr;
			T** doubleOutPtr = &singleOutPtr;

			TryAddComponent<T>(std::move(component), doubleOutPtr);
			if (Utils::IsValidPointer(doubleOutPtr)) return *doubleOutPtr;
			return nullptr;
		}

		template<typename T>
		T* TryGetComponent()
		{
			const ComponentType type = GetComponentFromType<T>();
			auto it = GetComponentIDIteratorMutable(type);

			return m_entityMapper.TryGetComponent<T>(it->second);
		}
	};
}

