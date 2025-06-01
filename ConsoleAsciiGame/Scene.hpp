#pragma once
#include <unordered_map>
#include <queue>
#include <vector>
#include <optional>
#include <functional>
#include <string>
#include <filesystem>
#include <fstream>
#include "EntityRegistry.hpp"
#include "Point2DInt.hpp"
#include "TextBuffer.hpp"
#include "RenderLayer.hpp"
#include "Updateable.hpp"
#include "Entity.hpp"
#include "IJsonSerializable.hpp"
#include "ILoadable.hpp"
#include "GlobalEntityManager.hpp"
//#include "EntityMapper.hpp"
//#include "CameraData.hpp"
#include "TransformData.hpp"

//using EntityCollection = std::unordered_map<ECS::EntityID, ECS::Entity*>;
//TODO: perhaps we should consolidate the string and local entity collection into one to not take up as much memory
//using EntityNameCollection = std::unordered_map<std::string, EntityData*>;
//using EntityIDCollection = std::unordered_map<ECS::EntityID, EntityData*>;
//constexpr std::uint8_t MAX_ENTITIES = 100;

class EntityData;

class Scene : public IValidateable
{
private:
	std::string m_sceneName;
	std::unordered_map<RenderLayerType, RenderLayer> m_layers;

	ECS::EntityRegistry m_registry;
	//This is where all the LOCAL entities are stored 
	// (ones that exist solely within this scene) and ONLY the root entities (no descendants/children)
	std::vector<EntityData*> m_localRootEntities;
	//This is for fast entity lookup by id for local entities
	//EntityNameCollection m_localEntityNameLookup;
	//EntityIDCollection m_localEntityIdLookup;

	//TODO: check if just removing this abstract and having the scene receive
	//the global entities directly might increase performance
	GlobalEntityManager* m_globalEntities;
	
	//ECS::Entity* m_mainCamera;
	// 
	//These are entities that are not located here, but are managed
	//by the scene manager and persist across scenes. Changes
	//to those entities can only be made throguh the scene manager
	//NOTE: this is not const because get entities should return a non-const pointer
	//no matter if the entity is owned by the scene or not when retreiving it from scene
	//EntityCollection& m_globalEntityLookup;

	int m_currentFrameDirtyComponents;

	//Physics::PhysicsWorld m_physicsWorld;

	//const std::filesystem::path m_scenePath;

public:
	//const std::vector<RenderLayer>& m_Layers;
	//The size in WIDTH, HEIGHT

	static const std::string SCENE_FILE_PREFIX;

private:
	//void ParseSceneFile(std::ifstream& stream, std::vector<std::vector<TextCharPosition>>& charPos);

	/*/// <summary>
	/// Will return the iterator to the entity, whether global or local to the scene
	/// that has the specified id
	/// </summary>
	/// <param name="id"></param>
	/// <returns></returns>
	EntityCollection::iterator GetEntityIterator(const EntityID& id);*/
	//EntityIDCollection::iterator GetLocalEntityIterator(const ECS::EntityID& id);
	//EntityNameCollection::iterator GetLocalEntityIterator(const std::string& name);

	/*bool IsGlobalEntity(const EntityID& id) const;*/

	Scene(const std::string& sceneName, GlobalEntityManager* manager);

public:
	Scene(const std::string& sceneName, GlobalEntityManager& globalEntities);

	static std::string ExtractSceneName(const std::filesystem::path& path);

	/// <summary>
	/// Will initialize the scene with deserialized entities and will initialize physics simulation
	/// </summary>
	//void InitScene();

	std::string GetName() const;
	GlobalEntityManager& TryGetGlobalEntityManagerMutable();
	const GlobalEntityManager& TryGetGlobalEntityManager() const;

	std::vector<RenderLayer*> GetLayersMutable();
	std::vector<RenderLayer*> GetLayersMutable(const RenderLayerType& renderLayers);
	std::vector<const RenderLayer*> GetLayers(const RenderLayerType& renderLayers) const;
	std::vector<const RenderLayer*> GetAllLayers() const;

	void AddToLayer(const RenderLayerType& type, TextBufferCharPosition& texture);
	std::vector<FragmentedTextBuffer*> GetLayerBufferMutable(const RenderLayerType& renderLayers);
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

	/*bool HasMainCamera() const;
	void SetMainCamera(ECS::Entity& cameraEntity);
	ECS::Entity* TryGetMainCameraEntityMutable();
	CameraData* TryGetMainCameraMutable();
	const CameraData* TryGetMainCamera() const;*/

	//const Physics::PhysicsWorld& GetPhysicsWorld() const;
	//Physics::PhysicsWorld& GetPhysicsWorldMutable();

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
	std::vector<const EntityData*> GetAllEntities() const;
	std::vector<EntityData*> GetAllEntitiesMutable();
	std::vector<const EntityData*> GetLocalEntities() const;
	std::vector<EntityData*> GetLocalEntitiesMutable();
	
	EntityData& CreateEntity(const std::string& name, const TransformData& transform);
	bool HasEntity(const ECS::EntityID& id);
	/// <summary>
	/// Will try to find an entity within the scene (global or local)
	/// </summary>
	/// <param name="id"></param>
	/// <returns></returns>
	EntityData* TryGetEntityMutable(const ECS::EntityID& id);

	/// <summary>
	/// A overloaded version of the entity id function. 
	/// Note: THIS IS SLOW SINCE IT LOOPS THROUGH ALL ENTITIES AND SHOULD RARELY BE USED
	/// </summary>
	/// <param name="name"></param>
	/// <returns></returns>
	EntityData* TryGetEntityMutable(const std::string& name, const bool& ignoreCase= false);

	const EntityData* TryGetEntity(const std::string& name, const bool& ignoreCase = false) const;

	template<typename T>
	requires std::is_base_of_v<ComponentData, T>
	void OperateOnComponents(const std::function<void(T&)> action)
	{
		ECS::OperateOnActiveComponents<T>(m_registry, action);
		TryGetGlobalEntityManagerMutable().OperateOnComponents<T>(action);
	}

	template<typename T>
	requires std::is_base_of_v<ComponentData, T>
	void GetComponentsMutable(std::vector<T*>& inputVector)
	{
		ECS::GetRegistryComponentsMutable<T>(m_registry, inputVector);
		TryGetGlobalEntityManagerMutable().GetComponents<T>(inputVector);
	}

	bool Validate() override;

	std::string ToString() const;
};
