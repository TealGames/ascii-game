#include "pch.hpp"
#include "JsonSerializers.hpp"
#include "JsonConstants.hpp"
#include "Debug.hpp"
#include "SceneManager.hpp"

SceneManagement::SceneManager* SceneManager = nullptr;

void InitJsonSerializationDependencies(SceneManagement::SceneManager& manager)
{
	SceneManager = &manager;
}

bool HasRequiredProperties(const Json& json, const std::vector<std::string>& propertyNames)
{
	bool hasAllProperties = true;
	for (const auto& propertyName : propertyNames)
	{
		if (!Assert(json.contains(propertyName), std::format("Tried to parse json: '{}' to type "
			"but it is missing the property: '{}'", JsonUtils::ToStringProperties(json), propertyName)))
		{
			hasAllProperties= false;
		}
	}
	return hasAllProperties;
}

void from_json(const Json& json, Vec2& vec)
{
	const char* X_PROPERTY = "X";
	const char* Y_PROPERTY = "Y";
	if (!HasRequiredProperties(json, { X_PROPERTY,  Y_PROPERTY })) return;

	vec.m_X = json.at(X_PROPERTY).get<float>();
	vec.m_Y = json.at(Y_PROPERTY).get<float>();
}
void to_json(Json& json, const Vec2& vec)
{
	json = { {"X", vec.m_X}, {"Y", vec.m_Y} };
}

void from_json(const Json& json, Vec2Int& vec)
{
	const char* X_PROPERTY = "X";
	const char* Y_PROPERTY = "Y";
	if (!HasRequiredProperties(json, { X_PROPERTY,  Y_PROPERTY })) return;

	vec.m_X = json.at(X_PROPERTY).get<int>();
	vec.m_Y = json.at(Y_PROPERTY).get<int>();
}
void to_json(Json& json, const Vec2Int& vec)
{
	json = { {"X", vec.m_X}, {"Y", vec.m_Y} };
}

void from_json(const Json& json, FloatRange& range)
{
	const char* MIN_PROPERTY = "Min";
	const char* MAX_PROPERTY = "Max";
	if (!HasRequiredProperties(json, { MIN_PROPERTY,  MAX_PROPERTY })) return;

	range.m_Min = json.at(MIN_PROPERTY).get<float>();
	range.m_Max = json.at(MAX_PROPERTY).get<float>();
}
void to_json(Json& json, const FloatRange& range)
{
	json = { {"Min", range.m_Min}, {"Max", range.m_Max}};
}

void from_json(const Json& json, Array2DPosition& pos)
{
	pos = Array2DPosition(json.at("Row").get<int>(), json.at("Col").get<int>());
}
void to_json(Json& json, const Array2DPosition& pos)
{
	json = { {"Row", pos.GetRow()}, {"Col", pos.GetCol()}};
}

void from_json(const Json& json, RenderLayerType& layer)
{
	layer = GetLayersFromStrings(json.get<std::vector<std::string>>());
	//Assert(false, std::format("Layer updating to; {}", ToString(layer)));
}
void to_json(Json& json, const RenderLayerType& layer)
{
	json = GetLayersAsStrings(layer);
}

void from_json(const Json& json, Color& color)
{
	if (!HasRequiredProperties(json, { "R", "G", "B" })) return;

	std::optional<std::string> maybeStringProperty = JsonUtils::TryGet<std::string>(json);
	if (maybeStringProperty.has_value())
	{
		std::optional<Color> maybeConstantColor = JsonConstants::TryGetConstantColor(maybeStringProperty.value());
		if (!Assert(maybeConstantColor.has_value(), std::format("Tried to convert json:'{} to color using constant "
			"names but it matches no constants!'", JsonUtils::ToStringProperties(json))))
			return;

		color = maybeConstantColor.value();
		return;
	}
	color.r = json.at("R").get<std::uint8_t>();
	color.g = json.at("G").get<std::uint8_t>();
	color.b = json.at("B").get<std::uint8_t>();

	if (json.contains("A")) color.a = json.at("A").get<std::uint8_t>();
	else color.a = 255;
}
void to_json(Json& json, const Color& color)
{
	std::optional<std::string> maybeConstant = JsonConstants::TryGetColorConstant(color);
	if (maybeConstant.has_value())
	{
		json = { maybeConstant.value() };
		return;
	}

	json= { {"R", color.r}, {"G", color.g}, {"B", color.b}, {"A", color.a}};
}

