#include "pch.hpp"
#include <deque>
#include "Core/Asset/SceneAsset.hpp"
#include "Core/Serialization/JsonUtils.hpp"
#include "Core/Serialization/Serializer.hpp"
#include "ECS/Component/Types/World/TransformComponent.hpp"
#include "ECS/Component/Types/World/AnimatorComponent.hpp"
#include "ECS/Component/Types/World/CameraComponent.hpp"
#include "ECS/Component/Types/World/EntityRenderer2DComponent.hpp"
#include "ECS/Component/Types/World/LightSource2DComponent.hpp"
#include "ECS/Component/Types/World/PhysicsBodyComponent.hpp"
#include "ECS/Component/Types/World/PlayerComponent.hpp"
#include "ECS/Component/Types/World/CollisionBoxComponent.hpp"
#include "ECS/Component/Types/World/SpriteAnimatorComponent.hpp"
#include "ECS/Component/Types/World/ParticleEmitterComponent.hpp"
#include "ECS/Component/Types/World/Mesh3DComponent.hpp"
#include "Core/Asset/AssetManager.hpp"
#include "Core/Scene/SceneManager.hpp"
#include "Core/EngineState.hpp"
#include "Utils/IOHandler.hpp"
#include "Core/Visual/VisualDataParser.hpp"
#include "ECS/Component/GlobalComponentInfo.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "Utils/ToStringFunctions.hpp"

namespace Engine::Scenes
{
	const std::array<std::string_view,1> SceneAsset::EXTENSIONS = {".json"};
	const std::string SceneAsset::LEVEL_EXTENSION = ".level";

	static const char* LEVEL_GROUND_PROPERTY_NAME = "Ground";
	static const char* LEVEL_BACKGOUND_PROPERTY_NAME = "Background";

	SceneAsset::SceneAsset(const std::filesystem::path& path) :
		Asset(path), m_assetManager(nullptr), m_scene(std::nullopt), m_levelFilePath()
	{
		ASSET_EXTENSION_CHECK

		std::filesystem::path maybePath = GetAbsolutePath().parent_path() / (GetName() + LEVEL_EXTENSION);
		if (::Utils::IO::DoesPathExist(maybePath))
		{
			m_levelFilePath = maybePath;
			Assets::AssetManager::SetAssetHiddenStatus(maybePath, true);
		}
	}

	Assets::AssetManager& SceneAsset::GetAssetManagerMutable()
	{
		if (!Assert(m_assetManager != nullptr, "Tried to retrieve asset manager MUTABLE from scene asset:{} "
			"but the asset manager has no reference yet due to dependencies for this asset not initialized", ToString()))
		{
			throw std::invalid_argument("Invalid asset manager scene asset dependency");
		}

		return *m_assetManager;
	}

	Scene& SceneAsset::GetSceneMutable()
	{
		if (!Assert(m_scene.has_value(), "Tried to retrieve scene MUTABLE from scene asset:{} "
			"but its asset has not been created yet due to dependencies for this asset not initialized", ToString()))
		{
			throw std::invalid_argument("Invalid scene asset dependency");
		}

		return m_scene.value();
	}
	const Scene& SceneAsset::GetScene() const
	{
		if (!Assert(m_scene.has_value(), "Tried to retrieve scene from scene asset:{} "
			"but its asset has not been created yet due to dependencies for this asset not initialized", ToString()))
		{
			throw std::invalid_argument("Invalid scene asset dependency");
		}

		return m_scene.value();
	}

	void SceneAsset::SetDependencies(Core::EngineState& state)
	{
		m_assetManager = state.m_AssetManager;
		m_scene = Scene(GetName(), state.m_SceneManager->m_GlobalEntityManager);
	}

