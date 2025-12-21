#include "pch.hpp"
#include <filesystem>
#include <fstream>
#include "Utils/Data/Point2DInt.hpp"
#include "Core/Scene/Scene.hpp"
#include "Core/Visual/TextBuffer.hpp"
#include "Utils/HelperFunctions.hpp"
#include "StaticGlobals.hpp"
#include "Utils/Data/Event.hpp"
#include "Utils/Data/Array2DPosition.hpp"
#include "Core/Visual/VisualData.hpp"
#include "ECS/Component/Types/World/PhysicsBodyData.hpp"
#include "ECS/Component/Types/World/TransformData.hpp"
#include "Utils/StringUtil.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"

#include "nlohmann/json.hpp"
#include "Core/Serialization/JsonUtils.hpp"

using Json = nlohmann::json;

const std::string Scene::SCENE_FILE_PREFIX = "scene_";

Scene::Scene(const std::string& sceneName, GlobalEntityManager* manager) :
	m_layers{}, m_sceneName(sceneName), //m_scenePath(scenePath),
	m_localRootEntities(), //m_localEntityIdLookup(), 
	//m_localEntityNameLookup(),//m_globalEntityLookup(globalEntities),
	m_currentFrameDirtyComponents(0), m_registry(),
	//m_mainCamera(nullptr),
	m_globalEntities(manager)
{
	/*if (!Assert(std::filesystem::exists(scenePath), std::format("Tried to create a scene at path: {} "
		"but that path does not exist", scenePath.string())))
		return;

	if (!Assert(scenePath.has_filename(), std::format("Tried to create a scene at path: {} "
		"but that path does not lead to a file", scenePath.string())))
		return;*/

		//m_sceneName = ExtractSceneName(scenePath);

	//m_localEntities.reserve(MAX_ENTITIES);
	//m_localEntityNameLookup.reserve(MAX_ENTITIES);
	//m_localEntityIdLookup.reserve(MAX_ENTITIES);

	//TODO: make it so that layers are not specific to a scene meaning the names are created in a spearate manager or elsewhere
	//and then during this constructor they are passed and craeted so each scene has the same layers, but their own instances
	m_layers.emplace(RenderLayerType::Background, RenderLayer{});
	m_layers.emplace(RenderLayerType::Player, RenderLayer{});
	m_layers.emplace(RenderLayerType::UI, RenderLayer{});

	//LoadData();

	//Log(std::format("New layer w: {} h: {}", std::to_string(newLayerW), std::to_string(newLayerH)));
	//TODO: right now these are global constants, but might have to be later parsed from data
	//for each scene, allowing each scene to have its own settings

	//TODO: right now we only have one layer, but later on we should add multiple
	//Note: right now we make two layers one for background and one for player, but this should
	//get abstracted more with ids and text file parsing of scene data
}

Scene::Scene(const std::string& sceneName, GlobalEntityManager& globalEntities) 
	: Scene(sceneName, &globalEntities) {}

std::string Scene::ExtractSceneName(const std::filesystem::path& path)
{
	std::string sceneName= path.stem().string().substr(SCENE_FILE_PREFIX.size() - 1);
	//std::replace(sceneName.begin(), sceneName.end(), '_', ' ');
	return sceneName;
}

std::string Scene::GetName() const
{
	return m_sceneName;
}
GlobalEntityManager& Scene::TryGetGlobalEntityManagerMutable()
{
	ENGINE_ASSERT(m_globalEntities != nullptr, "Tried to get global entities manager MUTABLE from scene: {} but global entities manager "
		"is not set up (could be due to creating scene using constructor that does not include dependency", GetName());
	return *m_globalEntities;
}
const GlobalEntityManager& Scene::TryGetGlobalEntityManager() const
{
	ENGINE_ASSERT(m_globalEntities != nullptr, "Tired to get global entities manager from scene: {} but global entities manager "
		"is not set up (could be due to creating scene using constructor that does not include dependency", GetName());

	return *m_globalEntities;
}

std::vector<RenderLayer*> Scene::GetLayersMutable()
{
	if (m_layers.empty()) return {};

	std::vector<RenderLayer*> layers = {};
	for (auto& layer : m_layers) layers.push_back(&(layer.second));
	return layers;
}
std::vector<RenderLayer*> Scene::GetLayersMutable(const RenderLayerType& renderLayers)
{
	if (m_layers.empty()) return {};

	std::vector<RenderLayer*> layers = {};
	for (auto& layer : m_layers)
	{
		if ((layer.first & renderLayers) != 0) layers.push_back(&(layer.second));
	}
	return layers;
}