void from_json(const Json& json, ColorGradientKeyFrame& gradientFrame)
{
	const char* COLOR_PROPERTY = "Color";
	const char* LOCATION_PROPERTY = "Location";
	if (!HasRequiredProperties(json, { COLOR_PROPERTY , LOCATION_PROPERTY })) 
		return;

	gradientFrame = ColorGradientKeyFrame(json.at(COLOR_PROPERTY).get<Color>(), 
										  json.at(LOCATION_PROPERTY).get<float>());
}
void to_json(Json& json, const ColorGradientKeyFrame& gradientFrame)
{
	json = { {"Color", gradientFrame.m_Color}, {"Location", gradientFrame.m_Location}};
}

void from_json(const Json& json, ColorGradient& gradient)
{
	gradient = ColorGradient(json.get<std::vector<ColorGradientKeyFrame>>());
}
void to_json(Json& json, const ColorGradient& gradient)
{
	json = {gradient.GetKeyframes()};
}

void from_json(const Json& json, TextChar& textChar)
{
	const char* COLOR_PROPERTY = "Color";
	const char* CHAR_PROPERTY = "Char";
	if (!HasRequiredProperties(json, { COLOR_PROPERTY, CHAR_PROPERTY })) 
		return;

	textChar = TextChar(json.at(COLOR_PROPERTY).get<Color>(), json.at(CHAR_PROPERTY).get<char>());
}
void to_json(Json& json, const TextChar& textChar)
{
	json = { {"Color", textChar.m_Char}, {"Char", textChar.m_Char}};
}

void from_json(const Json& json, TextCharPosition& textChar)
{
	const char* COLOR_PROPERTY = "Color";
	const char* CHAR_PROPERTY = "Char";
	const char* POS_PROPERTY = "Pos";
	if (!HasRequiredProperties(json, { COLOR_PROPERTY, CHAR_PROPERTY, POS_PROPERTY })) 
		return;

	textChar = TextCharPosition(json.at(POS_PROPERTY).get<Array2DPosition>(),
		TextChar(json.at(COLOR_PROPERTY).get<Color>(), json.at(CHAR_PROPERTY).get<std::string>()[0]));
}
void to_json(Json& json, const TextCharPosition& textChar)
{
	json = { {"Pos", textChar.m_RowColPos}, {"Char", textChar.m_Text.m_Char}, {"Color", textChar.m_Text.m_Color}};
}

std::optional<Font> TryDeserializeFont(const Json& json)
{
	std::optional<Font> maybeFont = JsonConstants::TryGetConstantFont(json.get<std::string>());
	if (!Assert(maybeFont.has_value(), std::format("Tried to convert json: {} to font "
		"but could not be deduced from its json value", JsonUtils::ToStringProperties(json))))
		return std::nullopt;

	return maybeFont;
}
Json TrySerializeFont(const Font& font)
{
	std::optional<std::string> maybeFontConstant = JsonConstants::TryGetFontConstant(font);
	if (!Assert(maybeFontConstant.has_value(), std::format("Tried to convert font to json but font constant"
		"could not be deduced from font")))
		return {};

	return maybeFontConstant.value();
}

void from_json(const Json& json, FontData& font)
{
	const char* FONT_PROEPRTY = "Font";
	const char* FONT_SIZE_PROPERTY = "FontSize";
	std::optional<Font> maybeFont = TryDeserializeFont(json.at(FONT_PROEPRTY).get<std::string>());

	float fontSize = 0;
	Json fontJson = json.at(FONT_SIZE_PROPERTY);
	if (fontJson.is_string())
	{
		//LogError("Reached font json string");
		std::optional<float> maybeFontSize = JsonConstants::TryGetConstantFontSize(fontJson.get<std::string>());
		if (!Assert(maybeFontSize.has_value(), std::format("Tried to convert json: {} to font data but font "
			"size could not be deduced from '{}' property", JsonUtils::ToStringProperties(json), FONT_SIZE_PROPERTY)))
			return;
		fontSize = maybeFontSize.value();
	}
	else fontSize = fontJson.get<float>();

	font = FontData(fontSize, maybeFont.value());
}
void to_json(Json& json, const FontData& font)
{
	json["Font"] = TrySerializeFont(font.m_Font);

	std::optional<std::string> maybeFontSizeConstant = JsonConstants::TryGetFontSizeConstant(font.m_FontSize);
	if (maybeFontSizeConstant.has_value()) json["FontSize"] = maybeFontSizeConstant.value();
	else json["FontSize"] = font.m_FontSize;
}

