#pragma once
#include "nlohmann/json.hpp"
#include "Vec2.hpp"
#include "Vec2Int.hpp"
#include "RenderLayer.hpp"
#include "VisualData.hpp"
#include "TextArray.hpp"

using Json = nlohmann::json;

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
