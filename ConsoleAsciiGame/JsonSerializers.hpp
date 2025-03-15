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
#include <functional>
#include <type_traits>

using Json = nlohmann::json;
namespace SceneManagement
{
	class SceneManager;
}

inline const char* OPTIONAL_NULL_VALUE = "null";

void InitJsonSerializers(SceneManagement::SceneManager& manager);

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
Json TrySerializeEntity(const ECS::Entity* entity, const Scene* entityScene);

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
