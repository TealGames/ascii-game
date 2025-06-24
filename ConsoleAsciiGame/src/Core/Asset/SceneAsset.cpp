#include "pch.hpp"
#include <deque>
#include "Core/Asset/SceneAsset.hpp"
#include "Core/Serialization/JsonUtils.hpp"
#include "Core/Serialization/JsonSerializers.hpp"
#include "ECS/Component/Types/World/AnimatorData.hpp"
#include "ECS/Component/Types/World/CameraData.hpp"
#include "ECS/Component/Types/World/EntityRendererData.hpp"
#include "ECS/Component/Types/World/LightSourceData.hpp"
#include "ECS/Component/Types/World/PhysicsBodyData.hpp"
#include "ECS/Component/Types/World/PlayerData.hpp"
#include "ECS/Component/Types/World/CollisionBoxData.hpp"
#include "ECS/Component/Types/World/SpriteAnimatorData.hpp"
#include "ECS/Component/Types/World/ParticleEmitterData.hpp"
#include "Core/Asset/AssetManager.hpp"
#include "Utils/IOHandler.hpp"
#include "Core/Visual/VisualDataParser.hpp"
#include "ECS/Component/GlobalComponentInfo.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"

const std::string SceneAsset::EXTENSION = ".json";
const std::string SceneAsset::LEVEL_EXTENSION = ".level";

static const char* LEVEL_BACKGROUND_PROPERTY_NAME = "Level";

SceneAsset::SceneAsset(const std::filesystem::path& path) : 
	Asset(path, true), m_assetManager(nullptr), m_scene(std::nullopt) 
{
	if (!Assert(this, IO::DoesPathHaveExtension(path, EXTENSION), std::format("Tried to create a scene asset from path:'{}' "
		"but it does not have required extension:'{}'", path.string(), EXTENSION)))
		return;
}

AssetManagement::AssetManager& SceneAsset::GetAssetManagerMutable()
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

void SceneAsset::SetDependencies(GlobalEntityManager& globalManager, AssetManagement::AssetManager& assetManager)
{
	m_assetManager = &assetManager;
	m_scene = Scene(GetName(), globalManager);
	MarkDependenciesSet();
	//LogError(std::format("Set scene dependencies: {}", m_scene.value().ToString()));
}

void SceneAsset::UpdateAssetFromFile()
{
	Json json = Json::parse(IO::TryReadFileFull(GetPathCopy()));

	Json entityComponentsJson = {};
	Json currentComponentJson = {};
	std::string componentName = "";
	std::string entityName = "";

	bool isTransformComponent = false;
	EntityData* currentEntity = nullptr;
	Component* componentCreated = nullptr;

	//Note: we want this to be a deque since it has queue-like behavior, but we still need indexing
	//so it must be double ended, and cannot be a hashmap for this reason
	std::deque<std::tuple<std::function<void()>, Component*>> delayedSiblingDependencies;
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
				"but found entity component that does not begin with Transform!", GetPathCopy().string())))
				return;

			if (isTransformComponent)
			{
				TransformData newEntityTransfrom = {};
				newEntityTransfrom.Deserialize(currentComponentJson);
				currentEntity = &(GetSceneMutable().CreateEntity(entityName, TransformData(newEntityTransfrom)));
				//LogError(std::format("Found component json; {} for entoty; {} entity json: {}", 
				//JsonUtils::ToStringProperties(currentComponentJson), JsonUtils::ToStringProperties(entityComponentsJson), entityName));

				//currentEntity->TryGetComponentMutable<TransformData>()->Deserialize(currentComponentJson);
				//LogError(std::format("Found transform for: {} entity:{}", currentEntity->TryGetComponent<TransformData>()->ToString(), currentEntity->GetName()));
				continue;
			}

			if (!Assert(this, currentEntity != nullptr, std::format("Tried to parse scene file at path: '{}' "
				"for component: {} but current entity: {} is null", GetPathCopy().string(), componentName, entityName)))
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
					"but no component by that name exists!", componentName, entityName, GetPathCopy().string()));
				return;
			}

			//if (componentCreated == nullptr) continue;
			if (!Assert(this, componentCreated != nullptr, std::format("Tried to deserialize component but reference stored after creation is NULL. "
				"This could mean the correct component was identified but it was not successfully added to the entity")))
				return;

			//auto componentDependencies = componentCreated->GetComponentDependencies();
			if (!GlobalComponentInfo::DoesComponentHaveDependencies(componentCreated))
			{
				componentCreated->Deserialize(currentComponentJson);
				LogError(std::format("Created {} component: {}", Utils::FormatTypeName(typeid(*componentCreated).name()), componentCreated->ToString()));
			}
			else
			{
				std::function<void()> delayedAction = [componentCreated, currentComponentJson]() mutable-> void
					{
						/*LogError("INSIDE DELAWED ACTION FUNC");
						LogError("Deserializing delayed component:{} dependencies:{} has dependencies:{}", 
							Utils::FormatTypeName(typeid(*componentCreated).name())), 
							Utils::ToStringIterable<std::vector<std::string>, std::string>(componentCreated->GetDependencyFlags()), 
							std::to_string(componentCreated->DoesEntityHaveComponentDependencies());*/

						componentCreated->Deserialize(currentComponentJson);
						//LogError(std::format("Created {} component: {}", Utils::FormatTypeName(typeid(*componentCreated).name()), componentCreated->ToString()));
					};

				if (GlobalComponentInfo::DoesComponentDependOnComponent(componentCreated)) delayedSiblingDependencies.push_back(std::make_tuple(delayedAction, componentCreated));
				else if (GlobalComponentInfo::DoesComponentDependOnEntity(componentCreated)) delayedEntityDependencies.AddListener(delayedAction);
			}
		}

		//LogError(std::format("Entity:{} has sibling dependenceis:{}", currentEntity->GetName(), std::to_string(delayedSiblingDependencies.size())));

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
				const Component* delayedComponent = std::get<1>(delayedSiblingDependencies[i]);
				if (!GlobalComponentInfo::DoesComponentHaveComponentDependencies(delayedComponent))
				{
					auto funcPairCopy = delayedSiblingDependencies[i];
					delayedSiblingDependencies.erase(delayedSiblingDependencies.begin() + i);
					delayedSiblingDependencies.push_front(funcPairCopy);

					continue;
				}

				//LogError(std::format("Invoking delayed func:{}", Utils::FormatTypeName(typeid(*std::get<1>(delayedSiblingDependencies[i])).name())));
				std::get<0>(delayedSiblingDependencies[i])();
			}
		}
	}

	if (delayedEntityDependencies.HasListeners()) delayedEntityDependencies.Invoke();
	TryLoadLevelBackground();
	//Assert(false, std::format("ENDED DESERIALIZATION"));
}

