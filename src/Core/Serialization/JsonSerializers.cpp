#include "pch.hpp"
#include "Core/Serialization/JsonSerializers.hpp"
#include "Core/Serialization/JsonConstants.hpp"
#include "Core/Serialization/SerializationUtils.hpp"
#include "Utils/Debug.hpp"
#include "Core/Asset/FontAsset.hpp"
#include "Utils/ToStringFunctions.hpp"

namespace Engine
{
	namespace Serialization
	{
		bool HasJsonProperty(const Json& json, const std::string_view& propertyName, bool errorOnInvalid)
		{
			bool hasProperty = json.contains(propertyName);
			if (!hasProperty && errorOnInvalid)
			{
				LogError(std::format("Tried to parse json: '{}' to type "
					"but it is missing the property: '{}'", JsonUtils::ToStringProperties(json), propertyName));
			}
			return hasProperty;
		}
		bool HasJsonProperties(const Json& json, const std::vector<std::string_view>& propertyNames, bool errorOnInvalid)
		{
			for (const auto& propertyName : propertyNames)
			{
				if (!HasJsonProperty(json, propertyName, errorOnInvalid))
				{
					return false;
				}
			}
			return true;
		}

		JsonSerializerImpl::JsonSerializerImpl() : m_json() {}
		void JsonSerializerImpl::ClearJson() { m_json = {}; }
		Json JsonSerializerImpl::GetJson() const { return m_json; }

		JsonDeserializerImpl::JsonDeserializerImpl(const Json& json) : m_json(json) {}
		void JsonDeserializerImpl::SetJson(const Json& json) { m_json = json; }
		Json JsonDeserializerImpl::GetJson() const { return m_json; }
	}

	namespace Utils
	{
		void from_json(const Json& json, Vec2& vec)
		{
			const char* X_PROPERTY = "X";
			const char* Y_PROPERTY = "Y";
			if (!Serialization::HasJsonProperties(json, { X_PROPERTY, Y_PROPERTY })) return;

			try
			{
				vec.m_X = json.at(X_PROPERTY).get<float>();
				vec.m_Y = json.at(Y_PROPERTY).get<float>();
			}
			catch (const std::exception& e)
			{
				LogError(std::format("Tried to deserialize vec2:{} but ran into error:{}", JsonUtils::ToStringProperties(json), e.what()));
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
			if (!Serialization::HasJsonProperties(json, { X_PROPERTY,  Y_PROPERTY })) return;

			try
			{
				vec.m_X = json.at(X_PROPERTY).get<int>();
				vec.m_Y = json.at(Y_PROPERTY).get<int>();
			}
			catch (const std::exception& e)
			{
				LogError(std::format("Tried to deserialize vec2int:{} but ran into error:{}", JsonUtils::ToStringProperties(json), e.what()));
			}
		}
		void to_json(Json& json, const Vec2Int& vec)
		{
			json = { {"X", vec.m_X}, {"Y", vec.m_Y} };
		}
	}

