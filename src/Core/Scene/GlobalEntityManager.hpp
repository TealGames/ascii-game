#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include "ECS/Systems/MultiBodySystem.hpp"
#include <optional>
#include <functional>
#include <format>
#include "ECS/Entity/EntityRegistry.hpp"
#include "Core/Scene/Scene.hpp"
#include "Utils/HelperFunctions.hpp"

namespace Engine::ECS { class EntityData; }
namespace Engine::Scenes
{
	//using EntityIDCollection = std::unordered_map<ECS::EntityID, EntityData*>;
	using EntityNameCollection = std::unordered_map<std::string, ECS::EntityData*>;
	class GlobalEntityManager
	{
	private:
		//AssetManager& m_assetManager;

		//EntityMapper m_globalEntityMapper;
		ECS::EntityRegistry m_globalRegistry;
		std::vector<ECS::EntityData*> m_globalEntities;
		//EntityIDCollection m_globalEntityIds;
		EntityNameCollection m_globalEntityNames;

	public:
		static const char* GLOBAL_SCENE_NAME;

	private:
		/// <summary>
		/// Will remove spaces, convert to lowercase
		/// </summary>
		/// <param name="name"></param>
		/// <returns></returns>
		std::string CleanName(const std::string name) const;
		bool HasGlobalEntity(const std::string& name, const bool& cleanName) const;

	public:
		GlobalEntityManager();

		int GetCount() const;
		std::string ToStringEntityData() const;

		//EntityIDCollection::iterator GetGlobalEntityIteratorMutable(const ECS::EntityID& id);
		EntityNameCollection::iterator GetGlobalEntityIteratorMutable(const std::string& name);
		//bool IsValidIterator(const EntityIDCollection::iterator& iterator);
		bool IsValidIterator(const EntityNameCollection::iterator& iterator);

		//EntityIDCollection::const_iterator GetGlobalEntityIterator(const ECS::EntityID& id) const;
		EntityNameCollection::const_iterator GetGlobalEntityIterator(const std::string& name) const;
		//bool IsValidIterator(const EntityIDCollection::const_iterator& iterator) const;
		bool IsValidIterator(const EntityNameCollection::const_iterator& iterator) const;

		bool HasGlobalEntity(const ECS::EntityID& id) const;
		bool HasGlobalEntity(const std::string& name) const;

		ECS::EntityData& CreateGlobalEntity(const std::string& name, const TransformComponent& transform);
		//EntityData& CreateGlobalEntity(const std::string& name, TransformData&& transform);

		ECS::EntityData* TryGetGlobalEntityMutable(const ECS::EntityID& id);
		ECS::EntityData* TryGetGlobalEntityMutable(const std::string& name);
		const ECS::EntityData* TryGetGlobalEntity(const ECS::EntityID& id) const;
		const ECS::EntityData* TryGetGlobalEntity(const std::string& name) const;

		const std::vector<ECS::EntityData*>& GetAllGlobalEntities() const;
		std::vector<ECS::EntityData*>& GetAllGlobalEntitiesMutable();

		template<typename T, typename TInvocable>
		requires (std::is_base_of_v<ECS::Component, T>&& ECS::IsComponentInvocableType<T, TInvocable>)
		void OperateOnComponents(const ECS::ComponentStateFlag flags, TInvocable&& action)
		{
			ECS::OperateOnComponents<T, TInvocable>(m_globalRegistry, flags, std::forward<TInvocable>(action));
		}

		template<typename T>
		requires std::is_base_of_v<ECS::Component, T>
		void GetComponents(const ECS::ComponentStateFlag flags, std::vector<T*>& inputVec)
		{
			ECS::GetRegistryComponentsMutable<T>(m_globalRegistry, flags, inputVec);
		}
	};
}