void SceneAsset::SaveToPath(const std::filesystem::path& path)
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
		currentEntityJson["Name"] = entity->m_Name;

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
						"but no component by that name exists!", componentName, entity->m_Name, GetPathCopy().string()));
					return;
				}
			}
			catch (const std::exception& e)
			{
				LogError(this, std::format("Tried to serialize scene asset but ran into error while converting component"
					"of type : {} for entity : {}. Error : {}", componentName, entity->m_Name, e.what()));
				return;
			}
			LogError(std::format("Created component json:{}", JsonUtils::ToStringProperties(serializedComponentJson)));

			if (!Assert(this, !serializedComponentJson.empty(), std::format("Tried to deserialize scene asset for entity:{} "
				"at component:{} but its component json is empty!", entity->m_Name, componentName)))
				return;

			currentComponentJson.insert(serializedComponentJson.begin(), serializedComponentJson.end());
			currentEntityJson["Components"].push_back(currentComponentJson);
		}
		json["Entities"].push_back(currentEntityJson);
	}
	LogError(std::format("Resulting json : {}", JsonUtils::ToStringProperties(json)));
	
	IO::TryWriteFile(path, json.dump());
}

bool SceneAsset::TryLoadLevelBackground()
{
	//TODO: right now we expect the level to have the same name but with different extension
	std::filesystem::path maybePath = GetAssetManagerMutable().TryGetAssetPath(GetName(), LEVEL_EXTENSION);
	if (!Assert(this, !maybePath.empty(), std::format("Attempted to load level background for scene asset:{} "
		"but could not find level from asset manager using name:{} extension:{}", ToString(), GetName(), LEVEL_EXTENSION)))
		return false;

	//std::ifstream fstream(maybePath);
	//std::vector<std::vector<TextCharArrayPosition>> visualPositions = {};
	Fig levelFig = Fig(maybePath);
	//Assert(false, std::format("Level fig:{}", levelFig.ToString()));

	VisualData backgroundVisual = ParseDefaultVisualData(levelFig.TryGetBaldValue(LEVEL_BACKGROUND_PROPERTY_NAME));
	if (!Assert(this, !backgroundVisual.IsEmpty(), std::format("Tried to parse level background for scene asset:{} "
		"but resulted in empty visual data when using fig value:{} visual data:{}", ToString(), 
		Utils::ToStringIterable<FigValue, std::string>(levelFig.TryGetBaldValue(LEVEL_BACKGROUND_PROPERTY_NAME)), backgroundVisual.ToString())))
		return false;

	EntityData& backgroundEntity = GetSceneMutable().CreateEntity("Background", TransformData(Vec2{ 0,-10 }));
	backgroundEntity.m_IsSerializable = false;
	EntityRendererData& backgroundRenderer = backgroundEntity.AddComponent<EntityRendererData>(EntityRendererData(backgroundVisual, RenderLayerType::Background));

	/*LogWarning(std::format("Created Backgorund: {}", backgroundRenderer.GetVisualData().ToString()));
	LogWarning(std::format("Creating backgrounf entity: {} from rednerer: {}", backgroundEntity.GetName(), backgroundRenderer.m_Entity->GetName()));*/

	CollisionBoxData& collisionBox = backgroundEntity.AddComponent<CollisionBoxData>(CollisionBoxData(backgroundEntity.GetTransform(), backgroundVisual.GetWorldSize(), {0,0}));
	PhysicsBodyData& physicsBody = backgroundEntity.AddComponent<PhysicsBodyData>(PhysicsBodyData(collisionBox, 10));
	physicsBody.SetConstraint(MoveContraints(true, true));
	/*LogWarning(std::format("Created Physics body: {} visual size: {}", physicsBody.GetAABB().ToString(backgroundEntity.m_Transform.m_Pos), 
		backgroundVisual.m_Text.GetSize().ToString()));*/
}