void from_json(const Json& json, TextBufferPosition& textChar)
{
	const char* TEXT_CHAR_PROPERTY = "Text";
	/*const char* FONT_PROEPRTY = "Font";
	const char* FONT_SIZE_PROPERTY = "FontSize";*/
	const char* POS_PROPERTY = "Pos";
	if (!HasRequiredProperties(json, { TEXT_CHAR_PROPERTY, POS_PROPERTY })) return;

	//std::optional<Font> maybeFont = TryDeserializeFont(json.at(FONT_PROEPRTY).get<std::string>());

	//float fontSize = 0;
	//Json fontJson = json.at(FONT_SIZE_PROPERTY);
	//if (fontJson.is_string())
	//{
	//	//LogError("Reached font json string");
	//	std::optional<float> maybeFontSize = JsonConstants::TryGetConstantFontSize(fontJson.get<std::string>());
	//	if (!Assert(maybeFontSize.has_value(), std::format("Tried to convert json: {} to text buffer position but font "
	//		"size could not be deduced from '{}' property", JsonUtils::ToStringProperties(json), FONT_SIZE_PROPERTY)))
	//		return;
	//	fontSize = maybeFontSize.value();
	//}
	//else fontSize = fontJson.get<float>();
	FontData data = json.get<FontData>();
	
	textChar = TextBufferPosition(json.at(POS_PROPERTY).get<Vec2>(),json.at(TEXT_CHAR_PROPERTY).get<TextChar>(), data);
}
void to_json(Json& json, const TextBufferPosition& textChar)
{
	/*json["Font"] = TrySerializeFont(textChar.m_FontData.m_Font);

	std::optional<std::string> maybeFontSizeConstant = JsonConstants::TryGetFontSizeConstant(textChar.m_FontData.m_FontSize);
	if (maybeFontSizeConstant.has_value()) json["FontSize"] = maybeFontSizeConstant.value();
	else json["FontSize"] = textChar.m_FontData.m_FontSize;*/
	Json fontDataJson = textChar.m_FontData;
	json.merge_patch(fontDataJson);

	json["Text"] = textChar.m_Text;
	json["Pos"] = textChar.m_Pos;
}

namespace Physics
{
	void from_json(const Json& json, Physics::AABB& aabb)
	{
		const char* SIZE_PROPERTY = "Size";
		if (!HasRequiredProperties(json, { SIZE_PROPERTY }))
			return;

		aabb = Physics::AABB(json.at(SIZE_PROPERTY).get<Vec2>());
	}
	void to_json(Json& json, const Physics::AABB& aabb)
	{
		json = { {"Size", aabb.GetSize()}};
	}
}