std::vector<const RenderLayer*> Scene::GetLayers(const RenderLayerType& renderLayers) const
{
	if (m_layers.empty()) return {};

	std::vector<const RenderLayer*> layers = {};
	for (const auto& layer : m_layers)
	{
		if ((layer.first & renderLayers)!=0) layers.push_back(&(layer.second));
	}
	return layers;
}

std::vector<const RenderLayer*> Scene::GetAllLayers() const
{
	if (m_layers.empty()) return {};

	std::vector<const RenderLayer*> layers = {};
	for (const auto& layer : m_layers)
	{
		layers.push_back(&(layer.second));
	}
	return layers;
}

void Scene::AddToLayer(const RenderLayerType& layers, TextBufferCharPosition2D& textBufferPos)
{
	if (layers == RenderLayerType::None) return;

	for (auto& layer : m_layers)
	{
		if ((layer.first & layers) == 0) continue;
		layer.second.AddText(textBufferPos);
		return;
	}
}

std::vector<FragmentedTextBuffer2D*> Scene::GetLayerBufferMutable(const RenderLayerType& renderLayers)
{
	std::vector<FragmentedTextBuffer2D*> buffers = {};
	if (renderLayers == RenderLayerType::None) return buffers;

	for (auto& layer : m_layers)
	{
		if ((layer.first & renderLayers) == 0) continue;
		buffers.push_back(&layer.second.GetBufferMutable());
	}
	return buffers;
}
std::vector<std::tuple<RenderLayerType, FragmentedTextBuffer2D*>> Scene::GetAllLayerBufferMutable()
{
	std::vector<std::tuple<RenderLayerType, FragmentedTextBuffer2D*>> buffers = {};
	for (auto& layer : m_layers)
	{
		buffers.emplace_back(std::make_tuple(layer.first, &layer.second.GetBufferMutable()));
	}
	return buffers;
}

void Scene::ResetAllLayers()
{
	for (auto& layer : m_layers) layer.second.ResetToDefault();
}

int Scene::GetEntityCount() const
{
	return m_localRootEntities.size() + TryGetGlobalEntityManager().GetCount();
}

bool Scene::HasEntities() const
{
	return GetEntityCount() > 0;
}

std::vector<const EntityData*> Scene::GetAllEntities() const
{
	std::vector<const EntityData*> entities = GetLocalEntities();

	for (const auto& globalEntity : TryGetGlobalEntityManager().GetAllGlobalEntities())
		entities.push_back(globalEntity);
	
	return entities;
}

std::vector<const EntityData*> Scene::GetLocalEntities() const
{
	std::vector<const EntityData*> entities = {};
	for (const auto& localEntity : m_localRootEntities)
		entities.push_back(localEntity);

	return entities;
}
std::vector<EntityData*> Scene::GetLocalEntitiesMutable()
{
	std::vector<EntityData*> entities = {};
	for (auto& localEntity : m_localRootEntities)
		entities.push_back(localEntity);

	return entities;
}

std::vector<EntityData*> Scene::GetAllEntitiesMutable()
{
	std::vector<EntityData*> entities = {};
	for (auto& entity : TryGetGlobalEntityManagerMutable().GetAllGlobalEntitiesMutable())
	{
		entities.push_back(entity);
	}

	for (auto& entity : m_localRootEntities)
	{
		entities.push_back(entity);
	}
	return entities;
}

EntityData& Scene::CreateEntity(const std::string& name, const TransformComponent& transform)
{
	EntityData& createdEntity = m_registry.CreateNewEntity(name, transform);
	m_localRootEntities.emplace_back(&createdEntity);
	createdEntity.m_SceneName= m_sceneName;

	return createdEntity;
}

bool Scene::HasEntity(const ECS::EntityID& id)
{
	return m_registry.IsValidID(id) || TryGetGlobalEntityManager().HasGlobalEntity(id);
	//bool isLocal = GetLocalEntityIterator(id) != m_localEntityLookup.end();
	//if (isLocal) return true;

	//return m_globalEntities.HasGlobalEntity(id);
}

//TODO: while this is fine, we should add version that allow us to add components to the entity
//and would probably be best if we create them on the heap probably to extend memory lifetime

//TODO: also maybe consider managing what objects are required to be in a scene, like a camera
EntityData* Scene::TryGetEntityMutable(const ECS::EntityID& id)
{
	//Note: even though we only have local root entities, all entity data for this scene INCLUDING ROOT CHILDREN
	//must still be stored in the registry, so it is a quick O(1) search
	EntityData* localEntity = m_registry.TryGetEntityMutable(id);
	if (localEntity != nullptr) return localEntity;

	localEntity= TryGetGlobalEntityManagerMutable().TryGetGlobalEntityMutable(id);
	return localEntity;
}

