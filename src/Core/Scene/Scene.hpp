#pragma once
#include <unordered_map>
#include <queue>
#include <vector>
#include <optional>
#include <functional>
#include <string>
#include <filesystem>
#include <fstream>
#include <tuple>
#include "ECS/Entity/EntityRegistry.hpp"
#include "Core/Visual/TextBuffer.hpp"
#include "Core/Rendering/RenderLayer.hpp"
#include "ECS/Entity/Entity.hpp"
#include "Core/Serialization/IJsonSerializable.hpp"
#include "Core/Scene/GlobalEntityManager.hpp"
#include "ECS/Component/Types/World/TransformComponent.hpp"

//using EntityCollection = std::unordered_map<ECS::EntityID, ECS::Entity*>;
//TODO: perhaps we should consolidate the string and local entity collection into one to not take up as much memory
//using EntityNameCollection = std::unordered_map<std::string, ECS::EntityData*>;
//using EntityIDCollection = std::unordered_map<ECS::EntityID, ECS::EntityData*>;
//constexpr std::uint8_t MAX_ENTITIES = 100;

namespace Engine::ECS { class ECS::EntityData; }
namespace Engine::Scenes
{
	class Scene : public IValidateable
	{
	private:
		std::string m_sceneName;
		std::unordered_map<Rendering::RenderLayerType, Rendering::RenderLayer> m_layers;

		ECS::EntityRegistry m_registry;
		//This is where all the LOCAL entities are stored 
		// (ones that exist solely within this scene) and ONLY the root entities (no descendants/children)
		std::vector<ECS::EntityData*> m_localRootEntities;

		//TODO: check if just removing this abstract and having the scene receive
		//the global entities directly might increase performance
		GlobalEntityManager* m_globalEntities;

		int m_currentFrameDirtyComponents;

	public:
		static const std::string SCENE_FILE_PREFIX;

	private:
		Scene(const std::string& sceneName, GlobalEntityManager* manager);

	public:
		Scene(const std::string& sceneName, GlobalEntityManager& globalEntities);

		static std::string ExtractSceneName(const std::filesystem::path& path);

		void Start();
		bool Validate() override;

		std::string GetName() const;
		GlobalEntityManager& TryGetGlobalEntityManagerMutable();
		const GlobalEntityManager& TryGetGlobalEntityManager() const;

		std::vector<Rendering::RenderLayer*> GetLayersMutable();
		std::vector<Rendering::RenderLayer*> GetLayersMutable(const Rendering::RenderLayerType& renderLayers);
		std::vector<const Rendering::RenderLayer*> GetLayers(const Rendering::RenderLayerType& renderLayers) const;
		std::vector<const Rendering::RenderLayer*> GetAllLayers() const;

		void AddToLayer(const Rendering::RenderLayerType& type, Rendering::TextBufferCharPosition2D& texture);
		std::vector<Rendering::FragmentedTextBuffer2D*> GetLayerBufferMutable(const Rendering::RenderLayerType& renderLayers);
		/// <summary>
		/// Gets all render layer buffers in ascending sorted order
		/// </summary>
		/// <returns></returns>
		std::vector<std::tuple<Rendering::RenderLayerType, Rendering::FragmentedTextBuffer2D*>> GetAllLayerBufferMutable();
		//std::vector<TextBuffer*> GetTextBuffersMutable(const RenderLayerType& renderLayers);
		//void SetLayers(const RenderLayerType& renderLayers, const std::vector<TextCharPosition>& positions);
		//void SetLayers(const RenderLayerType& renderLayers, const std::vector<ColorPosition>& positions);
		void ResetAllLayers();

		std::string ToStringLayers() const;
		std::string ToStringEntityData() const;

		void ResetFrameDirtyComponentCount();
		void IncreaseFrameDirtyComponentCount();
		int GetDirtyComponentCount() const;
		bool HasDirtyComponents() const;

		/// <summary>
		/// Will return the total number of entities in the scene, including local and 
		/// scene entities and global entities
		/// </summary>
		/// <returns></returns>
		int GetEntityCount() const;
		bool HasEntities() const;
		/// <summary>
		/// Will return all entities in the scene as immutable (including global and local entities)
		/// </summary>
		/// <returns></returns>
		std::vector<const ECS::EntityData*> GetAllEntities() const;
		std::vector<ECS::EntityData*> GetAllEntitiesMutable();
		std::vector<const ECS::EntityData*> GetLocalEntities() const;
		std::vector<ECS::EntityData*> GetLocalEntitiesMutable();

		ECS::EntityData& CreateEntity(const std::string& name, const TransformComponent& transform);
		bool HasEntity(const ECS::EntityID& id);
		/// <summary>
		/// Will try to find an entity within the scene (global or local)
		/// </summary>
		/// <param name="id"></param>
		/// <returns></returns>
		ECS::EntityData* TryGetEntityMutable(const ECS::EntityID& id);

		/// <summary>
		/// A overloaded version of the entity id function. 
		/// Note: THIS IS SLOW SINCE IT LOOPS THROUGH ALL ENTITIES AND SHOULD RARELY BE USED
		/// </summary>
		/// <param name="name"></param>
		/// <returns></returns>
		ECS::EntityData* TryGetEntityMutable(const std::string& name, const bool& ignoreCase = false);

		const ECS::EntityData* TryGetEntity(const std::string& name, const bool& ignoreCase = false) const;

		template<typename T, typename TInvocable>
		requires (std::is_base_of_v<ECS::Component, T>&& ECS::IsComponentInvocableType<T, TInvocable>)
		void OperateOnComponents(const ECS::ComponentStateFlag flags, TInvocable&& action)
		{
			ECS::OperateOnComponents<T, TInvocable>(m_registry, flags, std::forward<TInvocable>(action));
			TryGetGlobalEntityManagerMutable().OperateOnComponents<T, TInvocable>(flags, std::forward<TInvocable>(action));
		}

		template<typename T, typename TInvocable>
		requires (std::is_base_of_v<ECS::Component, T>&& ECS::IsComponentInvocableType<T, TInvocable>)
		void OperateOnActiveComponents(TInvocable&& action)
		{
			return OperateOnComponents<T, TInvocable>(ECS::ALL_ACTIVE_ENABLED_FLAG, std::forward<TInvocable>(action));
		}

		template<typename T>
		requires std::is_base_of_v<ECS::Component, T>
		void GetComponentsMutable(const ECS::ComponentStateFlag flags, std::vector<T*>& inputVector)
		{
			ECS::GetRegistryComponentsMutable<T>(m_registry, flags, inputVector);
			TryGetGlobalEntityManagerMutable().GetComponents<T>(flags, inputVector);
		}

		std::string ToString() const;
	};

}