	void from_json(const Json& json, Vec2& vec)
	{
		const char* X_PROPERTY = "X";
		const char* Y_PROPERTY = "Y";
		if (!Serialization::HasJsonProperties(json, { X_PROPERTY,  Y_PROPERTY })) return;

		try
		{
			vec.m_X = json.at(X_PROPERTY).get<float>();
			vec.m_Y = json.at(Y_PROPERTY).get<float>();
		}
		catch (const std::exception& e)
		{
			LogError(std::format("Tried to deserialize vec2:{} but ran into error:{}", JsonUtils::ToStringProperties(json), e.what()));
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
		if (!Serialization::HasJsonProperties(json, { X_PROPERTY,  Y_PROPERTY })) 
			return;

		try
		{
			vec.m_X = json.at(X_PROPERTY).get<int>();
			vec.m_Y = json.at(Y_PROPERTY).get<int>();
		}
		catch (const std::exception& e)
		{
			LogError(std::format("Tried to deserialize vec2int:{} but ran into error:{}", JsonUtils::ToStringProperties(json), e.what()));
		}
	}
	void to_json(Json& json, const Vec2Int& vec)
	{
		json = { {"X", vec.m_X}, {"Y", vec.m_Y} };
	}

	void from_json(const Json& json, Vec3& vec)
	{
		const char* X_PROPERTY = "X";
		const char* Y_PROPERTY = "Y";
		const char* Z_PROPERTY = "Z";
		if (!Serialization::HasJsonProperties(json, { X_PROPERTY,  Y_PROPERTY, Z_PROPERTY })) 
			return;

		try
		{
			vec.m_X = json.at(X_PROPERTY).get<float>();
			vec.m_Y = json.at(Y_PROPERTY).get<float>();
			vec.m_Z = json.at(Z_PROPERTY).get<float>();
		}
		catch (const std::exception& e)
		{
			LogError(std::format("Tried to deserialize vec3:{} but ran into error:{}", JsonUtils::ToStringProperties(json), e.what()));
		}
	}
	void to_json(Json& json, const Vec3& vec)
	{
		json = { {"X", vec.m_X}, {"Y", vec.m_Y}, {"Z", vec.m_Z} };
	}

	namespace Math
	{
		void from_json(const Json& json, Quat& q)
		{
			const char* X_PROPERTY = "X";
			const char* Y_PROPERTY = "Y";
			const char* Z_PROPERTY = "Z";
			const char* W_PROPERTY = "W";
			if (!Serialization::HasJsonProperties(json, { X_PROPERTY,  Y_PROPERTY, Z_PROPERTY, W_PROPERTY })) return;

			try
			{
				q.m_X = json.at(X_PROPERTY).get<float>();
				q.m_Y = json.at(Y_PROPERTY).get<float>();
				q.m_Z = json.at(Z_PROPERTY).get<float>();
				q.m_W = json.at(W_PROPERTY).get<float>();
			}
			catch (const std::exception& e)
			{
				LogError(std::format("Tried to deserialize quaternion:{} but ran into error:{}", JsonUtils::ToStringProperties(json), e.what()));
			}
		}
		void to_json(Json& json, const Quat& q)
		{
			json = { {"X", q.m_X}, {"Y", q.m_Y}, {"Z", q.m_Z}, {"W", q.m_W} };
		}
	}

	void from_json(const Json& json, Array2DPosition& pos)
	{
		const char* ROW_PROPERTY = "Row";
		const char* COL_PROPERTY = "Col";
		if (!Serialization::HasJsonProperties(json, { ROW_PROPERTY,  COL_PROPERTY })) return;

		try
		{
			pos = Array2DPosition(json.at(ROW_PROPERTY).get<int>(), json.at(COL_PROPERTY).get<int>());
		}
		catch (const std::exception& e)
		{
			LogError(std::format("Tried to deserialize array2dpos:{} but ran into error:{}", JsonUtils::ToStringProperties(json), e.what()));
		}
	}
	void to_json(Json& json, const Array2DPosition& pos)
	{
		json = { {"Row", pos.GetRow()}, {"Col", pos.GetCol()} };
	}

	void from_json(const Json& json, AABB2D& aabb)
	{
		const char* SIZE_PROPERTY = "Size";
		if (!Serialization::HasJsonProperties(json, { SIZE_PROPERTY }))
			return;

		try
		{
			aabb = AABB2D(json.at(SIZE_PROPERTY).get<Vec2>());
		}
		catch (const std::exception& e)
		{
			LogError(std::format("Tried to deserialize aabb:{} but ran into error:{}", JsonUtils::ToStringProperties(json), e.what()));
		}
	}
	void to_json(Json& json, const AABB2D& aabb)
	{
		json = { {"Size", aabb.GetSize()} };
	}

	namespace Rendering
	{
		void from_json(const Json& json, WorldFontProperties& font)
		{
			const char* FONT_PROEPRTY = "Font";
			const char* FONT_SIZE_PROPERTY = "FontSize";
			const char* TRACKING_PROPERTY = "Tracking";
			Serialization::SerializedAsset serializedFontAsset = json.at(FONT_PROEPRTY).get<Serialization::SerializedAsset>();
			Rendering::FontAsset* fontAsset = Serialization::TryDeserializeTypeAsset<Rendering::FontAsset>(serializedFontAsset);

			try
			{
				Vec2 fontSize = {};
				Json fontJson = json.at(FONT_SIZE_PROPERTY);
				if (fontJson.is_string())
				{
					//LogError("Reached font json string");
					std::optional<Vec2> maybeFontSize = Serialization::TryGetConstantFontSize(fontJson.get<std::string>());
					if (!Assert(maybeFontSize.has_value(), "Tried to convert json: {} to font data but font "
						"size could not be deduced from '{}' property", JsonUtils::ToStringProperties(json), FONT_SIZE_PROPERTY))
						return;
					fontSize = maybeFontSize.value();
				}
				else fontSize = fontJson.get<Vec2>();

				font = WorldFontProperties(fontSize, json.at(TRACKING_PROPERTY).get<float>(), *fontAsset);
			}
			catch (const std::exception& e)
			{
				LogError(std::format("Tried to deserialize fontdata:{} but ran into error:{}", JsonUtils::ToStringProperties(json), e.what()));
			}
		}

		void to_json(Json& json, const WorldFontProperties& font)
		{
			json["Font"] = Serialization::TrySerializeAsset(font.m_FontAsset);

			std::optional<std::string> maybeFontSizeConstant = Serialization::TryGetFontSizeConstant(font.m_RectSize);
			if (maybeFontSizeConstant.has_value()) json["FontSize"] = maybeFontSizeConstant.value();
			else json["FontSize"] = font.m_RectSize;
		}

		void from_json(const Json& json, VisualData& visualData)
		{
			const char* BUFFER_PROPERTY = "Buffer";
			/*const char* FONT_PROEPRTY = "Font";
			const char* FONT_SIZE_PROPERTY = "FontSize";*/
			const char* PIVOT_PROPERTY = "Pivot";
			if (!Serialization::HasJsonProperties(json, { BUFFER_PROPERTY, PIVOT_PROPERTY }))
				return;

			try
			{
				auto textChars = json.at(BUFFER_PROPERTY).get<std::vector<TextBufferCharPosition2D>>();

				Vec2 pivotPos = VisualData::DEFAULT_PIVOT;
				Json pivotJson = json.at(PIVOT_PROPERTY);
				if (pivotJson.is_string())
				{
					std::optional<Vec2> maybePivot = Serialization::TryGetConstantPivot(pivotJson.get<std::string>());
					if (!Assert(maybePivot.has_value(), "Tried to convert json: {} to visual data but pivot "
						"could not be deduced from '{}' property", JsonUtils::ToStringProperties(json), PIVOT_PROPERTY))
						return;
					pivotPos = maybePivot.value();
				}
				else pivotPos = pivotJson.get<Vec2>();

				visualData = VisualData(textChars, NormalizedVec2(pivotPos));
			}
			catch (const std::exception& e)
			{
				LogError(std::format("Tried to deserialize visualdata:{} but ran into error:{}", JsonUtils::ToStringProperties(json), e.what()));
			}
		}

		void to_json(Json& json, const VisualData& visualData)
		{
			/*json["Font"] = TrySerializeFont(visualData.GetFont());
			std::optional<std::string> maybeFontSizeConstant = JsonConstants::TryGetFontSizeConstant(visualData.GetFontSize());
			if (maybeFontSizeConstant.has_value()) json["FontSize"] = maybeFontSizeConstant.value();
			else json["FontSize"] = visualData.GetFontSize();*/
			json["Buffer"] = visualData.GetBuffer();

			std::optional<std::string> maybePivotConstant = Serialization::TryGetPivotConstant(visualData.GetPivotRelative());
			if (maybePivotConstant.has_value()) json["Pivot"] = maybePivotConstant.value();
			else json["Pivot"] = visualData.GetPivotRelative();
		}

		void from_json(const Json& json, TextBufferCharPosition2D& textChar)
		{
			const char* TEXT_CHAR_PROPERTY = "Text";
			const char* FONT_PROEPRTY = "Font";
			const char* POS_PROPERTY = "Pos";
			if (!Serialization::HasJsonProperties(json, { TEXT_CHAR_PROPERTY, POS_PROPERTY, FONT_PROEPRTY })) return;

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
				textChar = TextBufferCharPosition2D(json.at(POS_PROPERTY).get<Vec2>(),
					json.at(TEXT_CHAR_PROPERTY).get<Rendering::TextChar>(), json.at(FONT_PROEPRTY).get<Rendering::WorldFontProperties>());
			}
			catch (const std::exception& e)
			{
				LogError(std::format("Tried to deserialize text buffer pos:{} but ran into error:{}", JsonUtils::ToStringProperties(json), e.what()));
			}
		}
		void to_json(Json& json, const TextBufferCharPosition2D& textChar)
		{
			/*json["Font"] = TrySerializeFont(textChar.m_FontData.m_Font);

			std::optional<std::string> maybeFontSizeConstant = JsonConstants::TryGetFontSizeConstant(textChar.m_FontData.m_FontSize);
			if (maybeFontSizeConstant.has_value()) json["FontSize"] = maybeFontSizeConstant.value();
			else json["FontSize"] = textChar.m_FontData.m_FontSize;*/
			json["Text"] = textChar.m_Text;
			json["Pos"] = textChar.m_Pos;
			json["Font"] = textChar.m_FontData;
		}

		void from_json(const Json& json, RenderLayerType& layer)
		{
			try
			{
				layer = GetLayersFromStrings(json.get<std::vector<std::string>>());
			}
			catch (const std::exception& e)
			{
				LogError(std::format("Tried to deserialize renderlayertype:{} but ran into error:{}", JsonUtils::ToStringProperties(json), e.what()));
			}
			//LogError(std::format("Layer updating to; {}", ToString(layer)));
		}
		void to_json(Json& json, const RenderLayerType& layer)
		{
			json = GetLayersAsStrings(layer);
		}

		void from_json(const Json& json, TextChar& textChar)
		{
			const char* COLOR_PROPERTY = "Color";
			const char* CHAR_PROPERTY = "Char";
			if (!Serialization::HasJsonProperties(json, { COLOR_PROPERTY, CHAR_PROPERTY }))
				return;

			try
			{
				textChar = TextChar(json.at(COLOR_PROPERTY).get<ColHDR4>(), json.at(CHAR_PROPERTY).get<char>());
			}
			catch (const std::exception& e)
			{
				LogError(std::format("Tried to deserialize text char:{} but ran into error:{}", JsonUtils::ToStringProperties(json), e.what()));
			}
		}
		void to_json(Json& json, const TextChar& textChar)
		{
			json = { {"Color", textChar.m_Color}, {"Char", textChar.GetChar()} };
		}

		void from_json(const Json& json, TextCharArrayPosition& textChar)
		{
			const char* COLOR_PROPERTY = "Color";
			const char* CHAR_PROPERTY = "Char";
			const char* POS_PROPERTY = "Pos";
			if (!Serialization::HasJsonProperties(json, { COLOR_PROPERTY, CHAR_PROPERTY, POS_PROPERTY }))
				return;

			try
			{
				textChar = TextCharArrayPosition(json.at(POS_PROPERTY).get<Array2DPosition>(),
					TextChar(json.at(COLOR_PROPERTY).get<ColHDR4>(), json.at(CHAR_PROPERTY).get<std::string>()[0]));
			}
			catch (const std::exception& e)
			{
				LogError(std::format("Tried to deserialize textcharpos:{} but ran into error:{}", JsonUtils::ToStringProperties(json), e.what()));
			}
		}
		void to_json(Json& json, const TextCharArrayPosition& textChar)
		{
			json = { {"Pos", textChar.m_RowColPos}, {"Char", std::string(1, textChar.m_Text.GetChar())}, {"Color", textChar.m_Text.m_Color} };
		}
	}

