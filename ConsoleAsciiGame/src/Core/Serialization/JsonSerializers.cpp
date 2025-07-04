#include "pch.hpp"
#include "Core/Serialization/JsonSerializers.hpp"
#include "Core/Serialization/JsonConstants.hpp"
#include "Core/Analyzation/Debug.hpp"
#include "Core/Scene/SceneManager.hpp"
#include "Core/Asset/AssetManager.hpp"
#include "Core/Asset/FontAsset.hpp"

SceneManagement::SceneManager* SceneManager = nullptr;
AssetManagement::AssetManager* AssetManager = nullptr;
//static constexpr std::uint8_t SERIALIZATION_DECIMAL_COUNT = 3;

void InitJsonSerializationDependencies(SceneManagement::SceneManager& manager, AssetManagement::AssetManager& assetManager)
{
	SceneManager = &manager;
	AssetManager = &assetManager;
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

	try
	{
		vec.m_X = json.at(X_PROPERTY).get<float>();
		vec.m_Y = json.at(Y_PROPERTY).get<float>();
	}
	catch (const std::exception& e)
	{
		Assert(false, std::format("Tried to deserialize vec2:{} but ran into error:{}", JsonUtils::ToStringProperties(json), e.what()));
	}
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

	try
	{
		vec.m_X = json.at(X_PROPERTY).get<int>();
		vec.m_Y = json.at(Y_PROPERTY).get<int>();
	}
	catch (const std::exception& e)
	{
		Assert(false, std::format("Tried to deserialize vec2int:{} but ran into error:{}", JsonUtils::ToStringProperties(json), e.what()));
	}
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

	try
	{
		range.m_Min = json.at(MIN_PROPERTY).get<float>();
		range.m_Max = json.at(MAX_PROPERTY).get<float>();
	}
	catch (const std::exception& e)
	{
		Assert(false, std::format("Tried to deserialize floatrange:{} but ran into error:{}", JsonUtils::ToStringProperties(json), e.what()));
	}
}
void to_json(Json& json, const FloatRange& range)
{
	json = { {"Min", range.m_Min}, {"Max", range.m_Max}};
}

void from_json(const Json& json, Array2DPosition& pos)
{
	const char* ROW_PROPERTY = "Row";
	const char* COL_PROPERTY = "Col";
	if (!HasRequiredProperties(json, { ROW_PROPERTY,  COL_PROPERTY })) return;

	try
	{
		pos = Array2DPosition(json.at(ROW_PROPERTY).get<int>(), json.at(COL_PROPERTY).get<int>());
	}
	catch (const std::exception& e)
	{
		Assert(false, std::format("Tried to deserialize array2dpos:{} but ran into error:{}", JsonUtils::ToStringProperties(json), e.what()));
	}
}
void to_json(Json& json, const Array2DPosition& pos)
{
	json = { {"Row", pos.GetRow()}, {"Col", pos.GetCol()}};
}

void from_json(const Json& json, RenderLayerType& layer)
{
	try
	{
		layer = GetLayersFromStrings(json.get<std::vector<std::string>>());
	}
	catch (const std::exception& e)
	{
		Assert(false, std::format("Tried to deserialize renderlayertype:{} but ran into error:{}", JsonUtils::ToStringProperties(json), e.what()));
	}
	//Assert(false, std::format("Layer updating to; {}", ToString(layer)));
}
void to_json(Json& json, const RenderLayerType& layer)
{
	json = GetLayersAsStrings(layer);
}

