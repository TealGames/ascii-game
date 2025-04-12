#include "pch.hpp"
#include <deque>
#include "SceneAsset.hpp"
#include "JsonUtils.hpp"
#include "JsonSerializers.hpp"
#include "RaylibUtils.hpp"

#include "AnimatorData.hpp"
#include "CameraData.hpp"
#include "EntityRendererData.hpp"
#include "LightSourceData.hpp"
#include "PhysicsBodyData.hpp"
#include "PlayerData.hpp"
#include "CollisionBoxData.hpp"
#include "SpriteAnimatorData.hpp"
#include "UIObjectData.hpp"
#include "ParticleEmitterData.hpp"

const std::string SceneAsset::SCENE_EXTENSION = ".json";
const std::string SceneAsset::LEVEL_EXTENSION = ".level";

SceneAsset::SceneAsset(const std::filesystem::path& path) : 
	Asset(path), m_assetManager(nullptr), m_scene(std::nullopt) {}

AssetManager& SceneAsset::GetAssetManagerMutable()
{
	if (!Assert(this, m_assetManager!=nullptr, std::format("Tried to retrieve asset manager MUTABLE from scene asset:{} "
		"but the asset manager has no reference yet due to dependencies for this asset not initialized", ToString())))
	{
		throw std::invalid_argument("Invalid asset manager scene asset dependency");
	}

	//LogError(std::format("Scene has value:{}", m_scene.value().ToString()));
	return *m_assetManager;
}

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

void SceneAsset::SetDependencies(GlobalEntityManager& globalManager, AssetManager& assetManager)
{
	m_assetManager = &assetManager;
	m_scene = Scene(GetName(), globalManager);
	MarkDependenciesSet();
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

	std::deque<std::tuple<std::function<void()>, ComponentData*>> delayedSiblingDependencies;
	Event<void> delayedEntityDependencies;

	//NOTE: all entities extracted will be local (since globals are not associated with any single scene)
	Json entitiesJson = json.at("Entities");
	//TODO: maybe parsing entity should be extracted into a separate function in entity?
	for (Json& entityJson : entitiesJson)
	{
		entityName = entityJson.at("Name");
		entityComponentsJson = entityJson.at("Components");
		delayedSiblingDependencies = {};

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
			else if (componentName == Utils::GetTypeName<ParticleEmitterData>())
			{
				componentCreated = &(currentEntity->AddComponent<ParticleEmitterData>());
			}
			else if (componentName == Utils::GetTypeName<CollisionBoxData>())
			{
				componentCreated = &(currentEntity->AddComponent<CollisionBoxData>());
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

			auto componentDependencies = componentCreated->GetComponentDependencies();
			if (!componentCreated->HasDependencies())
			{
				componentCreated->Deserialize(currentComponentJson);
				LogError(std::format("Created {} component: {}", Utils::FormatTypeName(typeid(*componentCreated).name()), componentCreated->ToString()));
			}
			else
			{
				std::function<void()> delayedAction = [componentCreated, currentComponentJson]() mutable-> void
					{
						LogError("INSIDE DELAWED ACTION FUNC");
						LogError("Deserializing delayed component:{} dependencies:{} has dependencies:{}", 
							Utils::FormatTypeName(typeid(*componentCreated).name())), 
							Utils::ToStringIterable<std::vector<std::string>, std::string>(componentCreated->GetDependencyFlags()), 
							std::to_string(componentCreated->DoesEntityHaveComponentDependencies());

						componentCreated->Deserialize(currentComponentJson);
						//LogError(std::format("Created {} component: {}", Utils::FormatTypeName(typeid(*componentCreated).name()), componentCreated->ToString()));
					};

				if (componentCreated->DependsOnAnySiblingComponent()) delayedSiblingDependencies.push_back(std::make_tuple(delayedAction, componentCreated));
				else if (componentCreated->DependsOnEntity()) delayedEntityDependencies.AddListener(delayedAction);
			}
		}

		LogError(std::format("Entity:{} has sibling dependenceis:{}", currentEntity->GetName(), std::to_string(delayedSiblingDependencies.size())));

		//Since some components may require dependencies on other components before they could be deserialized
		//we wait until all other non-dependent components are deserialized then we do the others
		if (!delayedSiblingDependencies.empty())
		{
			
			//TODO: sibling dependencies should check what components it depdends on and push them further back if those have not been created
			//in order to prevent delayed siblings depending on each other

			//We go backwards throguh each function (so if we add to front, we still get to it)
			//and we check if its dependencies have been added and if not, it means we must delay its execution until later
			//(the start) when that component is hopefully added by then and component deserialization function works
			for (int i= delayedSiblingDependencies.size()-1; i>=0; i--)
			{
				const ComponentData* delayedComponent = std::get<1>(delayedSiblingDependencies[i]);
				if (!delayedComponent->DoesEntityHaveComponentDependencies())
				{
					auto funcPairCopy = delayedSiblingDependencies[i];
					delayedSiblingDependencies.erase(delayedSiblingDependencies.begin() + i);
					delayedSiblingDependencies.push_front(funcPairCopy);

					continue;
				}

				LogError(std::format("Invoking delayed func:{}", Utils::FormatTypeName(typeid(*std::get<1>(delayedSiblingDependencies[i])).name())));
				std::get<0>(delayedSiblingDependencies[i])();
			}
		}
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
		if (entity == nullptr || !entity->m_IsSerializable) continue;
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
				else if (componentName == Utils::GetTypeName<ParticleEmitterData>())
				{
					serializedComponentJson = dynamic_cast<ParticleEmitterData*>(component)->Serialize();
				}
				else if (componentName == Utils::GetTypeName<CollisionBoxData>())
				{
					serializedComponentJson = dynamic_cast<CollisionBoxData*>(component)->Serialize();
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
					"of type : {} for entity : {}. Error : {}", componentName, entity->GetName(), e.what()));
				return {};
			}
			LogError(std::format("Created component json:{}", JsonUtils::ToStringProperties(serializedComponentJson)));

			if (!Assert(this, !serializedComponentJson.empty(), std::format("Tried to deserialize scene asset for entity:{} "
				"at component:{} but its component json is empty!", entity->GetName(), componentName)))
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
	TryLoadLevelBackground();

	//Log("Creating new layer in scene");

	LogError(std::format("SCENE LOADED:{}", GetScene().ToString()));
	//Assert(false, "ENDED LAODING SCENE");
}

