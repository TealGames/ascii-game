#pragma once
#include <map>
#include <string>
#include <vector>
#include <memory>
#include <type_traits>
#include <algorithm>
#include "EntityID.hpp"
#include "Component.hpp"
#include "HelperFunctions.hpp"
#include "Updateable.hpp"
#include "TextBuffer.hpp"
#include "Point2DInt.hpp"
#include "Transform.hpp"

namespace ECS
{
	using ComponentCollectionType = std::vector<Component*>;
	class Entity : public Updateable
	{
	private:
		//We can't do this because of slicing-> since they are value types
		//the polymorphism data is lost
		/*std::vector<Component> m_components = {};*/
		ComponentCollectionType m_components;
		Transform& m_transform;

		std::string m_name;

	public:
		const UpdatePriority m_UpdatePriority;
		const std::string& m_Name;
		const Transform& m_Transform;
		//TODO: maybe replace id with GUID (UIUD is available in boost library)
		const EntityID m_Id;

	private:
		template <typename T>
		ComponentCollectionType::iterator GetComponentIteratorMutable()
		{
			for (auto it = m_components.begin(); it != m_components.end(); it++)
			{
				if (typeid(*(*it)).name() == typeid(T).name()) return it;
			}
			return m_components.end();
		}

		template <typename T>
		ComponentCollectionType::const_iterator GetComponentIterator() const
		{
			for (auto it = m_components.begin(); it != m_components.end(); it++)
			{
				if (typeid(*(*it)).name() == typeid(T).name()) return it;
			}
			return m_components.end();
		}

		/*template <typename T>
		bool IsComponentType() const
		{
			if (typeid(T).name() == typeid(Component).name()) return false;
			return static_cast<Component>(T);
		}*/

	public:
		/// <summary>
		/// 
		/// </summary>
		/// <param name="name"></param>
		/// <param name="objectId"></param>
		Entity(const std::string& name, Transform& transform, const UpdatePriority& updatePriority=0);

		template <typename T>
		auto HasComponent()
			-> typename std::enable_if<std::is_base_of_v<Component, T>, bool>::type
		{
			return GetComponentIterator<T>() != m_components.end();
		}

		/// <summary>
		/// 
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <returns></returns>
		template <typename T>
		auto TryGetComponent()
			-> typename std::enable_if<std::is_base_of_v<Component, T>, T*>::type
		{
			if (!HasComponent<T>()) return nullptr;
			return dynamic_cast<T*>(*GetComponentIteratorMutable<T>());
		}


		/// <summary>
		/// 
		/// </summary>
		/// <param name="component"></param>
		/// <param name="id"></param>
		template <typename T>
		auto TryAddComponent(T* component)
			-> typename std::enable_if<std::is_base_of_v<Component, T>, bool>::type
		{
			if (!Utils::Assert(!HasComponent<T>(), std::format("Tried to add a duplicate component of type {} to entity: {}",
				typeid(T).name(), m_Name)))
				return false;

			m_components.push_back(component);
			std::sort(m_components.begin(), m_components.end(), 
				//more priority=> means goes sooner
				[](const Component* firstComp, const Component* secondComp) -> bool
				{
					return secondComp->GetUpdatePriority() < firstComp->GetUpdatePriority();
				});

			return true;
		}

		/// <summary>
		/// Will remove the component with the id specified
		/// </summary>
		/// <param name="id"></param>
		/// <returns></returns>
		template <typename T>
		auto TryRemoveComponent()
			-> typename std::enable_if<std::is_base_of_v<Component, T>, bool>::type
		{
			if (!Utils::Assert(HasComponent<T>(), std::format("Tried to remove a non-existing component {} to entity: {}",
				typeid(T).name(), m_Name)))
				return false;

			m_components.erase(GetComponentIterator<T>());
			return true;
		}

		void Start() override;
		void UpdateStart(float deltaTime) override;
		void UpdateEnd(float deltaTime) override;

		virtual UpdatePriority GetUpdatePriority() const;
	};

}

