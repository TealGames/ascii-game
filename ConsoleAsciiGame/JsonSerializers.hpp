#pragma once
#include "nlohmann/json.hpp"
#include "Vec2.hpp"
#include "Vec2Int.hpp"
#include "RenderLayer.hpp"
#include "VisualData.hpp"
#include "TextArray.hpp"
#include "SerializableEntity.hpp"
#include "ComponentData.hpp"
#include "Entity.hpp"
#include "Scene.hpp"
#include "AnimatorData.hpp"
#include "SpriteAnimatorData.hpp"
#include "ComponentFieldReference.hpp"
#include "SerializableField.hpp"
#include "ColorGradient.hpp"
#include "JsonUtils.hpp"
#include "AABB.hpp"
#include <functional>
#include <type_traits>
#include <cstdint>
#include <vector>

using Json = nlohmann::json;
using JsonOrdered = nlohmann::ordered_json;
namespace SceneManagement
{
	class SceneManager;
}

inline const char* OPTIONAL_NULL_VALUE = "null";
extern SceneManagement::SceneManager* SceneManager;

void InitJsonSerializationDependencies(SceneManagement::SceneManager& manager);

bool HasRequiredProperties(const Json& json, const std::vector<std::string>& propertyNames);

void from_json(const Json& json, Vec2& vec);
void to_json(Json& json, const Vec2& vec);

void from_json(const Json& json, Vec2Int& vec);
void to_json(Json& json, const Vec2Int& vec);

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

void from_json(const Json& json, TextCharPosition& textChar);
void to_json(Json& json, const TextCharPosition& textChar);

void from_json(const Json& json, TextBufferPosition& textChar);
void to_json(Json& json, const TextBufferPosition& textChar);

void from_json(const Json& json, VisualData& visualData);
void to_json(Json& json, const VisualData& visualData);

void from_json(const Json& json, SpriteAnimationDelta& delta);
void to_json(Json& json, const SpriteAnimationDelta& delta);

namespace Physics
{
	void from_json(const Json& json, Physics::AABB& aabb);
	void to_json(Json& json, const Physics::AABB& aabb);
}

void from_json(const Json& json, SerializableEntity& serializableEntity);
void to_json(Json& json, const SerializableEntity& serializableEntity);

ECS::Entity* TryDeserializeEntity(const Json& json, const bool& isOptional = false);
Json TrySerializeEntity(const ECS::Entity* entity, const bool& isOptional = false);

void from_json(const Json& json, SerializableComponent& serializableComponent);
void to_json(Json& json, const SerializableComponent& serializableComponent);

void from_json(const Json& json, ComponentReference& fieldReference);
void to_json(Json& json, const ComponentReference& fieldReference);

template<typename T>
requires (!std::is_pointer_v<T> && ECS::IsComponent<T>)
T* TryDeserializeComponent(const Json& json, const bool& isOptional = false)
{
	std::function<T* (const Json&)> deserializationAction = [](const Json& json)-> T*
		{
			SerializableComponent serializableComponent = json.get<SerializableComponent>();
			ECS::Entity* entity = TryDeserializeEntity(json);
			if (!Assert(entity != nullptr, std::format("Tried to deserialize component from json:{} but failed to retrieve entity", 
				JsonUtils::ToStringProperties(json))))
				return nullptr;

			ComponentData* componentData = entity->TryGetComponentWithNameMutable(serializableComponent.m_ComponentName);
			if (!Assert(componentData != nullptr, std::format("Tried to deserialzie component from json:{} but failed to retrieve component at index:{}",
				JsonUtils::ToStringProperties(json), serializableComponent.m_ComponentName)))
				return nullptr;

			//SerializableComponent serializedComponent = json.get<SerializableComponent>();
			if (!Assert(SceneManager != nullptr, std::format("Tried to parse entity from serialized entity "
				"but parser does not contain valid scene manager")))
				return nullptr;

			try
			{
				return dynamic_cast<T*>(componentData);
			}
			catch (const std::exception& e)
			{
				return nullptr; 
			}
		};

	if (isOptional)
	{
		std::optional<T*> maybeEntity = TryDeserializeOptional<T*>(json,
			//We try to find entity based on its scene name (or whether it is global)
			[&deserializationAction](const Json& json)->std::optional<T*>
			{
				return deserializationAction(json);
			});

		if (maybeEntity.has_value() || (maybeEntity.has_value() && maybeEntity.value() == nullptr))
		{
			return nullptr;
		}
	}

	T* tPtr = deserializationAction(json);
	if (!Assert(tPtr != nullptr, std::format("Tried to deserialize component from json:{} but could not get a "
		"NON NULL component pointer for NON OPTIONAL functional call", JsonUtils::ToStringProperties(json))))
		return nullptr;
	
	return tPtr;
}

template<typename T>
requires (!std::is_pointer_v<T> && ECS::IsComponent<T>)
Json TrySerializeComponent(const T* component, const bool& isOptional = false)
{
	const ComponentData* componentBase = static_cast<const ComponentData*>(component);
	const ECS::Entity& entity = componentBase->GetEntitySafe();
	const std::string componentName = entity.TryGetComponentName(componentBase);

	if (isOptional)
	{
		return TrySerializeOptional<const T*>(component == nullptr ? std::nullopt : std::make_optional(component),
			[&entity, &componentName](const T* component)->Json
			{
				SerializableComponent serialized = SerializableComponent(entity.GetSceneName(), entity.GetName(), componentName);
				return serialized;
			});
	}

	if (!Assert(component != nullptr, std::format("Tried to serialie NULL component for a NON OPTIONAL call")))
		return {};

	return SerializableComponent(entity.GetSceneName(), entity.GetName(), componentName);
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
	float time = json.at("Time").get<float>();
	T value = json.at("Value").get<T>();

	var = AnimationPropertyKeyframe<T>(value, time);
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

	std::vector<AnimationPropertyKeyframe<T>> keyframes = json.at(KEYFRAMES_PROPERTY).get<std::vector<AnimationPropertyKeyframe<T>>>();
	property = AnimationProperty<T>(json.at(FIELD_REF_PROPERTY).get<ComponentFieldReference>(), keyframes);
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
	if (json.is_string() && json.get<std::string>() == OPTIONAL_NULL_VALUE) 
		return std::nullopt;

	if (serializeFunc != nullptr) return serializeFunc(json);
	return std::nullopt;
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