bool SceneAsset::TryLoadLevelBackground()
{
	//TODO: right now we expect the level to have the same name but with different extension
	std::filesystem::path maybePath = GetAssetManagerMutable().TryGetAssetPath(GetName(), LEVEL_EXTENSION);
	if (!Assert(this, !maybePath.empty(), std::format("Attempted to load level background for scene asset:{} "
		"but could not find level from asset manager using name:{} extension:{}", ToString(), GetName(), LEVEL_EXTENSION)))
		return false;

	std::ifstream fstream(maybePath);
	std::vector<std::vector<TextCharPosition>> visualPositions = {};

	int r = 0;
	std::string currentLine = "";
	std::unordered_map<std::string, Color> colorAliases = {};
	const std::string keyHeader = "key:";
	const std::string sceneHeader = "level:";
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
			if (!Assert(maybeConvertedHex.has_value(), std::format("Tried to parse level background fro scene asset: {}, but encountered "
				"unparsable hex: '{}' at line: {}", ToString(), hexString, std::to_string(lineIndex)))) continue;

			Color convertedColor = RaylibUtils::GetColorFromHex(maybeConvertedHex.value());
			//Log(std::format("Found the color: {} from hex: {}", RaylibUtils::ToString(convertedColor), hexString));
			colorAliases.emplace(colorAlias, convertedColor);
		}
		else
		{
			visualPositions.push_back({});
			//std::cout << "ALLOC with line: "<<currentLine<< std::endl;
			//if (currentLine.size() > maxLineChars) maxLineChars = currentLine.size();

			for (int i = 0; i < currentLine.size(); i++)
			{
				if (currentLine[i] == '\t' || currentLine[i]==' ') continue;
				//We need to make sure there is at least 2 chars in front for at least one for alias and one for ending symbol
				if (currentLine[i] == charColorAliasStart && i< currentLine.size()-2)
				{
					int colorAliasEndIndex = currentLine.find(charColorAliasEnd, i + 1);
					if (!Assert(colorAliasEndIndex != std::string::npos, std::format("Tried to parse a color alias for level background for scene asset: {} at line: {} "
						"but did not find color alias end at color alias start at index: {}",
						ToString(), std::to_string(lineIndex), std::to_string(i)))) continue;

					std::string colorAlias = currentLine.substr(i + 1, colorAliasEndIndex - (i + 1));
					if (!Assert(colorAliases.find(colorAlias) != colorAliases.end(), std::format("Tried to parse a color alias for level background "
						"for scene asset : {} at line : {} but color alias: {} starting at index:{} has no color data defined in KEY section",
						ToString(), std::to_string(lineIndex), colorAlias, std::to_string(i + 1))))
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
				visualPositions.back().push_back(TextCharPosition{ Array2DPosition(r, i), TextChar(currentColor, currentLine[i])});
			}
			r++;
		}
	}

	//We then create the background entity based off the the visual positions in the asset
	const VisualData backgroundVisual = VisualData(visualPositions, GetGlobalFont(), VisualData::DEFAULT_FONT_SIZE,
		VisualData::DEFAULT_CHAR_SPACING, VisualData::DEFAULT_PREDEFINED_CHAR_AREA, VisualData::PIVOT_CENTER);

	ECS::Entity& backgroundEntity = GetSceneMutable().CreateEntity("Background", TransformData(Vec2{ 0,-10 }));
	backgroundEntity.m_IsSerializable = false;
	EntityRendererData& backgroundRenderer = backgroundEntity.AddComponent<EntityRendererData>(EntityRendererData(backgroundVisual, RenderLayerType::Background));

	LogWarning(std::format("Created Backgorund: {}", backgroundRenderer.GetVisualData().m_Text.ToString()));
	LogWarning(std::format("Creating backgrounf entity: {} from rednerer: {}", backgroundEntity.GetName(), backgroundRenderer.m_Entity->GetName()));

	CollisionBoxData& collisionBox = backgroundEntity.AddComponent<CollisionBoxData>(CollisionBoxData(backgroundEntity.m_Transform, backgroundVisual.GetWorldSize(), { 0,0 }));
	PhysicsBodyData& physicsBody = backgroundEntity.AddComponent<PhysicsBodyData>(PhysicsBodyData(collisionBox, 10));
	physicsBody.SetConstraint(MoveContraints(true, true));
	/*LogWarning(std::format("Created Physics body: {} visual size: {}", physicsBody.GetAABB().ToString(backgroundEntity.m_Transform.m_Pos), 
		backgroundVisual.m_Text.GetSize().ToString()));*/
}

void SceneAsset::Unload()
{
	//TODO: implement
}