void from_json(const Json& json, Color& color)
{
	try
	{
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

		LogError(std::format("is str:{}", std::to_string(json.is_string())));
		if (!HasRequiredProperties(json, { "R", "G", "B" }))
		{
			
			return;
		}
		color.r = json.at("R").get<std::uint8_t>();
		color.g = json.at("G").get<std::uint8_t>();
		color.b = json.at("B").get<std::uint8_t>();

		if (json.contains("A")) color.a = json.at("A").get<std::uint8_t>();
		else color.a = 255;
	}
	catch (const std::exception& e)
	{
		Assert(false, std::format("Tried to deserialize color:{} but ran into error:{}", JsonUtils::ToStringProperties(json), e.what()));
	}
}

	
void to_json(Json& json, const Color& color)
{
	std::optional<std::string> maybeConstant = JsonConstants::TryGetColorConstant(color);
	if (maybeConstant.has_value())
	{
		json = maybeConstant.value();
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

	try
	{
		gradientFrame = ColorGradientKeyFrame(json.at(COLOR_PROPERTY).get<Color>(),
			json.at(LOCATION_PROPERTY).get<float>());
	}
	catch (const std::exception& e)
	{
		Assert(false, std::format("Tried to deserialize colorgradient keyframe:{} but ran into error:{}", JsonUtils::ToStringProperties(json), e.what()));
	}
}
void to_json(Json& json, const ColorGradientKeyFrame& gradientFrame)
{
	json = { {"Color", gradientFrame.m_Color}, {"Location", gradientFrame.m_Location}};
}

void from_json(const Json& json, ColorGradient& gradient)
{
	try
	{
		gradient = ColorGradient(json.get<std::vector<ColorGradientKeyFrame>>());
	}
	catch (const std::exception& e)
	{
		Assert(false, std::format("Tried to deserialize colorgradient:{} but ran into error:{}", JsonUtils::ToStringProperties(json), e.what()));
	}
}
void to_json(Json& json, const ColorGradient& gradient)
{
	json = gradient.GetKeyframes();
}

void from_json(const Json& json, TextChar& textChar)
{
	const char* COLOR_PROPERTY = "Color";
	const char* CHAR_PROPERTY = "Char";
	if (!HasRequiredProperties(json, { COLOR_PROPERTY, CHAR_PROPERTY })) 
		return;

	try
	{
		textChar = TextChar(json.at(COLOR_PROPERTY).get<Color>(), json.at(CHAR_PROPERTY).get<char>());
	}
	catch (const std::exception& e)
	{
		Assert(false, std::format("Tried to deserialize text char:{} but ran into error:{}", JsonUtils::ToStringProperties(json), e.what()));
	}
}
void to_json(Json& json, const TextChar& textChar)
{
	json = { {"Color", textChar.m_Color}, {"Char", textChar.GetChar()}};
}

void from_json(const Json& json, TextCharArrayPosition& textChar)
{
	const char* COLOR_PROPERTY = "Color";
	const char* CHAR_PROPERTY = "Char";
	const char* POS_PROPERTY = "Pos";
	if (!HasRequiredProperties(json, { COLOR_PROPERTY, CHAR_PROPERTY, POS_PROPERTY })) 
		return;

	try
	{
		textChar = TextCharArrayPosition(json.at(POS_PROPERTY).get<Array2DPosition>(),
			TextChar(json.at(COLOR_PROPERTY).get<Color>(), json.at(CHAR_PROPERTY).get<std::string>()[0]));
	}
	catch (const std::exception& e)
	{
		Assert(false, std::format("Tried to deserialize textcharpos:{} but ran into error:{}", JsonUtils::ToStringProperties(json), e.what()));
	}
}
void to_json(Json& json, const TextCharArrayPosition& textChar)
{
	json = { {"Pos", textChar.m_RowColPos}, {"Char", std::string(1, textChar.m_Text.GetChar())}, {"Color", textChar.m_Text.m_Color}};
}

/*
std::optional<Font> TryDeserializeFont(const Json& json)
{
	try
	{
		std::optional<Font> maybeFont = JsonConstants::TryGetConstantFont(json.get<std::string>());
		if (!Assert(maybeFont.has_value(), std::format("Tried to convert json: {} to font "
			"but could not be deduced from its json value", JsonUtils::ToStringProperties(json))))
			return std::nullopt;

		return maybeFont;
	}
	catch (const std::exception& e)
	{
		Assert(false, std::format("Tried to deserialize font:{} but ran into error:{}", JsonUtils::ToStringProperties(json), e.what()));
		return std::nullopt;
	}
}
Json TrySerializeFont(const Font& font)
{
	std::optional<std::string> maybeFontConstant = JsonConstants::TryGetFontConstant(font);
	if (!Assert(maybeFontConstant.has_value(), std::format("Tried to convert font to json but font constant"
		"could not be deduced from font")))
		return {};

	return maybeFontConstant.value();
}
*/

void from_json(const Json& json, FontProperties& font)
{
	const char* FONT_PROEPRTY = "Font";
	const char* FONT_SIZE_PROPERTY = "FontSize";
	const char* TRACKING_PROPERTY = "Tracking";
	FontAsset* fontAsset = TryDeserializeTypeAsset<FontAsset>(json.at(FONT_PROEPRTY));

	try
	{
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

		font = FontProperties(fontSize, json.at(TRACKING_PROPERTY).get<float>(), *fontAsset);
	}
	catch (const std::exception& e)
	{
		Assert(false, std::format("Tried to deserialize fontdata:{} but ran into error:{}", JsonUtils::ToStringProperties(json), e.what()));
	}
}
	
void to_json(Json& json, const FontProperties& font)
{
	json["Font"] = TrySerializeAsset(font.m_FontAsset);

	std::optional<std::string> maybeFontSizeConstant = JsonConstants::TryGetFontSizeConstant(font.m_Size);
	if (maybeFontSizeConstant.has_value()) json["FontSize"] = maybeFontSizeConstant.value();
	else json["FontSize"] = font.m_Size;
}

void from_json(const Json& json, TextBufferCharPosition& textChar)
{
	const char* TEXT_CHAR_PROPERTY = "Text";
	const char* FONT_PROEPRTY = "Font";
	const char* POS_PROPERTY = "Pos";
	if (!HasRequiredProperties(json, { TEXT_CHAR_PROPERTY, POS_PROPERTY, FONT_PROEPRTY })) return;

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
	try
	{
		textChar = TextBufferCharPosition(json.at(POS_PROPERTY).get<Vec2>(), 
			json.at(TEXT_CHAR_PROPERTY).get<TextChar>(), json.at(FONT_PROEPRTY).get<FontProperties>());
	}
	catch (const std::exception& e)
	{
		Assert(false, std::format("Tried to deserialize text buffer pos:{} but ran into error:{}", JsonUtils::ToStringProperties(json), e.what()));
	}
}
void to_json(Json& json, const TextBufferCharPosition& textChar)
{
	/*json["Font"] = TrySerializeFont(textChar.m_FontData.m_Font);

	std::optional<std::string> maybeFontSizeConstant = JsonConstants::TryGetFontSizeConstant(textChar.m_FontData.m_FontSize);
	if (maybeFontSizeConstant.has_value()) json["FontSize"] = maybeFontSizeConstant.value();
	else json["FontSize"] = textChar.m_FontData.m_FontSize;*/
	json["Text"] = textChar.m_Text;
	json["Pos"] = textChar.m_Pos;
	json["Font"] = textChar.m_FontData;
}

namespace Physics
{
	void from_json(const Json& json, Physics::AABB& aabb)
	{
		const char* SIZE_PROPERTY = "Size";
		if (!HasRequiredProperties(json, { SIZE_PROPERTY }))
			return;

		try
		{
			aabb = Physics::AABB(json.at(SIZE_PROPERTY).get<Vec2>());
		}
		catch (const std::exception& e)
		{
			Assert(false, std::format("Tried to deserialize aabb:{} but ran into error:{}", JsonUtils::ToStringProperties(json), e.what()));
		}
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
	if (!HasRequiredProperties(json, { BUFFER_PROPERTY, PIVOT_PROPERTY})) 
		return;

	try
	{
		auto textChars = json.at(BUFFER_PROPERTY).get<std::vector<TextBufferCharPosition>>();

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

		visualData = VisualData(textChars, NormalizedPosition(pivotPos));
	}
	catch (const std::exception& e)
	{
		Assert(false, std::format("Tried to deserialize visualdata:{} but ran into error:{}", JsonUtils::ToStringProperties(json), e.what()));
	}
}
void to_json(Json& json, const VisualData& visualData)
{
	/*json["Font"] = TrySerializeFont(visualData.GetFont());
	std::optional<std::string> maybeFontSizeConstant = JsonConstants::TryGetFontSizeConstant(visualData.GetFontSize());
	if (maybeFontSizeConstant.has_value()) json["FontSize"] = maybeFontSizeConstant.value();
	else json["FontSize"] = visualData.GetFontSize();*/
	json["Buffer"] = visualData.GetBuffer();

	std::optional<std::string> maybePivotConstant = JsonConstants::TryGetPivotConstant(visualData.GetPivot());
	if (maybePivotConstant.has_value()) json["Pivot"] = maybePivotConstant.value();
	else json["Pivot"] = visualData.GetPivot();
}

void from_json(const Json& json, SpriteAnimationFrame& frame)
{
	const char* TIME_PROPERTY = "Time";
	const char* VISUAL_PROPERTY = "Visual";
	if (!HasRequiredProperties(json, { TIME_PROPERTY, VISUAL_PROPERTY }))
		return;

	try
	{
		frame = SpriteAnimationFrame(json.at(TIME_PROPERTY).get<float>(), json.at(VISUAL_PROPERTY).get<VisualData>());
	}
	catch (const std::exception& e)
	{
		Assert(false, std::format("Tried to deserialize sprite animation frame:{} but ran into error:{}",
			JsonUtils::ToStringProperties(json), e.what()));
	}
}
void to_json(Json& json, const SpriteAnimationFrame& frame)
{
	json= { {"Time", frame.m_Time}, {"Visual", frame.m_VisualFrame} };
}

void from_json(const Json& json, SpriteAnimation& anim)
{
	const char* NAME_PROPERTY = "Name";
	const char* LOOP_PROPERTY = "Loop";
	const char* SPEED_PROPERTY = "Speed";
	const char* LENGTH_PROPERTY = "Length";
	const char* VISUALS_PROPERTY = "Visuals";
	if (!HasRequiredProperties(json, { NAME_PROPERTY, LOOP_PROPERTY, SPEED_PROPERTY,
		LENGTH_PROPERTY, VISUALS_PROPERTY }))
		return;

	try
	{
		anim.m_Name = json.at(NAME_PROPERTY).get<std::string>();
		anim.m_Loop = json.at(LOOP_PROPERTY).get<bool>();
		anim.m_AnimationSpeed = json.at(SPEED_PROPERTY).get<float>();
		anim.m_SingleLoopLength = json.at(LENGTH_PROPERTY).get<float>();
		anim.m_Frames = json.at(VISUALS_PROPERTY).get<std::vector<SpriteAnimationFrame>>();
	}
	catch (const std::exception& e)
	{
		Assert(false, std::format("Tried to deserialize sprite animation:{} but ran into error:{}", 
			JsonUtils::ToStringProperties(json), e.what()));
	}
}
void to_json(Json& json, const SpriteAnimation& anim)
{

	json= { {"Loop", anim.m_Loop}, {"Speed", anim.m_AnimationSpeed},
		{"Length", anim.m_SingleLoopLength }, {"Visuals", anim.m_Frames} };
}

void from_json(const Json& json, SerializableEntity& serializableEntity)
{
	const char* ENITTY_PROPERTY = "Entity";
	const char* SCENE_PROPERTY = "Scene";
	if (!HasRequiredProperties(json, { ENITTY_PROPERTY, SCENE_PROPERTY }))
		return;

	try
	{
		serializableEntity.m_EntityName = json.at(ENITTY_PROPERTY).get<std::string>();
		serializableEntity.m_SceneName = json.at(SCENE_PROPERTY).get<std::string>();
	}
	catch (const std::exception& e)
	{
		Assert(false, std::format("Tried to deserialize serializable entity:{} but ran into error:{}", 
			JsonUtils::ToStringProperties(json), e.what()));
	}
}
void to_json(Json& json, const SerializableEntity& serializableEntity)
{
	json = { {"Entity", serializableEntity.m_EntityName}, {"Scene", serializableEntity.m_SceneName} };
}

Asset* TryDeserializeAsset(const Json& json)
{
	try
	{
		return AssetManager->TryGetAssetFromPathMutable(json.get<std::string>());
	}
	catch (const std::exception& e)
	{
		Assert(false, std::format("Tried to deserialize asset:{} but ran into error:{}", 
			JsonUtils::ToStringProperties(json), e.what()));
	}
	return nullptr;
}
Json TrySerializeAsset(const Asset* asset)
{
	return asset->GetPath();
}
Json TrySerializeAssets(const std::vector<const Asset*>& assets)
{
	std::vector<Json> assetsSerialized;
	for (const auto& asset : assets)
	{
		if (asset == nullptr) continue;
		assetsSerialized.push_back(TrySerializeAsset(asset));
	}
	return assetsSerialized;
}

EntityData* TryDeserializeEntity(const Json& json, const bool& isOptional)
{
	std::function<EntityData*(const Json&)> deserializationAction = [](const Json& json)-> EntityData*
		{
			try
			{
				SerializableEntity serializedEntity = json.get<SerializableEntity>();
				if (!Assert(SceneManager != nullptr, std::format("Tried to parse entity from serialized entity "
					"but parser does not contain valid scene manager")))
					return nullptr;

				if (serializedEntity.m_SceneName == EntityData::GLOBAL_SCENE_NAME)
				{
					return SceneManager->m_GlobalEntityManager.TryGetGlobalEntityMutable(serializedEntity.m_EntityName);
				}

				Scene* maybeScene = SceneManager->TryGetSceneMutable(serializedEntity.m_SceneName);
				if (!Assert(maybeScene != nullptr, std::format("Tried to deserialize entity with non global scene: '{}', "
					"but no scene matches that name", serializedEntity.m_SceneName)))
					return nullptr;

				EntityData* maybeEntity = maybeScene->TryGetEntityMutable(serializedEntity.m_EntityName);
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
			}
			catch (const std::exception& e)
			{
				Assert(false, std::format("Tried to deserialize entity:{} but ran into error:{}", JsonUtils::ToStringProperties(json), e.what()));
				return nullptr;
			}
		};

	if (isOptional)
	{
		std::optional<EntityData*> maybeEntity = TryDeserializeOptional<EntityData*>(json,
			//We try to find entity based on its scene name (or whether it is global)
			[&deserializationAction](const Json& json)-> std::optional<EntityData*>
			{
				return deserializationAction(json);
			});
		/*Assert(false, std::format("Deserialized entity jsoN:{} to has value:{} entity value:{}", JsonUtils::ToStringProperties(json), std::to_string(maybeEntity.has_value()), 
			maybeEntity.has_value()? (maybeEntity.value()!=nullptr? maybeEntity.value()->ToString() : "NULL") : "NO VALUE"));*/

		if (!maybeEntity.has_value()) return nullptr;

		//LogError(std::format("Deserialized json: {} to entity: {}", JsonUtils::ToStringProperties(json), maybeEntity.value()->ToString()));
		return maybeEntity.value();
	}

	EntityData* entityPtr = deserializationAction(json);
	if (!Assert(entityPtr != nullptr, std::format("Tried to deserialize entity for json:{} "
		"but the resulting entity is NULLPTR which is not allowed since it was called as NON OPTIONAL",
		JsonUtils::ToStringProperties(json))))
		return nullptr;

	return entityPtr;
}
Json TrySerializeEntity(const EntityData* entity, const bool& isOptional)
{
	if (isOptional)
	{
		Json json= TrySerializeOptional<const EntityData*>(entity == nullptr ? std::nullopt : std::make_optional(entity),
			[](const EntityData* entity)->Json
			{
				return SerializableEntity(entity->m_SceneName, entity->m_Name);
			});

		//Assert(false, std::format("Serialized optioan entity: {} is:{}", entity!=nullptr? entity->ToString() : "NULL", JsonUtils::ToStringProperties(json)));
		return json;
	}

	if (!Assert(entity != nullptr, std::format("Tried to serialize entity to json but entity is "
		"NULL even with a NON OPTIOANL functional call")))
		return {};

	return SerializableEntity{entity->m_SceneName, entity->m_Name};
}

void from_json(const Json& json, SerializableComponent& serializableComponent)
{
	/*const char* SCENE_PROPERTY = "Scene";
	const char* ENTITY_PROPERTY = "Entity";*/
	const char* COMPONENT_NAME_PROPERTY = "Component";
	if (!HasRequiredProperties(json, { COMPONENT_NAME_PROPERTY }))
		return;

	try
	{
		//TODO: this is knowing about the implmenetation of from json of serializable entity
		const char* ENTITY_NAME_PROPERTY = "Entity";
		if (JsonUtils::HasProperty(json, ENTITY_NAME_PROPERTY) && 
			json.at(ENTITY_NAME_PROPERTY).get<std::string>() == SerializableComponent::SELF_COMPONENT_ENTITY_KEYWORD)
		{
			//LogError(std::format("serialized self component"));
			serializableComponent = SerializableComponent(COMPONENT_NAME_PROPERTY);
			return;
		}
	
		serializableComponent = SerializableComponent(json.get<SerializableEntity>(), 
			json.at(COMPONENT_NAME_PROPERTY).get<std::string>());
	}
	catch (const std::exception& e)
	{
		Assert(false, std::format("Tried to deserialize serializable component:{} but ran into error:{}", 
			JsonUtils::ToStringProperties(json), e.what()));
	}
}
void to_json(Json& json, const SerializableComponent& serializableComponent)
{
	json["Component"] = serializableComponent.m_ComponentName;
	Json serializedEntity = serializableComponent.m_SerializedEntity;
	json.merge_patch(serializedEntity);
}

void from_json(const Json& json, ComponentReference& fieldReference)
{
	try
	{
		SerializableComponent component = json.get<SerializableComponent>();
		if (!Assert(!component.IsComponentOfEntitySelf(), std::format("Tried to deserialize component reference from json:{} "
			"but its storage as serializable component resulted in a self component serialization which is not allowed", 
			JsonUtils::ToStringProperties(json))))
			return;

		EntityData* maybeEntity = SceneManager->TryGetEntityMutable(component.m_SerializedEntity.m_SceneName,
			component.m_SerializedEntity.m_EntityName);
		if (maybeEntity == nullptr) return;

		fieldReference = ComponentReference(*maybeEntity, component.m_ComponentName);
	}
	catch (const std::exception& e)
	{
		Assert(false, std::format("Tried to deserialize component reference:{} but ran into error:{}", JsonUtils::ToStringProperties(json), e.what()));
	}
}
void to_json(Json& json, const ComponentReference& fieldReference)
{
	const EntityData& entity = fieldReference.GetEntitySafe();
	SerializableComponent component = SerializableComponent(entity.m_SceneName, entity.m_Name,
		fieldReference.GetComponentName());
	json = component;
}

void from_json(const Json& json, SerializableField& serializableField)
{
	//const char* SCENE_PROPERTY = "Scene";
	//const char* ENTITY_PROPERTY = "Entity";
	//const char* COMPONENT_NAME_PROPERTY = "Component";
	const char* FIELD_PROPERTY = "Field";
	if (!HasRequiredProperties(json, { FIELD_PROPERTY }))
		return;

	try
	{
		serializableField = SerializableField(json.get<SerializableComponent>(), json.at(FIELD_PROPERTY).get<std::string>());
	}
	catch (const std::exception& e)
	{
		Assert(false, std::format("Tried to deserialize serializable field:{} but ran into error:{}", 
			JsonUtils::ToStringProperties(json), e.what()));
	}
}
void to_json(Json& json, const SerializableField& serializableField)
{
	json["Field"] = serializableField.m_FieldName;
	Json serializedComp = serializableField.m_SerializedComponent;
	json.merge_patch(serializedComp);
}

void from_json(const Json& json, ComponentFieldReference& fieldReference)
{
	try
	{
		SerializableField field = json.get<SerializableField>();
		EntityData* maybeEntity = SceneManager->TryGetEntityMutable(field.m_SerializedComponent.m_SerializedEntity.m_SceneName,
			field.m_SerializedComponent.m_SerializedEntity.m_EntityName);
		if (maybeEntity == nullptr) return;

		fieldReference = ComponentFieldReference(*maybeEntity, field.m_SerializedComponent.m_ComponentName, field.m_FieldName);
	}
	catch (const std::exception& e)
	{
		Assert(false, std::format("Tried to deserialize component field reference:{} but ran into error:{}", JsonUtils::ToStringProperties(json), e.what()));
	}
}	
void to_json(Json& json, const ComponentFieldReference& fieldReference)
{
	const EntityData& entity = fieldReference.GetEntitySafe();
	SerializableField field = SerializableField(entity.m_SceneName, entity.m_Name, 
		fieldReference.m_ComponentRef.GetComponentName(), fieldReference.GetFieldName());
	json = field;
}

void from_json(const Json& json, AnimationPropertyVariant& variant)
{
	const char* TYPE_PROPERTY = "Type";
	const char* PROPERTY_PROPERTY = "Property";
	if (!HasRequiredProperties(json, { TYPE_PROPERTY,  PROPERTY_PROPERTY }))
		return;

	try
	{
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
	catch (const std::exception& e)
	{
		Assert(false, std::format("Tried to deserialize animtion property variant:{} but ran into error:{}", JsonUtils::ToStringProperties(json), e.what()));
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