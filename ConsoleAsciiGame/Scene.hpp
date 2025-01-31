#pragma once
#include <unordered_map>
#include <queue>
#include <vector>
#include <optional>
#include <functional>
#include <string>
#include <filesystem>
#include <fstream>
#include <entt/entt.hpp>
#include "Point2DInt.hpp"
#include "TextBuffer.hpp"
#include "RenderLayer.hpp"
#include "Updateable.hpp"
#include "Entity.hpp"
//#include "EntityMapper.hpp"
#include "CameraData.hpp"
#include "TransformData.hpp"
#include "GlobalEntityManager.hpp"
#include "HelperFunctions.hpp"
#include "PhysicsWorld.hpp"

using EntityCollection = std::unordered_map<ECS::EntityID, ECS::Entity*>;
constexpr std::uint8_t MAX_ENTITIES = 100;
class Scene 
{
private:
	std::string m_sceneName;
	std::unordered_map<RenderLayerType, RenderLayer> m_Layers;

	entt::registry m_entityMapper;
	//This is where all the LOCAL entities are stored 
	// (ones that exist solely within this scene)
	std::vector<ECS::Entity> m_localEntities;
	//This is for fast entity lookup by id for local entities
	EntityCollection m_localEntityLookup;

	//TODO: check if just removing this abstract and having the scene receive
	//the global entities directly might increase performance
	GlobalEntityManager& m_globalEntities;
	
	ECS::Entity* m_mainCamera;
	//These are entities that are not located here, but are managed
	//by the scene manager and persist across scenes. Changes
	//to those entities can only be made throguh the scene manager
	//NOTE: this is not const because get entities should return a non-const pointer
	//no matter if the entity is owned by the scene or not when retreiving it from scene
	//EntityCollection& m_globalEntityLookup;

	int m_currentFrameDirtyComponents;

	//TODO: right now the physics world does not really help us since it just stores physics bodies
	//which we can retrieve from scene. This should be used to provide an effective and efficient traversal
	//of data for physics simulations to really be useful
	Physics::PhysicsWorld m_PhysicsWorld;

public:
	const std::string& m_SceneName;
	//const std::vector<RenderLayer>& m_Layers;

	const std::filesystem::path m_ScenePath;
	//The size in WIDTH, HEIGHT
	const Utils::Point2DInt m_BoundingSize;
	const GlobalEntityManager& m_GlobalEntities;

	static const std::string m_SCENE_FILE_PREFIX;

private:
	void ParseSceneFile(std::ifstream& stream, std::vector<std::vector<TextCharPosition>>& charPos) const;

	/*/// <summary>
	/// Will return the iterator to the entity, whether global or local to the scene
	/// that has the specified id
	/// </summary>
	/// <param name="id"></param>
	/// <returns></returns>
	EntityCollection::iterator GetEntityIterator(const EntityID& id);*/
	EntityCollection::iterator GetLocalEntityIterator(const ECS::EntityID& id);

	/*bool IsGlobalEntity(const EntityID& id) const;*/

public:
	Scene(const std::filesystem::path& scenePath, GlobalEntityManager& globalEntities);

	/// <summary>
	/// Will initialize the scene with deserialized entities and will initialize physics simulation
	/// </summary>
	void InitScene();

	std::vector<RenderLayer*> GetLayersMutable();
	std::vector<const RenderLayer*> GetLayers(const RenderLayerType& renderLayers) const;
	std::vector<const RenderLayer*> GetAllLayers() const;

	void AddToLayer(const RenderLayerType& type, TextBufferPosition& texture);
	std::vector<TextBufferMixed*> GetLayerBufferMutable(const RenderLayerType& renderLayers);
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

	bool HasMainCamera() const;
	void SetMainCamera(ECS::Entity& cameraEntity);
	ECS::Entity* TryGetMainCameraEntity();
	CameraData* TryGetMainCameraData();

	const Physics::PhysicsWorld& GetPhysicsWorld() const;
	Physics::PhysicsWorld& GetPhysicsWorldMutable();

	/// <summary>
	/// Will return the total number of entities in the scene, including local and 
	/// scene entities and global entities
	/// </summary>
	/// <returns></returns>
	int GetEntityCount() const;
	bool HasEntities() const;
	
	ECS::Entity& CreateEntity(const std::string& name, TransformData& transform);
	ECS::Entity& CreateEntity(const std::string& name, TransformData&& transform);
	bool HasEntity(const ECS::EntityID& id);
	ECS::Entity* TryGetEntity(const ECS::EntityID& id);

	template<typename T>
	void OperateOnComponents(const std::function<void(T&, ECS::Entity&)> action)
	{
		auto view = m_entityMapper.view<T>();
		for (auto entityId : view)
		{
			ECS::Entity* entityPtr = TryGetEntity(entityId);
			if (!Utils::Assert(this, entityPtr != nullptr, std::format("Tried to operate on component type: {} "
				"but failed to retrieve entity with ID: (it probably does not exist in the scene)",
				typeid(T).name()))) return;

			action(view.get<T>(entityId), *entityPtr);
		}
		m_globalEntities.OperateOnComponents<T>(action);
	}
};
