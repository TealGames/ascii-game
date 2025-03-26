#include "pch.hpp"
#include "SceneAsset.hpp"
#include "JsonUtils.hpp"
#include "JsonSerializers.hpp"

#include "AnimatorData.hpp"
#include "CameraData.hpp"
#include "EntityRendererData.hpp"
#include "LightSourceData.hpp"
#include "PhysicsBodyData.hpp"
#include "PlayerData.hpp"
#include "SpriteAnimatorData.hpp"
#include "UIObjectData.hpp"

const std::string SceneAsset::EXTENSION = ".json";

SceneAsset::SceneAsset(const std::filesystem::path& path) : 
	Asset(path), m_scene(std::nullopt) {}

Scene& SceneAsset::GetSceneMutable()
{
	if (!Assert(this, m_scene.has_value(), std::format("Tried to retrieve scene MUTABLE from scene asset:{} "
		"but its asset has not been created yet due to dependencies for this asset not initialized", ToString())))
	{
		throw std::invalid_argument("Invalid scene asset dependency");
	}

	//LogError(std::format("Scene has value:{}", m_scene.value().ToString()));
	return m_scene.value();
}
const Scene& SceneAsset::GetScene() const
{
	if (!Assert(this, m_scene.has_value(), std::format("Tried to retrieve scene from scene asset:{} "
		"but its asset has not been created yet due to dependencies for this asset not initialized", ToString())))
	{
		throw std::invalid_argument("Invalid scene asset dependency");
	}

	//LogError(std::format("Scene has value:{}", m_scene.value().ToString()));
	return m_scene.value();
}

void SceneAsset::SetDependencies(GlobalEntityManager& globalManager)
{
	m_scene = Scene(GetName(), globalManager);
	//LogError(std::format("Set scene dependencies: {}", m_scene.value().ToString()));
}

