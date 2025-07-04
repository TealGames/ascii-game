#pragma once
#include "nlohmann/json.hpp"
#include "Utils/Data/Vec2.hpp"
#include "Utils/Data/Vec2Int.hpp"
#include "Core/Rendering/RenderLayer.hpp"
#include "Core/Visual/VisualData.hpp"
#include "Core/Visual/TextArray.hpp"
#include "Core/Serialization/SerializableEntity.hpp"
#include "ECS/Component/Component.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "ECS/Component/Types/World/AnimatorData.hpp"
#include "Core/Visual/SpriteAnimation.hpp"
#include "ECS/Component/ComponentFieldReference.hpp"
#include "Core/Serialization/SerializableField.hpp"
#include "Utils/Data/ColorGradient.hpp"
#include "Core/Serialization/JsonUtils.hpp"
#include "Core/Collision/AABB.hpp"
#include <functional>
#include <type_traits>
#include <cstdint>
#include <vector>
#include "Utils/Data/FloatRange.hpp"
#include "Core/Asset/Asset.hpp"

using Json = nlohmann::json;
using JsonOrdered = nlohmann::ordered_json;
namespace SceneManagement
{
	class SceneManager;
}
namespace AssetManagement
{
	class AssetManager;
}

inline const char* OPTIONAL_NULL_VALUE = "null";

void InitJsonSerializationDependencies(SceneManagement::SceneManager& manager, 
	AssetManagement::AssetManager& assetManager);

bool HasRequiredProperties(const Json& json, const std::vector<std::string>& propertyNames);

void from_json(const Json& json, Vec2& vec);
void to_json(Json& json, const Vec2& vec);

void from_json(const Json& json, Vec2Int& vec);
void to_json(Json& json, const Vec2Int& vec);

void from_json(const Json& json, FloatRange& range);
void to_json(Json& json, const FloatRange& range);

void from_json(const Json& json, Array2DPosition& pos);
void to_json(Json& json, const Array2DPosition& pos);

void from_json(const Json& json, RenderLayerType& layer);
void to_json(Json& json, const RenderLayerType& layer);

void from_json(const Json& json, Color& color);
void to_json(Json& json, const Color& color);

void from_json(const Json& json, ColorGradientKeyFrame& gradientFrame);
void to_json(Json& json, const ColorGradientKeyFrame& gradientFrame);

void from_json(const Json& json, ColorGradient& gradient);
void to_json(Json& json, const ColorGradient& gradient);

void from_json(const Json& json, TextChar& textChar);
void to_json(Json& json, const TextChar& textChar);

void from_json(const Json& json, TextCharArrayPosition& textChar);
void to_json(Json& json, const TextCharArrayPosition& textChar);

//std::optional<Font> TryDeserializeFont(const Json& json);
//Json TrySerializeFont(const Font& font);

void from_json(const Json& json, FontProperties& font);
void to_json(Json& json, const FontProperties& font);

void from_json(const Json& json, TextBufferCharPosition& textChar);
void to_json(Json& json, const TextBufferCharPosition& textChar);

void from_json(const Json& json, VisualData& visualData);
void to_json(Json& json, const VisualData& visualData);

void from_json(const Json& json, SpriteAnimationFrame& frame);
void to_json(Json& json, const SpriteAnimationFrame& frame);

void from_json(const Json& json, SpriteAnimation& anim);
void to_json(Json& json, const SpriteAnimation& anim);

namespace Physics
{
	void from_json(const Json& json, Physics::AABB& aabb);
	void to_json(Json& json, const Physics::AABB& aabb);
}

Json TrySerializeAsset(const Asset* asset);
Json TrySerializeAssets(const std::vector<const Asset*>& assets);

Asset* TryDeserializeAsset(const Json& json);
template<typename T>
requires (!std::is_pointer_v<T> && IsAssetType<T>)
T* TryDeserializeTypeAsset(const Json& json)
{
	try
	{
		return dynamic_cast<T*>(TryDeserializeAsset(json));
	}
	catch (const std::exception& e)
	{
		Assert(false, std::format("Tried to deserialize Asset into type:{} but ran into error:{}", 
			Utils::GetTypeName<T>(), e.what()));
	}
	return nullptr;
}

template<typename T>
requires (!std::is_pointer_v<T> && IsAssetType<T>)
std::vector<T*> TryDeserializeTypeAssets(const Json& json)
{
	try
	{
		std::vector<T*> assets = {};
		for (const auto& jsonProeprty : json.get<std::vector<std::string>>())
		{
			assets.push_back(TryDeserializeAsset(jsonProeprty));
		}
	}
	catch (const std::exception& e)
	{
		Assert(false, std::format("Tried to deserialize Asset into type:{} but ran into error:{}",
			Utils::GetTypeName<T>(), e.what()));
	}
	return {};
}

void from_json(const Json& json, SerializableEntity& serializableEntity);
void to_json(Json& json, const SerializableEntity& serializableEntity);