EntityData* Scene::TryGetEntityMutable(const std::string& name, const bool& ignoreCase)
{
	const std::string targetName = ignoreCase ? Utils::StringUtil(name).ToLowerCase().ToString() : name;
	std::string currentLowercaseName = "";
	//LogError(std::format("Trying to look for entity: {} -> {} ignorecase: {}", name, targetName, std::to_string(ignoreCase)));
	for (auto& localEntity : m_localRootEntities)
	{
		if (localEntity == nullptr) continue;
		if (ignoreCase)
		{
			currentLowercaseName = Utils::StringUtil(localEntity->m_Name).ToLowerCase().ToString();
			/*LogError(std::format("Found local entity with name:{} when looking:{} ==: {}",
				currentLowercaseName, targetName, std::to_string(currentLowercaseName== targetName)));*/
			if (currentLowercaseName == targetName) return localEntity;
		}
		else if (localEntity->m_Name == targetName)
			return localEntity;
	}
	//Note: we do NOT need to do ignore case for global entities since they already have unique
	//names so their names already are cleaned to be as simple as possible
	return TryGetGlobalEntityManagerMutable().TryGetGlobalEntityMutable(targetName);
}

const EntityData* Scene::TryGetEntity(const std::string& name, const bool& ignoreCase) const
{
	std::string targetName = ignoreCase ? Utils::StringUtil(name).ToLowerCase().ToString() : name;
	std::string currentLowercaseName = "";
	for (auto& localEntity : m_localRootEntities)
	{
		if (ignoreCase)
		{
			currentLowercaseName = Utils::StringUtil(localEntity->m_Name).ToLowerCase().ToString();
			if (currentLowercaseName == targetName) return localEntity;
		}
		else if (localEntity->m_Name == targetName)
			return localEntity;
	}
	//Note: we do NOT need to do ignore case for global entities since they already have unique
	//names so their names already are cleaned to be as simple as possible
	return  TryGetGlobalEntityManager().TryGetGlobalEntity(name);
}

std::string Scene::ToStringLayers() const
{
	//Log(std::format("Began to stirng alyers fro scene. first layer: {}", m_Layers[0].m_SquaredTextBuffer.GetSize().ToString()));
	std::string result = "\n" + m_sceneName + ":\n";

	for (const auto& layer : m_layers)
	{
		result += "\nLAYER: ";
		//Log(std::format("Display all scene layers at layer: {}", layer.second.ToString()));
		result += layer.second.ToString();
	}
	return result;
}

std::string Scene::ToStringEntityData() const
{
	std::vector<std::string> sceneStr = {};
	for (const auto& entity : m_localRootEntities)
	{
		if (entity == nullptr) continue;
		sceneStr.push_back(entity->ToString());
	}
	return std::format("[\n-----GLOBALS----: {} \n----Local-----: {}]", 
		TryGetGlobalEntityManager().ToStringEntityData(), 
		Utils::ToStringIterable(sceneStr));
}

void Scene::ResetFrameDirtyComponentCount()
{
	m_currentFrameDirtyComponents = 0;
}

void Scene::IncreaseFrameDirtyComponentCount()
{
	m_currentFrameDirtyComponents++;
}

int Scene::GetDirtyComponentCount() const
{
	return m_currentFrameDirtyComponents;
}

bool Scene::HasDirtyComponents() const
{
	return GetDirtyComponentCount() > 0;
}

bool Scene::Validate()
{
	bool passesValidation = true;
	for (auto& entity : m_localRootEntities)
	{
		if (entity == nullptr) continue;
		if (!entity->Validate()) passesValidation = false;
	}

	for (auto& entity : TryGetGlobalEntityManagerMutable().GetAllGlobalEntitiesMutable())
	{
		if (entity == nullptr) continue;
		if (!entity->Validate()) passesValidation = false;
	}

	return passesValidation;
}

std::string Scene::ToString() const
{
	std::vector<std::string> currentEntitiesStr = {};
	for (const auto& entity : GetLocalEntities())
	{
		if (entity == nullptr) continue;

		currentEntitiesStr.push_back(entity->ToString());
		//LogError(std::format("Found entity str: {}", entitiesStringified.back()));
	}
	std::string localEntitiesStr = Utils::ToStringIterable(currentEntitiesStr);
	
	currentEntitiesStr.clear();
	for (const auto& entity : TryGetGlobalEntityManager().GetAllGlobalEntities())
	{
		if (entity == nullptr) continue;
		currentEntitiesStr.push_back(entity->ToString());
	}
	std::string globalEntitiesStr = Utils::ToStringIterable(currentEntitiesStr);
	
	return std::format("[Scene Globals:{} Local:{}]", globalEntitiesStr, localEntitiesStr);
}