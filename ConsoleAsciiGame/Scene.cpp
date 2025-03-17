#include "pch.hpp"
#include <filesystem>
#include <fstream>
#include "raylib.h"
#include "Point2DInt.hpp"
#include "Scene.hpp"
#include "TextBuffer.hpp"
#include "HelperFunctions.hpp"
#include "RaylibUtils.hpp"
#include "Globals.hpp"
#include "Event.hpp"
#include "Array2DPosition.hpp"
#include "VisualData.hpp"
#include "PhysicsBodyData.hpp"
#include "TransformData.hpp"
#include "StringUtil.hpp"

#include "nlohmann/json.hpp"
#include "JsonUtils.hpp"
using Json = nlohmann::json;

#include "AnimatorData.hpp"
#include "CameraData.hpp"
#include "EntityRendererData.hpp"
#include "LightSourceData.hpp"
#include "PhysicsBodyData.hpp"
#include "PlayerData.hpp"
#include "SpriteAnimatorData.hpp"
#include "UIObjectData.hpp"

const std::string Scene::SCENE_FILE_PREFIX = "scene_";

Scene::Scene(const std::filesystem::path& scenePath, GlobalEntityManager& globalEntities) :
	m_layers{}, m_sceneName(""), m_scenePath(scenePath),
	m_localEntities(), m_localEntityIdLookup(), m_localEntityNameLookup(),//m_globalEntityLookup(globalEntities),
	m_currentFrameDirtyComponents(0), m_entityMapper(),
	m_mainCamera(nullptr), 
	m_globalEntities(globalEntities)
	//m_physicsWorld()
{
	if (!Assert(std::filesystem::exists(scenePath), std::format("Tried to create a scene at path: {} "
		"but that path does not exist", scenePath.string()))) 
		return;

	if (!Assert(scenePath.has_filename(), std::format("Tried to create a scene at path: {} "
		"but that path does not lead to a file", scenePath.string()))) 
		return;

	m_sceneName = ExtractSceneName(scenePath);

	m_localEntities.reserve(MAX_ENTITIES);
	m_localEntityNameLookup.reserve(MAX_ENTITIES);
	m_localEntityIdLookup.reserve(MAX_ENTITIES);

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

std::string Scene::ExtractSceneName(const std::filesystem::path& path)
{
	std::string sceneName= path.stem().string().substr(SCENE_FILE_PREFIX.size() - 1);
	//std::replace(sceneName.begin(), sceneName.end(), '_', ' ');
	return sceneName;
}
/*
void Scene::ParseSceneFile(std::ifstream& fstream,  
	std::vector<std::vector<TextCharPosition>>& layerText) const
{
	int r = 0;
	std::string currentLine = "";
	std::unordered_map<std::string, Color> colorAliases = {};
	const std::string keyHeader = "key:";
	const std::string sceneHeader = "scene:";
	const char charColorAliasStart = '[';
	const char charColorAliasEnd = ']';
	bool isParsingKey = false;

	int lineIndex = -1;
	const Color defaultColor = BLACK;
	Color currentColor = defaultColor;
	while (std::getline(fstream, currentLine))
	{
		lineIndex++;
		if (currentLine.empty()) continue;

		if (currentLine == keyHeader) isParsingKey = true;
		else if (currentLine == sceneHeader) isParsingKey = false;

		else if (isParsingKey)
		{
			std::size_t equalsSignIndex = currentLine.find('=');
			std::string colorAlias = currentLine.substr(0, equalsSignIndex);
			std::string hexString = currentLine.substr(equalsSignIndex + 1);
			std::optional<uint32_t> maybeConvertedHex = Utils::TryParseHex<uint32_t>(hexString);
			if (!Assert(maybeConvertedHex.has_value(), std::format("Tried to parse scene data: {}, but encountered "
				"unparsable hex: '{}' at line: {}", m_SceneName, hexString, std::to_string(lineIndex)))) continue;

			Color convertedColor = RaylibUtils::GetColorFromHex(maybeConvertedHex.value());
			//Log(std::format("Found the color: {} from hex: {}", RaylibUtils::ToString(convertedColor), hexString));
			colorAliases.emplace(colorAlias, convertedColor);
		}
		else
		{
			layerText.push_back({});
			//std::cout << "ALLOC with line: "<<currentLine<< std::endl;
			//if (currentLine.size() > maxLineChars) maxLineChars = currentLine.size();

			for (int i = 0; i < currentLine.size(); i++)
			{
				if (currentLine[i] == '\t' || currentLine[i]==' ') continue;
				//We need to make sure there is at least 2 chars in front for at least one for alias and one for ending symbol
				if (currentLine[i] == charColorAliasStart && i< currentLine.size()-2)
				{
					int colorAliasEndIndex = currentLine.find(charColorAliasEnd, i + 1);
					if (!Assert(colorAliasEndIndex != std::string::npos, std::format("Tried to parse a color alias for scene data: {} at line: {} "
						"but did not find color alias end at color alias start at index: {}", 
						m_SceneName, std::to_string(lineIndex), std::to_string(i)))) continue;

					std::string colorAlias = currentLine.substr(i + 1, colorAliasEndIndex - (i + 1));
					if (!Assert(colorAliases.find(colorAlias) != colorAliases.end(), std::format("Tried to parse a color alias for scene data: {} at line: {} "
						"but color alias: {} starting at index:{} has no color data defined in KEY section",
						m_SceneName, std::to_string(lineIndex), colorAlias, std::to_string(i + 1))))
					{
						i = colorAliasEndIndex;
						continue;
					}

					//Log(std::format("Found good color alias: {}", colorAlias));
					currentColor = colorAliases[colorAlias];
					i = colorAliasEndIndex;
					continue;
				}

				//TODO: what is the best way of doing this? putting in text chars and putting empty chars 
				//which would work fine for init but hard to create collision bound 
				//OR do we leave empty spots and put them in with positions?
				layerText.back().push_back(TextCharPosition{ Array2DPosition(r, i), TextChar(currentColor, currentLine[i])});
			}
			r++;
		}
	}
}
*/

std::string Scene::GetName() const
{
	return m_sceneName;
}
GlobalEntityManager& Scene::GetGlobalEntityManager()
{
	return m_globalEntities;
}

void Scene::Deserialize(const Json& json)
{
	Json entityComponentsJson = {};
	Json currentComponentJson = {};
	std::string componentName = "";
	std::string entityName = "";

	bool isTransformComponent = false;
	ECS::Entity* currentEntity = nullptr;
	ComponentData* componentCreated = nullptr;

	Event<void> delayedSiblingDependencies;
	Event<void> delayedEntityDependencies;

	//NOTE: all entities extracted will be local (since globals are not associated with any single scene)
	Json entitiesJson = json.at("Entities");
	//TODO: maybe parsing entity should be extracted into a separate function in entity?
	for (Json& entityJson : entitiesJson)
	{
		entityName = entityJson.at("Name");
		entityComponentsJson = entityJson.at("Components");
		delayedSiblingDependencies.RemoveAllListeners();

		LogError(std::format("Found compoinents: {}", std::to_string(entityComponentsJson.size())));
		//TODO: maybe components should have to implement a parsing function for json?
		for (size_t i = 0; i < entityComponentsJson.size(); i++)
		{
			currentComponentJson = entityComponentsJson[i];
			componentName = currentComponentJson.at("Type").get<std::string>();
			isTransformComponent = componentName == Utils::GetTypeName<TransformData>();
			if (i == 0 && !Assert(isTransformComponent, std::format("Tried to parse scene file at path: '{}' "
				"but found entity component that does not begin with Transform!", m_scenePath.string())))
				return;

			if (isTransformComponent)
			{
				TransformData newEntityTransfrom = {};
				newEntityTransfrom.Deserialize(currentComponentJson);
				currentEntity = &(CreateEntity(entityName, TransformData(newEntityTransfrom)));
				//LogError(std::format("Found component json; {} for entoty; {} entity json: {}", 
				//JsonUtils::ToStringProperties(currentComponentJson), JsonUtils::ToStringProperties(entityComponentsJson), entityName));

				//currentEntity->TryGetComponentMutable<TransformData>()->Deserialize(currentComponentJson);
				LogError(std::format("Found transform for: {} entity:{}", currentEntity->TryGetComponent<TransformData>()->ToString(), currentEntity->GetName()));
				continue;
			}

			if (!Assert(this, currentEntity != nullptr, std::format("Tried to parse scene file at path: '{}' "
				"for component: {} but current entity: {} is null", m_scenePath.string(), componentName, entityName)))
				return;

			if (componentName == Utils::GetTypeName<AnimatorData>())
			{
				componentCreated = &(currentEntity->AddComponent<AnimatorData>());
			}
			else if (componentName == Utils::GetTypeName<CameraData>())
			{
				componentCreated = &(currentEntity->AddComponent<CameraData>());
			}
			else if (componentName == Utils::GetTypeName<EntityRendererData>())
			{
				componentCreated = &(currentEntity->AddComponent<EntityRendererData>());
				LogError(std::format("Deserialized entity renderer: {} to: {}", JsonUtils::ToStringProperties(currentComponentJson),
					currentEntity->TryGetComponent<EntityRendererData>()->ToString()));
			}
			else if (componentName == Utils::GetTypeName<LightSourceData>())
			{
				componentCreated = &(currentEntity->AddComponent<LightSourceData>());
			}
			else if (componentName == Utils::GetTypeName<PhysicsBodyData>())
			{
				componentCreated = &(currentEntity->AddComponent<PhysicsBodyData>());
			}
			else if (componentName == Utils::GetTypeName<PlayerData>())
			{
				componentCreated = &(currentEntity->AddComponent<PlayerData>());
			}
			else if (componentName == Utils::GetTypeName<SpriteAnimatorData>())
			{
				componentCreated = &(currentEntity->AddComponent<SpriteAnimatorData>());
			}
			else if (componentName == Utils::GetTypeName<UIObjectData>())
			{
				componentCreated = &(currentEntity->AddComponent<UIObjectData>());
			}
			else
			{
				Assert(false, std::format("Tried to parse component:'{}' of entity:'{} 'to scene file at path: '{}', "
					"but no component by that name exists!", componentName, entityName, m_scenePath.string()));
				return;
			}

			//if (componentCreated == nullptr) continue;
			if (!Assert(this, componentCreated != nullptr, std::format("Tried to deserialize component but reference stored after creation is NULL. "
				"This could mean the correct component was identified but it was not successfully added to the entity")))
				return;

			HighestDependecyLevel componentDependencies = componentCreated->GetDependencyLevel();
			if (componentDependencies == HighestDependecyLevel::None)
			{
				componentCreated->Deserialize(currentComponentJson);
				LogError(std::format("Created component: {}", componentCreated->ToString()));
			}
			else
			{
				std::function<void()> delayedAction = [componentCreated, currentComponentJson]() mutable-> void
					{
						LogError("Deserializing delayed component");
						componentCreated->Deserialize(currentComponentJson);
						LogError(std::format("Created component: {}", componentCreated->ToString()));
					};

				if (componentDependencies == HighestDependecyLevel::SiblingComponent) delayedSiblingDependencies.AddListener(delayedAction);
				else delayedEntityDependencies.AddListener(delayedAction);
			}
		}

		//Since some components may require dependencies on other components before they could be deserialized
		//we wait until all other non-dependent components are deserialized then we do the others
		if (delayedSiblingDependencies.HasListeners()) delayedSiblingDependencies.Invoke();
	}

	if (delayedEntityDependencies.HasListeners()) delayedEntityDependencies.Invoke();
	//Assert(false, std::format("After scene: {} was created: found: {}", m_sceneName, ToString()));
}
void Scene::Load()
{
	std::string currentLine = "";
	std::string fullJson = "";
	std::ifstream fstream(m_scenePath);

	while (std::getline(fstream, currentLine))
	{
		fullJson += currentLine;
	}

	Json parsedJson = Json::parse(fullJson);

	Deserialize(parsedJson);

	//Log("Creating new layer in scene");
	const VisualData backgroundVisual = VisualData({}, GetGlobalFont(), VisualData::DEFAULT_FONT_SIZE,
		VisualData::DEFAULT_CHAR_SPACING, VisualData::DEFAULT_PREDEFINED_CHAR_AREA, VisualData::PIVOT_CENTER);
	ECS::Entity& backgroundEntity = CreateEntity("Background", TransformData(Vec2{ 0,0 }));
	EntityRendererData& backgroundRenderer = backgroundEntity.AddComponent<EntityRendererData>(EntityRendererData(backgroundVisual, RenderLayerType::Background));
	LogWarning(std::format("Created Backgorund: {}", backgroundRenderer.GetVisualData().m_Text.ToString()));
	LogWarning(std::format("Creating backgrounf entity: {} from rednerer: {}", backgroundEntity.GetName(), backgroundRenderer.m_Entity->GetName()));

	PhysicsBodyData& physicsBody = backgroundEntity.AddComponent<PhysicsBodyData>(PhysicsBodyData(5, backgroundVisual.GetWorldSize(), { 0,0 }));
	LogWarning(std::format("Created Physics body: {} visual size: {}",
		physicsBody.GetAABB().ToString(backgroundEntity.m_Transform.m_Pos), backgroundVisual.m_Text.GetSize().ToString()));

	//Assert(false, "ENDED LAODING SCENE");
}

void Scene::Unload()
{
	m_mainCamera = nullptr;
	//m_physicsWorld.ClearAllBodies();

	m_localEntityIdLookup = {};
	m_localEntityNameLookup = {};
	m_localEntities.clear();
	m_localEntities.shrink_to_fit();

	m_entityMapper.clear();
}

Json Scene::Serialize()
{
	//TODO: implmenet
	return {};
}

//void Scene::InitScene()
//{
//	for (auto& entity : m_localEntities)
//	{
//		if (PhysicsBodyData* maybeBody = entity.TryGetComponentMutable<PhysicsBodyData>())
//		{
//			m_physicsWorld.AddBody(*maybeBody);
//		}
//	}
//
//	for (auto& entity : m_globalEntities.GetAllGlobalEntitiesMutable())
//	{
//		if (PhysicsBodyData* maybeBody = entity.TryGetComponentMutable<PhysicsBodyData>())
//		{
//			m_physicsWorld.AddBody(*maybeBody);
//		}
//	}
//}

std::vector<RenderLayer*> Scene::GetLayersMutable()
{
	if (m_layers.empty()) return {};

	std::vector<RenderLayer*> layers = {};
	for (auto& layer : m_layers) layers.push_back(&(layer.second));
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

void Scene::AddToLayer(const RenderLayerType& layers, TextBufferPosition& textBufferPos)
{
	if (layers == RenderLayerType::None) return;

	for (auto& layer : m_layers)
	{
		if ((layer.first & layers) == 0) continue;
		layer.second.AddText(textBufferPos);
		return;
	}
}

std::vector<TextBufferMixed*> Scene::GetLayerBufferMutable(const RenderLayerType& renderLayers)
{
	std::vector<TextBufferMixed*> buffers = {};
	if (renderLayers == RenderLayerType::None) return buffers;

	for (auto& layer : m_layers)
	{
		if ((layer.first & renderLayers) == 0) continue;
		buffers.push_back(&layer.second.GetBufferMutable());
	}
	return buffers;
}

void Scene::ResetAllLayers()
{
	for (auto& layer : m_layers) layer.second.ResetToDefault();
}

EntityIDCollection::iterator Scene::GetLocalEntityIterator(const ECS::EntityID& id)
{
	return m_localEntityIdLookup.find(id);
}
EntityNameCollection::iterator Scene::GetLocalEntityIterator(const std::string& name)
{
	return m_localEntityNameLookup.find(name);
}

class CameraData;
void Scene::SetMainCamera(ECS::Entity& cameraEntity)
{
	if (!Assert(cameraEntity.HasComponent<CameraData>(), 
		std::format("Tried to set the non-camera entity: {} as the main camera for scene: {}",
			cameraEntity.GetName(), m_sceneName))) return;

	m_mainCamera = &cameraEntity;
}

bool Scene::HasMainCamera() const
{
	return m_mainCamera != nullptr;
}

ECS::Entity* Scene::TryGetMainCameraEntityMutable()
{
	return m_mainCamera;
}

CameraData* Scene::TryGetMainCameraMutable()
{
	if (!HasMainCamera()) return nullptr;
	return m_mainCamera->TryGetComponentMutable<CameraData>();
}

const CameraData* Scene::TryGetMainCamera() const
{
	if (!HasMainCamera()) return nullptr;
	return m_mainCamera->TryGetComponent<CameraData>();
}

int Scene::GetEntityCount() const
{
	return m_localEntities.size() + m_globalEntities.GetCount();
	/*return m_entityMapper.va + m_globalEntities.GetCount();*/
}

bool Scene::HasEntities() const
{
	return GetEntityCount() > 0;
}

const std::vector<const ECS::Entity*> Scene::GetAllEntities() const
{
	std::vector<const ECS::Entity*> entities = GetLocalEntities();

	for (const auto& globalEntity : m_globalEntities.GetAllGlobalEntities())
		entities.push_back(&globalEntity);
	
	return entities;
}

const std::vector<const ECS::Entity*> Scene::GetLocalEntities() const
{
	std::vector<const ECS::Entity*> entities = {};
	for (const auto& localEntity : m_localEntities)
		entities.push_back(&localEntity);

	return entities;
}

std::vector<ECS::Entity*> Scene::GetAllEntitiesMutable()
{
	std::vector<ECS::Entity*> entities = {};
	for (auto& entity : m_globalEntities.GetAllGlobalEntitiesMutable())
	{
		entities.push_back(&entity);
	}

	for (auto& entity : m_localEntities)
	{
		entities.push_back(&entity);
	}
	return entities;
}

ECS::Entity& Scene::CreateEntity(const std::string& name, TransformData& transform)
{
	ECS::Entity& addedEntity= m_localEntities.emplace_back(name, m_entityMapper, transform);
	//addedEntity.m_Transform.m_Entity = &(addedEntity);
	addedEntity.SetScene(m_sceneName);

	m_localEntityNameLookup.emplace(addedEntity.GetName(), &(addedEntity));
	m_localEntityIdLookup.emplace(addedEntity.m_Id, &(addedEntity));

	return addedEntity;
}

ECS::Entity& Scene::CreateEntity(const std::string& name, TransformData&& transform)
{
	ECS::Entity& addedEntity = m_localEntities.emplace_back(name, m_entityMapper, std::move(transform));
	//addedEntity.m_Transform.m_Entity = &(addedEntity);
	addedEntity.SetScene(m_sceneName);

	m_localEntityNameLookup.emplace(addedEntity.GetName(), &(addedEntity));
	m_localEntityIdLookup.emplace(addedEntity.m_Id, &(addedEntity));

	return addedEntity;
}

bool Scene::HasEntity(const ECS::EntityID& id)
{
	return m_entityMapper.valid(id) || m_globalEntities.HasGlobalEntity(id);
	//bool isLocal = GetLocalEntityIterator(id) != m_localEntityLookup.end();
	//if (isLocal) return true;

	//return m_globalEntities.HasGlobalEntity(id);
}

//TODO: while this is fine, we should add version that allow us to add components to the entity
//and would probably be best if we create them on the heap probably to extend memory lifetime

//TODO: also maybe consider managing what objects are required to be in a scene, like a camera
ECS::Entity* Scene::TryGetEntityMutable(const ECS::EntityID& id)
{
	auto localIt = GetLocalEntityIterator(id);
	if (localIt != m_localEntityIdLookup.end()) return localIt->second;

	auto globalIt = m_globalEntities.GetGlobalEntityIteratorMutable(id);
	if (m_globalEntities.IsValidIterator(globalIt)) return globalIt->second;

	return nullptr;
}

ECS::Entity* Scene::TryGetEntityMutable(const std::string& name, const bool& ignoreCase)
{
	std::string targetName = ignoreCase ? Utils::StringUtil(name).ToLowerCase().ToString() : name;
	std::string currentLowercaseName = "";
	//LogError(std::format("Trying to look for entity: {} -> {} ignorecase: {}", name, targetName, std::to_string(ignoreCase)));
	for (auto& localEntity : m_localEntities)
	{
		if (ignoreCase)
		{
			currentLowercaseName = Utils::StringUtil(localEntity.GetName()).ToLowerCase().ToString();
			/*LogError(std::format("Found local entity with name:{} when looking:{} ==: {}",
				currentLowercaseName, targetName, std::to_string(currentLowercaseName== targetName)));*/
			if (currentLowercaseName == targetName) return &localEntity;
		}
		else if (localEntity.GetName() == targetName)
			return &localEntity;
	}
	//Note: we do NOT need to do ignore case for global entities since they already have unique
	//names so their names already are cleaned to be as simple as possible
	return m_globalEntities.TryGetGlobalEntityMutable(name);
}

const ECS::Entity* Scene::TryGetEntity(const std::string& name, const bool& ignoreCase) const
{
	std::string targetName = ignoreCase ? Utils::StringUtil(name).ToLowerCase().ToString() : name;
	std::string currentLowercaseName = "";
	for (auto& localEntity : m_localEntities)
	{
		if (ignoreCase)
		{
			currentLowercaseName = Utils::StringUtil(localEntity.GetName()).ToLowerCase().ToString();
			if (currentLowercaseName == targetName) return &localEntity;
		}
		else if (localEntity.GetName() == targetName)
			return &localEntity;
	}
	//Note: we do NOT need to do ignore case for global entities since they already have unique
	//names so their names already are cleaned to be as simple as possible
	return m_globalEntities.TryGetGlobalEntity(name);
}

std::string Scene::ToStringLayers() const
{
	//Log(std::format("Began to stirng alyers fro scene. first layer: {}", m_Layers[0].m_SquaredTextBuffer.GetSize().ToString()));
	std::string result = "\n" + m_sceneName + ":\n";

	for (const auto& layer : m_layers)
	{
		result += "\nLAYER: ";
		Log(std::format("Display all scene layers at layer: {}", layer.second.ToString()));
		result += layer.second.ToString();
	}
	return result;
}

std::string Scene::ToStringEntityData() const
{
	return "NULL";
	/*return std::format("[Global: {} Local: {}]", 
		m_globalEntities.ToStringEntityData(), m_entityMapper.ToStringData());*/
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

//const Physics::PhysicsWorld& Scene::GetPhysicsWorld() const
//{
//	return m_physicsWorld;
//}
//
//Physics::PhysicsWorld& Scene::GetPhysicsWorldMutable()
//{
//	return m_physicsWorld;
//}

bool Scene::Validate()
{
	bool passesValidation = true;
	for (auto& entity : m_localEntities)
	{
		if (!entity.Validate()) passesValidation = false;
	}

	for (auto& entity : m_globalEntities.GetAllGlobalEntitiesMutable())
	{
		if (!entity.Validate()) passesValidation = false;
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
		//LogError(this, std::format("Found entity str: {}", entitiesStringified.back()));
	}
	std::string localEntitiesStr = Utils::ToStringIterable<std::vector<std::string>, std::string>(currentEntitiesStr);
	
	currentEntitiesStr.clear();
	for (const auto& entity : m_globalEntities.GetAllGlobalEntities())
	{
		currentEntitiesStr.push_back(entity.ToString());
	}
	std::string globalEntitiesStr = Utils::ToStringIterable<std::vector<std::string>, std::string>(currentEntitiesStr);
	
	return std::format("[Scene Globals:{} Local:{}]", globalEntitiesStr, localEntitiesStr);
}