EntityData* TryDeserializeEntity(const Json& json, const bool& isOptional = false);
Json TrySerializeEntity(const EntityData* entity, const bool& isOptional = false);

void from_json(const Json& json, SerializableComponent& serializableComponent);
void to_json(Json& json, const SerializableComponent& serializableComponent);

void from_json(const Json& json, ComponentReference& fieldReference);
void to_json(Json& json, const ComponentReference& fieldReference);

template<typename T>
requires (!std::is_pointer_v<T> && std::is_base_of_v<Component, T>)
T* TryDeserializeComponent(const Json& json, EntityData& entitySelf, const bool& isOptional = false)
{
	SerializableComponent serializableComponent = json.get<SerializableComponent>();
	if (serializableComponent.IsComponentOfEntitySelf()) 
		return TryDeserializeComponentSelf<T>(json, entitySelf, isOptional);

	std::function<T* (const Json&)> deserializationAction = [&serializableComponent](const Json& json)-> T*
		{
			try
			{
				EntityData* entity = TryDeserializeEntity(json);
				if (!Assert(entity != nullptr, std::format("Tried to deserialize component from json:{} but failed to retrieve entity",
					JsonUtils::ToStringProperties(json))))
					return nullptr;

				Component* componentData = entity->TryGetComponentWithNameMutable(serializableComponent.m_ComponentName);
				if (!Assert(componentData != nullptr, std::format("Tried to deserialzie component from json:{} but failed to retrieve component at index:{}",
					JsonUtils::ToStringProperties(json), serializableComponent.m_ComponentName)))
					return nullptr;

				//SerializableComponent serializedComponent = json.get<SerializableComponent>();
				/*if (!Assert(SceneManager != nullptr, std::format("Tried to parse entity from serialized entity "
					"but parser does not contain valid scene manager")))
					return nullptr;*/

				//LogError("GOTTEN TO DYNAMIC CASY");
				return dynamic_cast<T*>(componentData);
			}
			catch (const std::exception& e)
			{
				Assert(false, std::format("Tried to deserialize component but ran into error:{}", e.what()));
				return nullptr;
			}
		};

	if (isOptional)
	{
		std::optional<T*> maybeComponent = TryDeserializeOptional<T*>(json,
			//We try to find entity based on its scene name (or whether it is global)
			[&deserializationAction](const Json& json)->std::optional<T*>
			{
				return deserializationAction(json);
			});

		if (!maybeComponent.has_value()) return nullptr;
		return maybeComponent.value();
	}

	T* tPtr = deserializationAction(json);
	if (!Assert(tPtr != nullptr, std::format("Tried to deserialize component from json:{} but could not get a "
		"NON NULL component pointer for NON OPTIONAL functional call", JsonUtils::ToStringProperties(json))))
		return nullptr;
	
	//LogError(std::format("Resulting comp: from deserializ:{}", std::to_string(tPtr!=nullptr)));
	return tPtr;
}

/// <summary>
/// Behaves similar to TryDeserializeComponent, but assumes the component is retrieved from the entity argument
/// bypassing needing to find the entity with scene manager. Note: this is for getting components 
/// of the same parent entity
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="json"></param>
/// <param name="entity"></param>
/// <param name="isOptional"></param>
/// <returns></returns>
template<typename T>
requires (!std::is_pointer_v<T>&& std::is_base_of_v<Component, T>)
T* TryDeserializeComponentSelf(const Json& json, EntityData& selfEntity, const bool& isOptional = false)
{
	std::function<T*(const Json&)> deserializationAction = [&selfEntity](const Json& json)->T*
		{
			try
			{
				SerializableComponent serializableComponent = json.get<SerializableComponent>();

				const std::string tType = Utils::GetTypeName<T>();
				const std::string jsonType = json.at("Component").get<std::string>();
				if (!Assert(tType == jsonType, std::format("Tried to get component from SELF of entity:{} "
					"but json type:{} does not match template:{}", selfEntity.ToString(), jsonType, tType)))
					return nullptr;

				return selfEntity.TryGetComponentMutable<T>();
			}
			catch (const std::exception& e)
			{
				Assert(false, std::format("Tried to deserialize component (SELF) but ran into error:{}", e.what()));
				return nullptr;
			}
		};

	if (isOptional)
	{
		std::optional<T*> maybeComponent = TryDeserializeOptional<T*>(json,
			//We try to find entity based on its scene name (or whether it is global)
			[&deserializationAction](const Json& json)->std::optional<T*>
			{
				return deserializationAction(json);
			});

		if (!maybeComponent.has_value()) return nullptr;
		return maybeComponent.value();
	}

	return deserializationAction(json);
}