	void SceneAsset::UpdateAssetFromFile()
	{
		std::string fullFile = ::Utils::IO::TryReadFileFull(GetAbsolutePathCopy());
		if (fullFile.empty())
		{
			TryLoadLevelBackground();
			return;
		}
		Json json = Json::parse(fullFile);
		Json EntityDatasJson = {};
		Json currentComponentJson = {};
		std::string componentName = "";
		std::string entityName = "";

		Serialization::JsonDeserializerImpl jsonDeserializer = {};
		Serialization::Deserializer deserializer = Serialization::Deserializer(&jsonDeserializer);

		bool isTransformComponent = false;
		ECS::EntityData* currentEntity = nullptr;
		ECS::Component* componentCreated = nullptr;

		//Note: we want this to be a deque since it has queue-like behavior, but we still need indexing
		//so it must be double ended, and cannot be a hashmap for this reason
		std::deque<std::tuple<std::function<void()>, ECS::Component*>> delayedSiblingDependencies;
		Event<void> delayedEntityDependencies;

		//NOTE: all entities extracted will be local (since globals are not associated with any single scene)
		Json entitiesJson = json.at("Entities");
		//TODO: maybe parsing entity should be extracted into a separate function in entity?
		for (Json& entityJson : entitiesJson)
		{
			entityName = entityJson.at("Name");
			EntityDatasJson = entityJson.at("Components");
			delayedSiblingDependencies = {};

			//LogError(std::format("Found compoinents: {}", std::to_string(EntityDatasJson.size())));
			//TODO: maybe components should have to implement a parsing function for json?
			for (size_t i = 0; i < EntityDatasJson.size(); i++)
			{
				currentComponentJson = EntityDatasJson[i];
				jsonDeserializer.SetJson(currentComponentJson);

				componentName = currentComponentJson.at("Type").get<std::string>();
				isTransformComponent = componentName == ::Utils::ToStringTypeName<TransformComponent>();
				if (i == 0 && !Assert(isTransformComponent, "Tried to parse scene file at path: '{}' "
					"but found entity component that does not begin with Transform!", GetAbsolutePathCopy().string()))
					return;

				if (isTransformComponent)
				{
					TransformComponent newEntityTransfrom = {};
					newEntityTransfrom.Deserialize(deserializer);
					currentEntity = &(GetSceneMutable().CreateEntity(entityName, newEntityTransfrom));
					continue;
				}

				if (!Assert(currentEntity != nullptr, "Tried to parse scene file at path: '{}' "
					"for component: {} but current entity: {} is null", GetAbsolutePathCopy().string(), componentName, entityName))
					return;

				//NOTE: since we explicitly create transform for the entity, 
				if (!isTransformComponent)
				{
					componentCreated = ECS::Utils::AddOrGetComponentToEntityByName(*currentEntity, componentName);
				}

				//if (componentCreated == nullptr) continue;
				if (!Assert(componentCreated != nullptr, "Tried to deserialize component: '{}' but reference stored after creation is NULL. "
					"This could mean the correct component was identified but it was not successfully added to the entity", componentName))
					return;

				//auto componentDependencies = componentCreated->GetComponentDependencies();
				if (!ECS::GlobalComponentInfo::DoesComponentHaveDependencies(componentCreated))
				{
					componentCreated->Deserialize(deserializer);
					//LogError(std::format("Created {} component: {}", ::Utils::FormatTypeName(typeid(*componentCreated).name()), componentCreated->ToString()));
				}
				else
				{
					std::function<void()> delayedAction = [componentCreated, &deserializer]() mutable-> void
						{
							componentCreated->Deserialize(deserializer);
						};

					if (ECS::GlobalComponentInfo::DoesComponentDependOnComponent(componentCreated)) 
						delayedSiblingDependencies.push_back(std::make_tuple(delayedAction, componentCreated));
					else if (ECS::GlobalComponentInfo::DoesComponentDependOnEntity(componentCreated)) 
						delayedEntityDependencies.AddListener(delayedAction);
				}
			}
			//Since some components may require dependencies on other components before they could be deserialized
			//we wait until all other non-dependent components are deserialized then we do the others
			if (!delayedSiblingDependencies.empty())
			{
				//TODO: sibling dependencies should check what components it depdends on and push them further back if those have not been created
				//in order to prevent delayed siblings depending on each other

				//We go backwards throguh each function (so if we add to front, we still get to it)
				//and we check if its dependencies have been added and if not, it means we must delay its execution until later
				//(the start) when that component is hopefully added by then and component deserialization function works
				for (int i = delayedSiblingDependencies.size() - 1; i >= 0; i--)
				{
					const ECS::Component* delayedComponent = std::get<1>(delayedSiblingDependencies[i]);
					if (!ECS::GlobalComponentInfo::DoesComponentHaveComponentDependencies(delayedComponent))
					{
						auto funcPairCopy = delayedSiblingDependencies[i];
						delayedSiblingDependencies.erase(delayedSiblingDependencies.begin() + i);
						delayedSiblingDependencies.push_front(funcPairCopy);

						continue;
					}

					//LogError(std::format("Invoking delayed func:{}", ::Utils::FormatTypeName(typeid(*std::get<1>(delayedSiblingDependencies[i])).name())));
					std::get<0>(delayedSiblingDependencies[i])();
				}
			}
		}

		if (delayedEntityDependencies.HasListeners()) delayedEntityDependencies.Invoke();
		TryLoadLevelBackground();
	}

	void SceneAsset::SaveToPath(const std::filesystem::path& path)
	{
		SaveSceneToPath(GetScene(), path);
	}