	namespace Animation
	{
		void from_json(const Json& json, SpriteAnimationFrame& frame)
		{
			const char* TIME_PROPERTY = "Time";
			const char* VISUAL_PROPERTY = "Visual";
			if (!Serialization::HasJsonProperties(json, { TIME_PROPERTY, VISUAL_PROPERTY }))
				return;

			try
			{
				frame = SpriteAnimationFrame(json.at(TIME_PROPERTY).get<float>(), json.at(VISUAL_PROPERTY).get<Rendering::VisualData>());
			}
			catch (const std::exception& e)
			{
				LogError(std::format("Tried to deserialize sprite animation frame:{} but ran into error:{}",
					JsonUtils::ToStringProperties(json), e.what()));
			}
		}
		void to_json(Json& json, const SpriteAnimationFrame& frame)
		{
			json = { {"Time", frame.m_Time}, {"Visual", frame.m_VisualFrame} };
		}

		void from_json(const Json& json, SpriteAnimation& anim)
		{
			const char* NAME_PROPERTY = "Name";
			const char* LOOP_PROPERTY = "Loop";
			const char* SPEED_PROPERTY = "Speed";
			const char* LENGTH_PROPERTY = "Length";
			const char* VISUALS_PROPERTY = "Visuals";
			if (!Serialization::HasJsonProperties(json, { NAME_PROPERTY, LOOP_PROPERTY, SPEED_PROPERTY,
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
				LogError(std::format("Tried to deserialize sprite animation:{} but ran into error:{}",
					JsonUtils::ToStringProperties(json), e.what()));
			}
		}
		void to_json(Json& json, const SpriteAnimation& anim)
		{

			json = { {"Loop", anim.m_Loop}, {"Speed", anim.m_AnimationSpeed},
				{"Length", anim.m_SingleLoopLength }, {"Visuals", anim.m_Frames} };
		}
	}

	namespace Serialization
	{
		void from_json(const Json& json, SerializedField& serializableField)
		{
			//const char* SCENE_PROPERTY = "Scene";
			//const char* ENTITY_PROPERTY = "Entity";
			//const char* COMPONENT_NAME_PROPERTY = "Component";
			const char* FIELD_PROPERTY = "Field";
			if (!Serialization::HasJsonProperties(json, { FIELD_PROPERTY }))
				return;

			try
			{
				serializableField = SerializedField(json.get<SerializedComponent>(),
					json.at(FIELD_PROPERTY).get<std::string>());
			}
			catch (const std::exception& e)
			{
				LogError(std::format("Tried to deserialize serializable field:{} but ran into error:{}",
					JsonUtils::ToStringProperties(json), e.what()));
			}
		}
		void to_json(Json& json, const SerializedField& serializableField)
		{
			json["Field"] = serializableField.m_FieldName;
			Json serializedComp = serializableField.m_SerializedComponent;
			json.merge_patch(serializedComp);
		}

		void from_json(const Json& json, SerializedComponent& serializableComponent)
		{
			/*const char* SCENE_PROPERTY = "Scene";
			const char* ENTITY_PROPERTY = "Entity";*/
			const char* COMPONENT_NAME_PROPERTY = "Component";
			if (!Serialization::HasJsonProperties(json, { COMPONENT_NAME_PROPERTY }))
				return;

			try
			{
				serializableComponent = SerializedComponent(json.get<SerializedEntity>(),
					json.at(COMPONENT_NAME_PROPERTY).get<std::string>());
			}
			catch (const std::exception& e)
			{
				LogError(std::format("Tried to deserialize serializable component:{} but ran into error:{}",
					JsonUtils::ToStringProperties(json), e.what()));
			}
		}
		void to_json(Json& json, const SerializedComponent& serializableComponent)
		{
			json["Component"] = serializableComponent.m_ComponentName;
			Json serializedEntity = serializableComponent.m_SerializedEntity;
			json.merge_patch(serializedEntity);
		}

		void from_json(const Json& json, SerializedEntity& serializableEntity)
		{
			const char* ENITTY_PROPERTY = "Entity";
			const char* SCENE_PROPERTY = "Scene";
			if (!Serialization::HasJsonProperties(json, { ENITTY_PROPERTY, SCENE_PROPERTY }))
				return;

			try
			{
				serializableEntity.m_EntityName = json.at(ENITTY_PROPERTY).get<std::string>();
				serializableEntity.m_SceneName = json.at(SCENE_PROPERTY).get<std::string>();
			}
			catch (const std::exception& e)
			{
				LogError(std::format("Tried to deserialize serializable entity:{} but ran into error:{}",
					JsonUtils::ToStringProperties(json), e.what()));
			}
		}
		void to_json(Json& json, const SerializedEntity& serializableEntity)
		{
			json = { {"Entity", serializableEntity.m_EntityName}, {"Scene", serializableEntity.m_SceneName} };
		}

		void from_json(const Json& json, SerializedAsset& serializedAsset)
		{
			const char* PATH_PROPERTY = "AssetPath";
			if(!Serialization::HasJsonProperties(json, { PATH_PROPERTY }))
				return;

			try
			{
				serializedAsset.m_AssetPath = json.at(PATH_PROPERTY).get<std::filesystem::path>();
			}
			catch (const std::exception& e)
			{
				LogError(std::format("Tried to deserialize serializable entity:{} but ran into error:{}",
					JsonUtils::ToStringProperties(json), e.what()));
			}
		}
		void to_json(Json& json, const SerializedAsset& serializedAsset)
		{
			json = {"AssetPath", serializedAsset.m_AssetPath};
		}

		void from_json(const Json& json, std::vector<SerializedAsset>& serializedAssets)
		{
			for (const auto& jsonProperty : json.get<std::vector<Json>>())
			{
				SerializedAsset serializedAsset = jsonProperty;
				serializedAssets.push_back(serializedAsset);
			}
		}
		void to_json(Json& json, const std::vector<SerializedAsset>& serializedAssets)
		{
			for (const auto& serializedAsset : serializedAssets)
			{
				json.push_back(serializedAsset);
			}
		}
	}
	
	namespace ECS
	{
		void from_json(const Json& json, ComponentFieldReference& fieldReference)
		{
			try
			{
				Serialization::SerializedField serializedField = json.get<Serialization::SerializedField>();
				EntityData* maybeEntity = Serialization::TryDeserializeEntity(serializedField.m_SerializedComponent.m_SerializedEntity);
				if (maybeEntity == nullptr) return;

				fieldReference = ComponentFieldReference(*maybeEntity, serializedField.m_SerializedComponent.m_ComponentName, serializedField.m_FieldName);
			}
			catch (const std::exception& e)
			{
				LogError(std::format("Tried to deserialize component field reference:{} but ran into error:{}", JsonUtils::ToStringProperties(json), e.what()));
			}
		}
		void to_json(Json& json, const ComponentFieldReference& fieldReference)
		{
			const EntityData& entity = fieldReference.GetEntitySafe();
			Serialization::SerializedField field = Serialization::SerializedField(entity.m_SceneName, entity.m_Name,
				fieldReference.m_ComponentRef.GetComponentName(), fieldReference.GetFieldName());
			json = field;
		}

		void from_json(const Json& json, ComponentReference& fieldReference)
		{
			try
			{
				Serialization::SerializedComponent serializedComponent = json.get<Serialization::SerializedComponent>();
				ECS::EntityData* maybeEntity = Serialization::TryDeserializeEntity(serializedComponent.m_SerializedEntity);
				if (maybeEntity == nullptr) return;

				fieldReference = ECS::ComponentReference(*maybeEntity, serializedComponent.m_ComponentName);
			}
			catch (const std::exception& e)
			{
				LogError(std::format("Tried to deserialize component reference:{} but ran into error:{}", JsonUtils::ToStringProperties(json), e.what()));
			}
		}
		void to_json(Json& json, const ComponentReference& fieldReference)
		{
			const EntityData& entity = fieldReference.GetEntitySafe();
			Serialization::SerializedComponent component = Serialization::SerializedComponent(entity.m_SceneName, entity.m_Name,
				fieldReference.GetComponentName());
			json = component;
		}
	}
	

	

	namespace Animation
	{
		void from_json(const Json& json, AnimationPropertyVariant& variant)
		{
			const char* TYPE_PROPERTY = "Type";
			const char* PROPERTY_PROPERTY = "Property";
			if (!Serialization::HasJsonProperties(json, { TYPE_PROPERTY,  PROPERTY_PROPERTY }))
				return;

			try
			{
				std::string propertyType = json.at(TYPE_PROPERTY).get<std::string>();
				if (propertyType == ::Utils::ToStringTypeName<int>())
				{
					variant = AnimationPropertyVariant(json.at(PROPERTY_PROPERTY).get<AnimationProperty<int>>());
				}
				else if (propertyType == ::Utils::ToStringTypeName<float>())
				{
					variant = json.at(PROPERTY_PROPERTY).get<AnimationProperty<float>>();
				}
				else if (propertyType == ::Utils::ToStringTypeName<std::uint8_t>())
				{
					variant = json.at(PROPERTY_PROPERTY).get<AnimationProperty<std::uint8_t>>();
				}
				else
				{
					LogError(std::format("Tried to deserialize json:{} to animtion property variant, "
						"but could not find actions for type:{}", JsonUtils::ToStringProperties(json), propertyType));
				}
			}
			catch (const std::exception& e)
			{
				LogError(std::format("Tried to deserialize animtion property variant:{} but ran into error:{}", JsonUtils::ToStringProperties(json), e.what()));
			}
		}
		void to_json(Json& json, const AnimationPropertyVariant& var)
		{
			if (std::holds_alternative<AnimationProperty<int>>(var))
			{
				json = { {"Type", ::Utils::ToStringTypeName<int>() },
					{"Property", std::any_cast<AnimationProperty<int>>(var)} };
			}
			else if (std::holds_alternative<AnimationProperty<float>>(var))
			{
				json = { {"Type", ::Utils::ToStringTypeName<float>() },
					   {"Property", std::any_cast<AnimationProperty<float>>(var)} };

			}
			else if (std::holds_alternative<AnimationProperty<std::uint8_t>>(var))
			{
				json = { {"Type", ::Utils::ToStringTypeName<std::uint8_t>() },
					{"Property", std::any_cast<AnimationProperty<std::uint8_t>>(var)} };
			}
			else
			{
				LogError(std::format("Tried to serialize animation property variatn to json, "
					"but could not find actions for its type"));
			}
		}
	}
}
