#pragma once
#include "nlohmann/json.hpp"
#include "Vec2.hpp"
#include "Vec2Int.hpp"
#include "RenderLayer.hpp"
#include "VisualData.hpp"
#include "TextArray.hpp"
#include "SerializableEntity.hpp"
#include "Entity.hpp"
#include "Scene.hpp"
#include "AnimatorData.hpp"
#include "ComponentFieldReference.hpp"
#include "SerializableField.hpp"
#include <functional>
#include <type_traits>
#include <cstdint>
#include <vector>

using Json = nlohmann::json;
namespace SceneManagement
{
	class SceneManager;
}

inline const char* OPTIONAL_NULL_VALUE = "null";

void InitJsonSerializers(SceneManagement::SceneManager& manager);

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

void from_json(const Json& json, TextChar& textChar);
void to_json(Json& json, const TextChar& textChar);

void from_json(const Json& json, TextCharPosition& textChar);
void to_json(Json& json, const TextCharPosition& textChar);

void from_json(const Json& json, VisualData& visualData);
void to_json(Json& json, const VisualData& visualData);

void from_json(const Json& json, SerializableEntity& serializableEntity);
void to_json(Json& json, const SerializableEntity& serializableEntity);

ECS::Entity* TryDeserializeEntity(const Json& json);
Json TrySerializeEntity(const ECS::Entity* entity);

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