void from_json(const Json& json, VisualData& visualData)
{
	const char* BUFFER_PROPERTY = "Buffer";
	/*const char* FONT_PROEPRTY = "Font";
	const char* FONT_SIZE_PROPERTY = "FontSize";*/
	const char* PIVOT_PROPERTY = "Pivot";
	const char* CHAR_SPACING_PROPERTY = "CharSpacing";
	const char* CHAR_AREA_PROPERTY = "CharArea";
	if (!HasRequiredProperties(json, { BUFFER_PROPERTY, PIVOT_PROPERTY, CHAR_SPACING_PROPERTY })) return;

	auto textChars = json.at(BUFFER_PROPERTY).get<std::vector<std::vector<TextCharPosition>>>();

	//std::optional<Font> maybeFont = TryDeserializeFont(json.at(FONT_PROEPRTY).get<std::string>());

	//float fontSize = 0;
	//Json fontJson = json.at(FONT_SIZE_PROPERTY);
	//if (fontJson.is_string())
	//{
	//	//LogError("Reached font json string");
	//	std::optional<float> maybeFontSize = JsonConstants::TryGetConstantFontSize(fontJson.get<std::string>());
	//	if (!Assert(maybeFontSize.has_value(), std::format("Tried to convert json: {} to visual data but font "
	//		"size could not be deduced from '{}' property", JsonUtils::ToStringProperties(json), FONT_SIZE_PROPERTY)))
	//		return;
	//	fontSize = maybeFontSize.value();
	//}
	//else fontSize = fontJson.get<float>();
	FontData fontData = json.get<FontData>();

	Vec2 pivotPos = VisualData::DEFAULT_PIVOT;
	Json pivotJson = json.at(PIVOT_PROPERTY);
	if (pivotJson.is_string())
	{
		std::optional<Vec2> maybePivot = JsonConstants::TryGetConstantPivot(pivotJson.get<std::string>());
		if (!Assert(maybePivot.has_value(), std::format("Tried to convert json: {} to visual data but pivot "
			"could not be deduced from '{}' property", JsonUtils::ToStringProperties(json), PIVOT_PROPERTY)))
			return;
		pivotPos = maybePivot.value();
	}
	else pivotPos = pivotJson.get<Vec2>();

	visualData = VisualData(textChars, fontData.m_Font, fontData.m_FontSize, json.at(CHAR_SPACING_PROPERTY).get<Vec2>(), NormalizedPosition(pivotPos));
	if (json.contains(CHAR_AREA_PROPERTY))
	{
		visualData.SetPredefinedCharArea(json.at(CHAR_AREA_PROPERTY).get<Vec2>());
	}
}
void to_json(Json& json, const VisualData& visualData)
{
	/*json["Font"] = TrySerializeFont(visualData.GetFont());
	std::optional<std::string> maybeFontSizeConstant = JsonConstants::TryGetFontSizeConstant(visualData.GetFontSize());
	if (maybeFontSizeConstant.has_value()) json["FontSize"] = maybeFontSizeConstant.value();
	else json["FontSize"] = visualData.GetFontSize();*/
	Json fontDataJson = visualData.GetFontData();
	json.merge_patch(fontDataJson);

	json["Buffer"] = visualData.GetRawBuffer();

	std::optional<std::string> maybePivotConstant = JsonConstants::TryGetPivotConstant(visualData.GetPivot());
	if (maybePivotConstant.has_value()) json["Pivot"] = maybePivotConstant.value();
	else json["Pivot"] = visualData.GetPivot();

	json["CharSpacing"] = visualData.GetCharSpacing();
	if (visualData.HasPredefinedCharArea()) json["CharArea"] = visualData.GetPredefinedCharArea();
}

void from_json(const Json& json, SpriteAnimationDelta& delta)
{
	const char* TIME_PROPERTY = "Time";
	const char* DELTA_PROPERTY = "Delta";
	if (!HasRequiredProperties(json, { TIME_PROPERTY, DELTA_PROPERTY })) 
		return;

	delta = SpriteAnimationDelta(json.at(TIME_PROPERTY).get<float>(), json.at(DELTA_PROPERTY).get<VisualDataPositions>());
}
void to_json(Json& json, const SpriteAnimationDelta& delta)
{
	json = { {"Time", delta.m_Time}, {"Delta", delta.m_VisualDelta}};
}

void from_json(const Json& json, SerializableEntity& serializableEntity)
{
	serializableEntity.m_EntityName = json.at("Entity").get<std::string>();
	serializableEntity.m_SceneName = json.at("Scene").get<std::string>();
}
void to_json(Json& json, const SerializableEntity& serializableEntity)
{
	json = { {"Entity", serializableEntity.m_EntityName}, {"Scene", serializableEntity.m_SceneName} };
}

