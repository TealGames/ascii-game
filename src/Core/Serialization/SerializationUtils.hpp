#pragma once
#include "Core/Serialization/SerializedObjects.hpp"
#include "Core/Asset/Asset.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "Core/ID.hpp"

namespace Engine::Scenes { class SceneManager; }
namespace Engine::Assets { class AssetManager; }
namespace Engine::ECS { class EntityData; }
namespace Engine::Serialization
{
	void InitSerializationUtils(Scenes::SceneManager& manager, Assets::AssetManager& assetManager);

	SerializedAsset TrySerializeAsset(const Assets::Asset* asset);
	std::optional<SerializedAsset> TrySerializeOptionalAsset(const Assets::Asset* asset);
	std::vector<SerializedAsset> TrySerializeAssets(const std::vector<const Assets::Asset*>& assets);

	Assets::Asset* TryDeserializeAsset(const SerializedAsset& asset);
	Assets::Asset* TryDeserializeAsset(const ObjectID& id);

	template<typename T>
	requires (!std::is_pointer_v<T> && Engine::Assets::IsAssetType<T>)
	T* TryDeserializeTypeAsset(const SerializedAsset& asset)
	{
		if (T* typePtr = dynamic_cast<T*>(TryDeserializeAsset(asset)))
			return typePtr;
		return nullptr;
	}
	template<typename T>
	requires (!std::is_pointer_v<T>&& Engine::Assets::IsAssetType<T>)
	T* TryDeserializeOptionalTypeAsset(const std::optional<SerializedAsset>& asset)
	{
		if (!asset.has_value())
			return nullptr;
		return TryDeserializeTypeAsset<T>(asset.value());
	}
	template<typename T>
	requires (!std::is_pointer_v<T>&& Engine::Assets::IsAssetType<T>)
	T* TryDeserializeTypeAsset(const ObjectID& id)
	{
		if (T* typePtr = dynamic_cast<T*>(TryDeserializeAsset(id)))
			return typePtr;
		return nullptr;
	}
	template<typename T>
	requires (!std::is_pointer_v<T> && Engine::Assets::IsAssetType<T>)
	std::vector<T*> TryDeserializeTypeAssets(const std::vector<SerializedAsset>& serializedAssets)
	{
		std::vector<T*> assets = {};
		for (const auto& serializedAsset : serializedAssets)
		{
			assets.push_back(TryDeserializeAsset(serializedAsset));
		}
		return assets;
	}

	std::optional<SerializedEntity> TrySerializeEntity(const ECS::EntityData* entity, bool isOptional = false);
	ECS::EntityData* TryDeserializeEntity(const std::optional<SerializedEntity>& entity, bool isOptional = false);

	template<typename T>
	requires (!std::is_pointer_v<T> && std::is_base_of_v<ECS::Component, T>)
	T* TryDeserializeComponent(const std::optional<SerializedComponent>& maybeSerializedComponent, ECS::EntityData& targetEntity, bool isOptional = false)
	{
		if (!maybeSerializedComponent.has_value())
		{
			if (!isOptional)
			{
				LogError("Tried to deserialize component from SerializedComponent but component is "
					"NULL even with a NON OPTIONAL functional call");
			}
			return nullptr;
		}

		SerializedComponent serializedComponent = maybeSerializedComponent.value();
		//NOTE: `targetEntity` is the entity that has its component deserialized AND `maybeSerializedComponent` is
		//the serialized component data found for `targetEntity` so `maybeSerializedComponent`'s entity may NOT
		//necessarily be `targetEntity`, but if it is, we can take a shortcut and not need to go through scene manager
		//since we already have access the the entity
		if (targetEntity.m_Name == serializedComponent.m_SerializedEntity.m_EntityName &&
			targetEntity.m_SceneName == serializedComponent.m_SerializedEntity.m_SceneName)
		{
			return targetEntity.TryGetComponentMutable<T>();
		}

		ECS::EntityData* entity = TryDeserializeEntity(serializedComponent.m_SerializedEntity);
		if (!Assert(entity != nullptr, "Tried to deserialize component from serialized format: {} but failed to retrieve entity",
			serializedComponent.ToString()))
			return nullptr;

		ECS::Component* componentData = entity->TryGetComponentWithNameMutable(serializedComponent.m_ComponentName);
		if (!Assert(componentData != nullptr, "Tried to deserialzie component from serialized format: {} but failed to retrieve component:{}",
			serializedComponent.ToString(), serializedComponent.m_ComponentName))
			return nullptr;

		return dynamic_cast<T*>(componentData);
	}

	template<typename T>
	requires (!std::is_pointer_v<T> && std::is_base_of_v<ECS::Component, T>)
	std::optional<SerializedComponent> TrySerializeComponent(const T* component, bool isOptional = false)
	{
		if (component == nullptr)
		{
			if (!isOptional)
			{
				LogError("Tried to serialize component to SerializedComponent but component is "
					"NULL even with a NON OPTIONAL functional call");
			}
			return std::nullopt;
		}

		const ECS::Component* componentBase = static_cast<const ECS::Component*>(component);
		const ECS::EntityData& entity = componentBase->GetEntity();
		const std::string componentName = entity.TryGetComponentName(componentBase);
		return SerializedComponent(entity.m_SceneName, entity.m_Name, componentName);
	}
}