void SceneAsset::Deserialize(const Json& json)
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

		//LogError(std::format("Found compoinents: {}", std::to_string(entityComponentsJson.size())));
		//TODO: maybe components should have to implement a parsing function for json?
		for (size_t i = 0; i < entityComponentsJson.size(); i++)
		{
			currentComponentJson = entityComponentsJson[i];
			componentName = currentComponentJson.at("Type").get<std::string>();
			isTransformComponent = componentName == Utils::GetTypeName<TransformData>();
			if (i == 0 && !Assert(isTransformComponent, std::format("Tried to parse scene file at path: '{}' "
				"but found entity component that does not begin with Transform!", GetPath().string())))
				return;

			if (isTransformComponent)
			{
				TransformData newEntityTransfrom = {};
				newEntityTransfrom.Deserialize(currentComponentJson);
				currentEntity = &(GetSceneMutable().CreateEntity(entityName, TransformData(newEntityTransfrom)));
				//LogError(std::format("Found component json; {} for entoty; {} entity json: {}", 
				//JsonUtils::ToStringProperties(currentComponentJson), JsonUtils::ToStringProperties(entityComponentsJson), entityName));

				//currentEntity->TryGetComponentMutable<TransformData>()->Deserialize(currentComponentJson);
				LogError(std::format("Found transform for: {} entity:{}", currentEntity->TryGetComponent<TransformData>()->ToString(), currentEntity->GetName()));
				continue;
			}

			if (!Assert(this, currentEntity != nullptr, std::format("Tried to parse scene file at path: '{}' "
				"for component: {} but current entity: {} is null", GetPath().string(), componentName, entityName)))
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
				Assert(this, false, std::format("Tried to DESERIALIZE component:'{}' of entity:'{} 'to scene file at path: '{}', "
					"but no component by that name exists!", componentName, entityName, GetPath().string()));
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
				LogError(std::format("Created {} component: {}", Utils::FormatTypeName(typeid(*componentCreated).name()), componentCreated->ToString()));
			}
			else
			{
				std::function<void()> delayedAction = [componentCreated, currentComponentJson]() mutable-> void
					{
						LogError("Deserializing delayed component");
						componentCreated->Deserialize(currentComponentJson);
						LogError(std::format("Created {} component: {}", Utils::FormatTypeName(typeid(*componentCreated).name()), componentCreated->ToString()));
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
	//Assert(false, std::format("ENDED DESERIALIZATION"));
}

Json SceneAsset::Serialize()
{
	//TODO: floats should not be fully serialized and should be partially cut off

	Json json = {};
	Json currentEntityJson = {};
	//NOTE: by making these ordered, we can keep the the order the properties are added,
	//whcih is especially useful for bwing consistent if editing is necesssary
	JsonOrdered currentComponentJson = {};
	JsonOrdered serializedComponentJson = {};
	//json["Entities"] = {};

	Scene& scene = GetSceneMutable();
	std::string componentName = "";

	for (auto& entity : scene.GetLocalEntitiesMutable())
	{
		if (entity == nullptr) continue;
		currentEntityJson = {};
		currentEntityJson["Name"] = entity->GetName();

		for (auto& component : entity->GetAllComponentsMutable())
		{
			if (component == nullptr) continue;

			currentComponentJson = {};
			serializedComponentJson = {};
			componentName = Utils::FormatTypeName(typeid(*component).name());
			currentComponentJson["Type"] = componentName;

			try
			{
				if (componentName == Utils::GetTypeName<TransformData>())
				{
					serializedComponentJson = dynamic_cast<TransformData*>(component)->Serialize();
				}
				else if (componentName == Utils::GetTypeName<AnimatorData>())
				{
					serializedComponentJson = dynamic_cast<AnimatorData*>(component)->Serialize();
				}
				else if (componentName == Utils::GetTypeName<CameraData>())
				{
					serializedComponentJson = dynamic_cast<CameraData*>(component)->Serialize();
				}
				else if (componentName == Utils::GetTypeName<EntityRendererData>())
				{
					serializedComponentJson = dynamic_cast<EntityRendererData*>(component)->Serialize();
					/*LogError(std::format("Serialized component entity renderer:{} converted:{} serialize:{}", 
						JsonUtils::ToStringProperties(serializedComponentJson), converted->ToString(), JsonUtils::ToStringProperties(serialized)));*/
				}
				else if (componentName == Utils::GetTypeName<LightSourceData>())
				{
					serializedComponentJson = dynamic_cast<LightSourceData*>(component)->Serialize();
				}
				else if (componentName == Utils::GetTypeName<PhysicsBodyData>())
				{
					serializedComponentJson = dynamic_cast<PhysicsBodyData*>(component)->Serialize();
				}
				else if (componentName == Utils::GetTypeName<PlayerData>())
				{
					serializedComponentJson = dynamic_cast<PlayerData*>(component)->Serialize();
				}
				else if (componentName == Utils::GetTypeName<SpriteAnimatorData>())
				{
					serializedComponentJson = dynamic_cast<SpriteAnimatorData*>(component)->Serialize();
				}
				else if (componentName == Utils::GetTypeName<UIObjectData>())
				{
					serializedComponentJson = dynamic_cast<UIObjectData*>(component)->Serialize();
				}
				else
				{
					Assert(false, std::format("Tried to SERIALIZE component:'{}' of entity:'{} 'to scene file at path: '{}', "
						"but no component by that name exists!", componentName, entity->GetName(), GetPath().string()));
					return {};
				}
			}
			catch (const std::exception& e)
			{
				LogError(this, std::format("Tried to serialize scene asset but ran into error while converting component"
					"of type : {} for entity : {}. Error : {}", componentName, entity->ToString(), e.what()));
				return {};
			}
			LogError(std::format("Created component json:{}", JsonUtils::ToStringProperties(serializedComponentJson)));

			if (!Assert(this, !serializedComponentJson.empty(), std::format("Tried to deserialize scene asset for entity:{} "
				"at component:{} but its component json is empty!", entity->ToString(), componentName)))
				return {};

			currentComponentJson.insert(serializedComponentJson.begin(), serializedComponentJson.end());
			currentEntityJson["Components"].push_back(currentComponentJson);
		}
		json["Entities"].push_back(currentEntityJson);
	}
	LogError(std::format("Resulting json : {}", JsonUtils::ToStringProperties(json)));
	return json;
}

void SceneAsset::SerializeToPath(const std::filesystem::path& path)
{
	std::ofstream stream(path);
	if (!Assert(this, !!stream, std::format("Tried to serialzie scene asset to path:{}"
		"but file could not be opened", path.string())))
		return;

	Json serializedJson = Serialize();
	stream << serializedJson;
	LogError(std::format("Serialized json:{}", JsonUtils::ToStringProperties(serializedJson)));
}
void SceneAsset::SerializeToSelf()
{
	SerializeToPath(GetPath());
}

void SceneAsset::Load()
{
	std::string currentLine = "";
	std::string fullJson = "";
	std::ifstream fstream(GetPath());

	while (std::getline(fstream, currentLine))
	{
		fullJson += currentLine;
	}

	Json parsedJson = Json::parse(fullJson);

	Deserialize(parsedJson);
	LogError(std::format("DESERIALIZE SCENE:{}", GetScene().ToString()));

	//Log("Creating new layer in scene");
	const VisualData backgroundVisual = VisualData({}, GetGlobalFont(), VisualData::DEFAULT_FONT_SIZE,
		VisualData::DEFAULT_CHAR_SPACING, VisualData::DEFAULT_PREDEFINED_CHAR_AREA, VisualData::PIVOT_CENTER);
	ECS::Entity& backgroundEntity = GetSceneMutable().CreateEntity("Background", TransformData(Vec2{0,0}));
	EntityRendererData& backgroundRenderer = backgroundEntity.AddComponent<EntityRendererData>(EntityRendererData(backgroundVisual, RenderLayerType::Background));
	LogWarning(std::format("Created Backgorund: {}", backgroundRenderer.GetVisualData().m_Text.ToString()));
	LogWarning(std::format("Creating backgrounf entity: {} from rednerer: {}", backgroundEntity.GetName(), backgroundRenderer.m_Entity->GetName()));

	PhysicsBodyData& physicsBody = backgroundEntity.AddComponent<PhysicsBodyData>(PhysicsBodyData(5, backgroundVisual.GetWorldSize(), { 0,0 }));
	LogWarning(std::format("Created Physics body: {} visual size: {}",
		physicsBody.GetAABB().ToString(backgroundEntity.m_Transform.m_Pos), backgroundVisual.m_Text.GetSize().ToString()));

	LogError(std::format("SCENE LOADED:{}", GetScene().ToString()));
	//Assert(false, "ENDED LAODING SCENE");
}

void SceneAsset::Unload()
{
	//TODO: implement
}