ECS::Entity* TryDeserializeEntity(const Json& json, const bool& isOptional)
{
	std::function<ECS::Entity*(const Json&)> deserializationAction = [](const Json& json)-> ECS::Entity*
		{
			SerializableEntity serializedEntity = json.get<SerializableEntity>();
			if (!Assert(SceneManager != nullptr, std::format("Tried to parse entity from serialized entity "
				"but parser does not contain valid scene manager")))
				return nullptr;

			if (serializedEntity.m_SceneName == ECS::Entity::GLOBAL_SCENE_NAME)
			{
				return SceneManager->m_GlobalEntityManager.TryGetGlobalEntityMutable(serializedEntity.m_EntityName);
			}

			Scene* maybeScene = SceneManager->TryGetSceneMutable(serializedEntity.m_SceneName);
			if (!Assert(maybeScene != nullptr, std::format("Tried to deserialize entity with non global scene: '{}', "
				"but no scene matches that name", serializedEntity.m_SceneName)))
				return nullptr;

			ECS::Entity* maybeEntity = maybeScene->TryGetEntityMutable(serializedEntity.m_EntityName);
			if (maybeEntity == nullptr)
			{
				if (!Assert(maybeScene->GetEntityCount() > 0, std::format("Tried to deserialize entity with non glboal scene:'{}', "
					"but no entities exist in that scene. It could be because that scene was no loaded yet "
					"(and another scene tried to create a reference to an entity)", serializedEntity.m_SceneName)))
					return nullptr;

				Assert(false, std::format("Tried to deserialize entity with non glboal scene:'{}', "
					"but no entities with that name exist!", serializedEntity.m_SceneName));
				return nullptr;
			}

			return maybeEntity;
		};

	if (isOptional)
	{
		std::optional<ECS::Entity*> maybeEntity = TryDeserializeOptional<ECS::Entity*>(json,
			//We try to find entity based on its scene name (or whether it is global)
			[&deserializationAction](const Json& json)-> std::optional<ECS::Entity*>
			{
				return deserializationAction(json);
			});
		/*Assert(false, std::format("Deserialized entity jsoN:{} to has value:{} entity value:{}", JsonUtils::ToStringProperties(json), std::to_string(maybeEntity.has_value()), 
			maybeEntity.has_value()? (maybeEntity.value()!=nullptr? maybeEntity.value()->ToString() : "NULL") : "NO VALUE"));*/

		if (!maybeEntity.has_value() || (maybeEntity.has_value() && maybeEntity.value() == nullptr))
			return nullptr;

		//LogError(std::format("Deserialized json: {} to entity: {}", JsonUtils::ToStringProperties(json), maybeEntity.value()->ToString()));
		return maybeEntity.value();
	}

	ECS::Entity* entityPtr = deserializationAction(json);
	if (!Assert(entityPtr != nullptr, std::format("Tried to deserialize entity for json:{} "
		"but the resulting entity is NULLPTR which is not allowed since it was called as NON OPTIONAL",
		JsonUtils::ToStringProperties(json))))
		return nullptr;

	return entityPtr;
}
Json TrySerializeEntity(const ECS::Entity* entity, const bool& isOptional)
{
	if (isOptional)
	{
		Json json= TrySerializeOptional<const ECS::Entity*>(entity == nullptr ? std::nullopt : std::make_optional(entity),
			[](const ECS::Entity* entity)->Json
			{
				SerializableEntity serializedEntity = { entity->GetName(), entity->GetSceneName() };
				return { serializedEntity };
			});

		//Assert(false, std::format("Serialized optioan entity: {} is:{}", entity!=nullptr? entity->ToString() : "NULL", JsonUtils::ToStringProperties(json)));
		return json;
	}

	if (!Assert(entity != nullptr, std::format("Tried to serialize entity to json but entity is "
		"NULL even with a NON OPTIOANL functional call")))
		return {};

	return SerializableEntity{entity->GetName(), entity->GetSceneName()};
}

void from_json(const Json& json, SerializableComponent& serializableComponent)
{
	const char* SCENE_PROPERTY = "Scene";
	const char* ENTITY_PROPERTY = "Entity";
	const char* COMPONENT_NAME_PROPERTY = "Component";
	if (!HasRequiredProperties(json, { SCENE_PROPERTY, ENTITY_PROPERTY, COMPONENT_NAME_PROPERTY }))
		return;

	serializableComponent = SerializableComponent(json.at(SCENE_PROPERTY).get<std::string>(), json.at(ENTITY_PROPERTY).get<std::string>(),
		json.at(COMPONENT_NAME_PROPERTY).get<std::string>());
}
void to_json(Json& json, const SerializableComponent& serializableComponent)
{
	json = { {"Scene", serializableComponent.m_SerializedEntity.m_SceneName},
			 {"Entity", serializableComponent.m_SerializedEntity.m_SceneName},
			 {"Component", serializableComponent.m_ComponentName} 
		   };
}

void from_json(const Json& json, ComponentReference& fieldReference)
{
	SerializableComponent field = json.get<SerializableComponent>();
	ECS::Entity* maybeEntity = SceneManager->TryGetEntityMutable(field.m_SerializedEntity.m_SceneName, 
																 field.m_SerializedEntity.m_EntityName);
	if (maybeEntity == nullptr) return;

	fieldReference = ComponentReference(*maybeEntity, field.m_ComponentName);
}
void to_json(Json& json, const ComponentReference& fieldReference)
{
	const ECS::Entity& entity = fieldReference.GetEntitySafe();
	SerializableComponent component = SerializableComponent(entity.GetSceneName(), entity.GetName(),
		fieldReference.GetComponentName());
	json = { component };
}