template<typename T>
requires (!std::is_pointer_v<T> && std::is_base_of_v<Component, T>)
Json TrySerializeComponent(const T* component, const bool& isOptional = false)
{
	const Component* componentBase = static_cast<const Component*>(component);
	const EntityData& entity = componentBase->GetEntity();
	const std::string componentName = entity.TryGetComponentName(componentBase);

	if (isOptional)
	{
		return TrySerializeOptional<const T*>(component == nullptr ? std::nullopt : std::make_optional(component),
			[&entity, &componentName](const T* component)->Json
			{
				return SerializableComponent(entity.m_SceneName, entity.m_Name, componentName);
				//if (componentName == "PhysicsBodyData") Assert(false, std::format("Created serializvble comp:{}", serialized.ToString()));
			});
	}

	if (!Assert(component != nullptr, std::format("Tried to serialize NULL component for a NON OPTIONAL call")))
		return {};

	return SerializableComponent(entity.m_SceneName, entity.m_Name, componentName);
}

template<typename T>
requires (!std::is_pointer_v<T>&& std::is_base_of_v<Component, T>)
Json TrySerializeComponentSelf(const T* component, EntityData& selfEntity, const bool& isOptional = false)
{
	const std::string componentName = Utils::GetTypeName<T>();

	if (isOptional)
	{
		return TrySerializeOptional<const T*>(component == nullptr ? std::nullopt : std::make_optional(component),
			[&componentName](const T* component)->Json
			{
				return SerializableComponent(componentName);
				//if (componentName == "PhysicsBodyData") Assert(false, std::format("Created serializvble comp:{}", serialized.ToString()));
			});
	}

	if (!Assert(component != nullptr, std::format("Tried to serialize NULL component (SELF) for a NON OPTIONAL call")))
		return {};

	return SerializableComponent(componentName);
}

void from_json(const Json& json, SerializableField& serializableField);
void to_json(Json& json, const SerializableField& serializableField);

void from_json(const Json& json, ComponentFieldReference& fieldReference);
void to_json(Json& json, const ComponentFieldReference& fieldReference);

void from_json(const Json& json, AnimationPropertyVariant& var);
void to_json(Json& json, const AnimationPropertyVariant& var);

template<typename T>
void from_json(const Json& json, AnimationPropertyKeyframe<T>& var)
{
	const char* TIME_PROPERTY = "Time";
	const char* VALUE_PROPERTY = "Value";
	if (!HasRequiredProperties(json, { TIME_PROPERTY,  VALUE_PROPERTY }))
		return;

	try
	{
		float time = json.at(TIME_PROPERTY).get<float>();
		T value = json.at(VALUE_PROPERTY).get<T>();

		var = AnimationPropertyKeyframe<T>(value, time);
	}
	catch (const std::exception& e)
	{
		Assert(false, std::format("Tried to deserialize animtion property keyframe but ran into error:{}", e.what()));
	}
}
template<typename T>
void to_json(Json& json, const AnimationPropertyKeyframe<T>& var)
{
	json = { {"Time", var.GetTime()}, {"Value", var.GetValue()}};
}

template<typename T>
void from_json(const Json& json, AnimationProperty<T>& property)
{
	const char* KEYFRAMES_PROPERTY = "Keyframes";
	const char* FIELD_REF_PROPERTY = "FieldRef";
	if (!HasRequiredProperties(json, { KEYFRAMES_PROPERTY,  FIELD_REF_PROPERTY }))
		return;

	try
	{
		std::vector<AnimationPropertyKeyframe<T>> keyframes = json.at(KEYFRAMES_PROPERTY).get<std::vector<AnimationPropertyKeyframe<T>>>();
		property = AnimationProperty<T>(json.at(FIELD_REF_PROPERTY).get<ComponentFieldReference>(), keyframes);
	}
	catch (const std::exception& e)
	{
		Assert(false, std::format("Tried to deserialize animtion property but ran into error:{}", e.what()));
	}
}
template<typename T>
void to_json(Json& json, const AnimationProperty<T>& property)
{
	json = { {"Keyframes", property.m_Keyframes}, {"FieldRef", property.m_ComponentFieldRef}};
}

template<typename T>
std::optional<T> TryDeserializeOptional(const Json& json, 
	const std::function<std::optional<T>(const Json& json)> serializeFunc)
{
	try
	{
		if (json.is_string() && json.get<std::string>() == OPTIONAL_NULL_VALUE)
			return std::nullopt;

		if (serializeFunc != nullptr) return serializeFunc(json);
		return std::nullopt;
	}
	catch (const std::exception& e)
	{
		Assert(false, std::format("Tried to deserialize optional json property but ran into error:{}", e.what()));
		return std::nullopt;
	}
}

template<typename T>
Json TrySerializeOptional(const std::optional<T>& maybeT, 
	const std::function<Json(const T& value)> deserializeFunc)
{
	if (!maybeT.has_value()) return { OPTIONAL_NULL_VALUE };
	if (std::is_pointer<T>() && maybeT.has_value() && maybeT.value() == nullptr) 
		return { OPTIONAL_NULL_VALUE };

	if (deserializeFunc != nullptr && maybeT.has_value())
		return deserializeFunc(maybeT.value());
	return {};
}
