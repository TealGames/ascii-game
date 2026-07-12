#include "Core/Serialization/SerializationUtils.hpp"
#include "Core/Scene/SceneManager.hpp"
#include "Core/Asset/AssetManager.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"

namespace Engine::Serialization
{
	Scenes::SceneManager* SceneManager = nullptr;
	Assets::AssetManager* AssetManager = nullptr;

	void InitSerializationUtils(Scenes::SceneManager& manager, Assets::AssetManager& assetManager)
	{
		SceneManager = &manager;
		AssetManager = &assetManager;
	}

	SerializedAsset TrySerializeAsset(const Assets::Asset* asset)
	{
		ENGINE_ASSERT(asset != nullptr, "Attempted to serialize asset but it is null");
		return asset->GetAbsolutePath();
	}
	std::optional<SerializedAsset> TrySerializeOptionalAsset(const Assets::Asset* asset)
	{
		if (asset == nullptr)
			return std::nullopt;
		return TrySerializeAsset(asset);
	}
	
	std::vector<SerializedAsset> TrySerializeAssets(const std::vector<const Assets::Asset*>& assets)
	{
		std::vector<SerializedAsset> assetsSerialized;
		for (const auto& asset : assets)
		{
			if (asset == nullptr) continue;
			assetsSerialized.push_back(TrySerializeAsset(asset));
		}
		return assetsSerialized;
	}

	Assets::Asset* TryDeserializeAsset(const SerializedAsset& asset)
	{
		ENGINE_ASSERT(AssetManager != nullptr, "Tried to deserialize asset: {} but AssetManager is null", asset.ToString());
		return AssetManager->TryGetAssetFromPathMutable(asset.m_AssetPath);
	}
	Assets::Asset* TryDeserializeAsset(const ObjectID& id)
	{
		ENGINE_ASSERT(AssetManager != nullptr, "Tried to deserialize asset but AssetManager is null");
		return AssetManager->TryGetAssetFromIdMutable(id);
	}

	std::optional<SerializedEntity> TrySerializeEntity(const ECS::EntityData* entity, bool isOptional)
	{
		if (entity == nullptr)
		{
			if (!isOptional)
			{
				LogError("Tried to serialize entity to SerializedEntity but entity is "
					"NULL even with a NON OPTIONAL functional call");
			}
			return std::nullopt;
		}

		return SerializedEntity{ entity->m_SceneName, entity->m_Name };
	}

	ECS::EntityData* TryDeserializeEntity(const std::optional<SerializedEntity>& maybeSerializedEntity, bool isOptional)
	{
		if (!maybeSerializedEntity.has_value())
		{
			if (!isOptional)
			{
				LogError("Tried to deserialize entity from SerializedEntity but entity is "
					"NULL even with a NON OPTIONAL functional call");
			}
			return nullptr;
		}

		ENGINE_ASSERT(SceneManager != nullptr, "Tried to parse entity from serialized entity "
			"but parser does not contain valid scene manager");

		SerializedEntity serializedEntity = maybeSerializedEntity.value();
		if (serializedEntity.m_SceneName == ECS::EntityData::GLOBAL_SCENE_NAME)
		{
			return SceneManager->m_GlobalEntityManager.TryGetGlobalEntityMutable(serializedEntity.m_EntityName);
		}

		Scenes::Scene* maybeScene = SceneManager->TryGetSceneMutable(serializedEntity.m_SceneName);
		if (!Assert(maybeScene != nullptr, "Tried to deserialize entity with non global scene : '{}', "
			"but no scene matches that name", serializedEntity.m_SceneName))
			return nullptr;

		ECS::EntityData* maybeEntity = maybeScene->TryGetEntityMutable(serializedEntity.m_EntityName);
		if (maybeEntity == nullptr)
		{
			if (!Assert(maybeScene->GetEntityCount() > 0, "Tried to deserialize entity with non glboal scene:'{}', "
				"but no entities exist in that scene. It could be because that scene was no loaded yet "
				"(and another scene tried to create a reference to an entity)", serializedEntity.m_SceneName))
				return nullptr;

			LogError(std::format("Tried to deserialize entity with non glboal scene:'{}', "
				"but no entities with that name exist!", serializedEntity.m_SceneName));
			return nullptr;
		}

		return maybeEntity;
	}
}