void from_json(const Json& json, SerializableField& serializableField)
{
	const char* SCENE_PROPERTY = "Scene";
	const char* ENTITY_PROPERTY = "Entity";
	const char* COMPONENT_NAME_PROPERTY = "Component";
	const char* FIELD_PROPERTY = "Field";
	if (!HasRequiredProperties(json, { SCENE_PROPERTY, ENTITY_PROPERTY, COMPONENT_NAME_PROPERTY, FIELD_PROPERTY }))
		return;

	serializableField = SerializableField(json.at(SCENE_PROPERTY).get<std::string>(), json.at(ENTITY_PROPERTY).get<std::string>(),
						json.at(COMPONENT_NAME_PROPERTY).get<std::string>(), json.at(FIELD_PROPERTY).get<std::string>());
}
void to_json(Json& json, const SerializableField& serializableField)
{
	json = { {"Scene", serializableField.m_SerializedComponent.m_SerializedEntity.m_SceneName}, 
			 {"Entity", serializableField.m_SerializedComponent.m_SerializedEntity.m_SceneName},
			 {"Component", serializableField.m_SerializedComponent.m_ComponentName}, 
			 {"Field", serializableField.m_FieldName}};
}

void from_json(const Json& json, ComponentFieldReference& fieldReference)
{
	SerializableField field = json.get<SerializableField>();
	ECS::Entity* maybeEntity = SceneManager->TryGetEntityMutable(field.m_SerializedComponent.m_SerializedEntity.m_SceneName, 
																 field.m_SerializedComponent.m_SerializedEntity.m_EntityName);
	if (maybeEntity == nullptr) return;

	fieldReference = ComponentFieldReference(*maybeEntity, field.m_SerializedComponent.m_ComponentName, field.m_FieldName);
}	
void to_json(Json& json, const ComponentFieldReference& fieldReference)
{
	const ECS::Entity& entity = fieldReference.GetEntitySafe();
	SerializableField field = SerializableField(entity.GetSceneName(), entity.GetName(), 
		fieldReference.m_ComponentRef.GetComponentName(), fieldReference.GetFieldName());
	json = { field };
}

void from_json(const Json& json, AnimationPropertyVariant& variant)
{
	const char* TYPE_PROPERTY = "Type";
	const char* PROPERTY_PROPERTY = "Property";
	if (!HasRequiredProperties(json, { TYPE_PROPERTY,  PROPERTY_PROPERTY }))
		return;

	std::string propertyType = json.at(TYPE_PROPERTY).get<std::string>();
	if (propertyType == Utils::GetTypeName<int>())
	{
		variant = AnimationPropertyVariant(json.at(PROPERTY_PROPERTY).get<AnimationProperty<int>>());
	}
	else if (propertyType == Utils::GetTypeName<float>())
	{
		variant = json.at(PROPERTY_PROPERTY).get<AnimationProperty<float>>();
	}
	else if (propertyType == Utils::GetTypeName<std::uint8_t>())
	{
		variant = json.at(PROPERTY_PROPERTY).get<AnimationProperty<std::uint8_t>>();
	}
	else
	{
		Assert(false, std::format("Tried to deserialize json:{} to animtion property variant, "
			"but could not find actions for type:{}", JsonUtils::ToStringProperties(json), propertyType));
	}
}
void to_json(Json& json, const AnimationPropertyVariant& var)
{
	if (std::holds_alternative<AnimationProperty<int>>(var))
	{
		json = { {"Type", Utils::GetTypeName<int>() }, 
			{"Property", std::any_cast<AnimationProperty<int>>(var)}};
	}
	else if (std::holds_alternative<AnimationProperty<float>>(var))
	{
		json = { {"Type", Utils::GetTypeName<float>() },
			   {"Property", std::any_cast<AnimationProperty<float>>(var)} };

	}
	else if (std::holds_alternative<AnimationProperty<std::uint8_t>>(var))
	{
		json = { {"Type", Utils::GetTypeName<std::uint8_t>() },
			{"Property", std::any_cast<AnimationProperty<std::uint8_t>>(var)} };
	}
	else
	{
		Assert(false, std::format("Tried to serialize animation property variatn to json, "
			"but could not find actions for its type"));
	}
}