	bool SceneAsset::TryLoadLevelBackground()
	{
		//TODO: right now we expect the level to have the same name but with different extension
		if (m_levelFilePath.empty())
			return false;

		FigFormat::Fig levelFig = FigFormat::Fig(m_levelFilePath);
		Rendering::VisualData groundVisual = Rendering::ParseDefaultVisualData(levelFig.TryGetBaldValue(LEVEL_GROUND_PROPERTY_NAME));
		if (!Assert(!groundVisual.IsEmpty(), "Tried to parse level ground for scene asset:{} "
			"but resulted in empty visual data when using fig value:{} visual data:{}", ToString(),
			::Utils::ToStringIterable(levelFig.TryGetBaldValue(LEVEL_GROUND_PROPERTY_NAME)), groundVisual.ToString()))
			return false;

		ECS::EntityData& groundEntity = GetSceneMutable().CreateEntity("Ground", TransformComponent(Vec3{ 0,-10, 0 }));
		groundEntity.m_IsSerializable = false;
		Rendering::EntityRenderer2DComponent& groundRenderer = groundEntity.AddComponent(
			Rendering::EntityRenderer2DComponent(groundVisual, Rendering::RenderLayerType::Background));

		Physics::CollisionBoxComponent& groundCollisionBox = groundEntity.AddComponent(Physics::CollisionBoxComponent(groundVisual.GetWorldSize(), { 0,0 }));
		Physics::PhysicsBodyComponent& groundBody = groundEntity.AddComponent(Physics::PhysicsBodyComponent(&groundCollisionBox, 10));
		groundBody.SetConstraint(Physics::MoveContraints(true, true));

		Rendering::VisualData backgroundVisual = Rendering::ParseDefaultVisualData(levelFig.TryGetBaldValue(LEVEL_BACKGOUND_PROPERTY_NAME));
		if (!Assert(!backgroundVisual.IsEmpty(), "Tried to parse level background for scene asset:{} "
			"but resulted in empty visual data when using fig value:{} visual data:{}", ToString(),
			::Utils::ToStringIterable(levelFig.TryGetBaldValue(LEVEL_BACKGOUND_PROPERTY_NAME)), backgroundVisual.ToString()))
			return false;

		ECS::EntityData& backgroundEntity = GetSceneMutable().CreateEntity("Background", TransformComponent(Vec3{ 0,5, 0 }));
		backgroundEntity.m_IsSerializable = false;
		Rendering::EntityRenderer2DComponent& backgroundRenderer = backgroundEntity.AddComponent(
			Rendering::EntityRenderer2DComponent(backgroundVisual, Rendering::RenderLayerType::Background));
		return true;
	}

	void SaveSceneToPath(const Scene& scene, const std::filesystem::path& path)
	{
		//TODO: floats should not be fully serialized and should be partially cut off

		Json sceneJson = {};
		Json currentEntityJson = {};
		//NOTE: by making these ordered, we can keep the the order the properties are added,
		//whcih is especially useful for bwing consistent if editing is necesssary
		JsonOrdered currentComponentOrderedJsonFull = {};
		JsonOrdered currentComponentOrderedJson = {};
		Json currentComponentJson = {};
		std::string componentName = "";

		Serialization::JsonSerializerImpl jsonSerializer = {};
		Serialization::Serializer serializer = Serialization::Serializer(&jsonSerializer);

		for (auto& entity : scene.GetLocalEntities())
		{
			if (entity == nullptr || !entity->m_IsSerializable) continue;
			currentEntityJson = {};
			currentEntityJson["Name"] = entity->m_Name;

			for (const auto& component : entity->GetAllComponents())
			{
				if (component == nullptr) continue;

				currentComponentOrderedJsonFull = {};
				currentComponentOrderedJson = {};
				componentName = ::Utils::FormatTypeName(typeid(*component).name());
				currentComponentOrderedJsonFull["Type"] = componentName;
				
				jsonSerializer.ClearJson();
				component->Serialize(serializer);
				currentComponentJson = jsonSerializer.GetJson();
				if (currentComponentJson.empty())
				{
					LogError(std::format("Tried to serialize scene asset but ran into error while converting component"
						"of type: {} for entity: {}", componentName, entity->m_Name));
					return;
				}
				//NOTE: since by default json does not have .begin() or .end() we must convert regular json to a ordered list of properties
				currentComponentOrderedJson = currentComponentJson;
				if (!Assert(!currentComponentOrderedJson.empty(), "Tried to deserialize scene asset for entity:{} "
					"at component:{} but its component json is empty!", entity->m_Name, componentName))
					return;

				currentComponentOrderedJsonFull.insert(currentComponentOrderedJson.begin(), currentComponentOrderedJson.end());
				currentEntityJson["Components"].push_back(currentComponentOrderedJsonFull);
			}
			sceneJson["Entities"].push_back(currentEntityJson);
		}

		if (sceneJson.empty())
		{
			LogWarning(std::format("Failed to save scene asset:{} to path:{} "
				"because resulting json was null", scene.ToString(), path.string()));
			return;
		}
		::Utils::IO::TryWriteFile(path, sceneJson.dump());
	}
}
