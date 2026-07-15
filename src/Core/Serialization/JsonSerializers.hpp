#pragma once
#include <functional>
#include <type_traits>
#include <cstdint>
#include <vector>
#include <optional>
#include "nlohmann/json.hpp"
#include "Core/Serialization/ISerializerImpl.hpp"
#include "Core/Primitives/Vector.hpp"
#include "Math/Quaternion.hpp"
#include "Core/Rendering/RenderLayer.hpp"
#include "Core/Visual/VisualData.hpp"
#include "Core/Visual/TextArray.hpp"
#include "Core/Serialization/SerializedObjects.hpp"
#include "ECS/Component/Component.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "ECS/Component/Types/World/AnimatorComponent.hpp"
#include "Core/Visual/SpriteAnimation.hpp"
#include "ECS/Component/ComponentFieldReference.hpp"
#include "Core/Primitives/ColorGradient.hpp"
#include "Core/Serialization/JsonUtils.hpp"
#include "Core/Primitives/AABB.hpp"
#include "Core/Asset/Asset.hpp"
#include "Utils/ToStringFunctions.hpp"

namespace Engine
{
	using Json = nlohmann::json;
	using JsonOrdered = nlohmann::ordered_json;
	inline const char* OPTIONAL_NULL_VALUE = "null";

	namespace Serialization
	{
		bool HasJsonProperty(const Json& json, const std::string_view& propertyName, bool errorOnInvalid= true);
		bool HasJsonProperties(const Json& json, const std::vector<std::string_view>& propertyNames, bool errorOnInvalid = true);


		template<typename T>
		concept HasJsonConversionFunctions = requires(T& mutableT, const T& constT, Json& mutableJson, const Json& constJson) 
		{
			{ from_json(constJson, mutableT) } -> std::same_as<void>;
			{ to_json(mutableJson, constT) } -> std::same_as<void>;
		};

		class JsonSerializerImpl : public ISerializerImpl
		{
		private:
			Json m_json;
		public:

		private:
		public:
			JsonSerializerImpl();
			~JsonSerializerImpl() = default;

			template<typename T>
			void AddProperty(const char* propertyName, const T& propertyValue)
			{
				static_assert(HasJsonConversionFunctions<T>, "T does not have Json conversion functions");
				//NOTE: since nlohman json lib needs from/to json conversions
				//if we can convert property value type to json this should be a valid automatic conversion
				m_json[propertyName] = propertyValue;
			}

			void ClearJson();
			Json GetJson() const;
		};

		class JsonDeserializerImpl : public IDeserializerImpl
		{
		private:
			Json m_json;
		public:

		private:
		public:
			JsonDeserializerImpl(const Json& json = {});
			~JsonDeserializerImpl() = default;

			template<typename T>
			void GetProperty(const char* propertyName, T* outPropertyValue)
			{
				static_assert(HasJsonConversionFunctions<T>, "T does not have Json conversion functions");

				if (outPropertyValue == nullptr)
					return;

				if (!HasJsonProperty(m_json, propertyName, false))
					return;

				try 
				{
					*outPropertyValue = m_json.at(propertyName).get<T>();
				}
				catch (const std::exception& e)
				{
					return;
				}
			}

			void SetJson(const Json& json);
			Json GetJson() const;
		};
	}

	namespace Utils
	{
		void from_json(const Json& json, Vec2& vec);
		void to_json(Json& json, const Vec2& vec);

		void from_json(const Json& json, Vec2Int& vec);
		void to_json(Json& json, const Vec2Int& vec);
	}

	void from_json(const Json& json, Vec2& vec);
	void to_json(Json& json, const Vec2& vec);

	void from_json(const Json& json, Vec2Int& vec);
	void to_json(Json& json, const Vec2Int& vec);

	void from_json(const Json& json, Vec3& vec);
	void to_json(Json& json, const Vec3& vec);

	namespace Math
	{
		void from_json(const Json& json, Quat& q);
		void to_json(Json& json, const Quat& q);
	}

	void from_json(const Json& json, Array2DPosition& pos);
	void to_json(Json& json, const Array2DPosition& pos);

	template<typename T, size_t N>
	void from_json(const Json& json, Col<T, N>& color)
	{
		try
		{
			/*
			std::optional<std::string> maybeStringProperty = JsonUtils::TryGet<std::string>(json);
			if (maybeStringProperty.has_value())
			{
				std::optional<ColHDR4> maybeConstantColor = Serialization::TryGetConstantColor(maybeStringProperty.value());
				if (!Assert(maybeConstantColor.has_value(), "Tried to convert json:'{} to color using constant "
					"names but it matches no constants!'", JsonUtils::ToStringProperties(json)))
					return;

				color = maybeConstantColor.value();
				return;
			}
			*/

			std::string_view channelNames[4] = { "R", "G", "B", "A" };
			std::array<T, N> channelValues = {};
			for (size_t i = 0; i < N && i < 4; i++)
			{
				if (!Serialization::HasJsonProperty(json, channelNames[i]))
					return;

				channelValues[i] = json.at(channelNames[i]).get<T>();
			}
			color = Col<T, N>(channelValues);
		}
		catch (const std::exception& e)
		{
			Assert(false, "Tried to deserialize color:{} but ran into error:{}",
				JsonUtils::ToStringProperties(json), e.what());
		}
	}

	template<typename T, size_t N>
	void to_json(Json& json, const Col<T, N>& color)
	{
		/*
		std::optional<std::string> maybeConstant = Serialization::TryGetColorConstant(color);
		if (maybeConstant.has_value())
		{
			json = maybeConstant.value();
			return;
		}
		*/

		std::string_view channelNames[4] = { "R", "G", "B", "A" };
		for (size_t i = 0; i < N && i < 4; i++)
		{
			json.emplace(channelNames[i], color[i]);
		}
	}

	template<typename T, size_t N>
	void from_json(const Json& json, ColorGradientKeyFrame<T, N>& gradientFrame)
	{
		const char* COLOR_PROPERTY = "Color";
		const char* LOCATION_PROPERTY = "Location";
		if (!Serialization::HasJsonProperties(json, { COLOR_PROPERTY , LOCATION_PROPERTY }))
			return;

		try
		{
			gradientFrame = ColorGradientKeyFrame(json.at(COLOR_PROPERTY).get<Col<T, N>>(),
				json.at(LOCATION_PROPERTY).get<float>());
		}
		catch (const std::exception& e)
		{
			Assert(false, "Tried to deserialize colorgradient keyframe:{} but ran into error:{}",
				JsonUtils::ToStringProperties(json), e.what());
		}
	}

	template<typename T, size_t N>
	void to_json(Json& json, const ColorGradientKeyFrame<T, N>& gradientFrame)
	{
		json = { {"Color", gradientFrame.m_Color}, {"Location", gradientFrame.m_Location} };
	}

	template<typename T, size_t N>
	void from_json(const Json& json, ColorGradient<T, N>& gradient)
	{
		try
		{
			gradient = ColorGradient(json.get<std::vector<ColorGradientKeyFrame<T, N>>>());
		}
		catch (const std::exception& e)
		{
			Assert(false, "Tried to deserialize colorgradient:{} but ran into error:{}", JsonUtils::ToStringProperties(json), e.what());
		}
	}

	template<typename T, size_t N>
	void to_json(Json& json, const ColorGradient<T,N>& gradient)
	{
		json = gradient.GetKeyframes();
	}

	void from_json(const Json& json, AABB2D& aabb);
	void to_json(Json& json, const AABB2D& aabb);

	namespace Rendering
	{
		void from_json(const Json& json, WorldFontProperties& font);
		void to_json(Json& json, const WorldFontProperties& font);

		void from_json(const Json& json, VisualData& visualData);
		void to_json(Json& json, const VisualData& visualData);

		void from_json(const Json& json, TextBufferCharPosition2D& textChar);
		void to_json(Json& json, const TextBufferCharPosition2D& textChar);

		void from_json(const Json& json, RenderLayerType& layer);
		void to_json(Json& json, const RenderLayerType& layer);

		void from_json(const Json& json, TextChar& textChar);
		void to_json(Json& json, const TextChar& textChar);

		void from_json(const Json& json, TextCharArrayPosition& textChar);
		void to_json(Json& json, const TextCharArrayPosition& textChar);
	}

	namespace Animation
	{
		void from_json(const Json& json, SpriteAnimationFrame& frame);
		void to_json(Json& json, const SpriteAnimationFrame& frame);

		void from_json(const Json& json, SpriteAnimation& anim);
		void to_json(Json& json, const SpriteAnimation& anim);
	}
	
	namespace Serialization
	{
		void from_json(const Json&, SerializedField&);
		void to_json(Json&, const SerializedField&);

		void from_json(const Json&, SerializedComponent&);
		void to_json(Json&, const SerializedComponent&);

		void from_json(const Json&, SerializedEntity&);
		void to_json(Json&, const SerializedEntity&);

		void from_json(const Json&, SerializedAsset&);
		void to_json(Json&, const SerializedAsset&);

		void from_json(const Json&, std::vector<SerializedAsset>&);
		void to_json(Json&, const std::vector<SerializedAsset>&);
	}

	namespace ECS
	{
		void from_json(const Json& json, ECS::ComponentFieldReference& fieldReference);
		void to_json(Json& json, const ECS::ComponentFieldReference& fieldReference);

		void from_json(const Json& json, ECS::ComponentReference& fieldReference);
		void to_json(Json& json, const ECS::ComponentReference& fieldReference);
	}

	namespace Animation
	{
		void from_json(const Json& json, AnimationPropertyVariant& var);
		void to_json(Json& json, const AnimationPropertyVariant& var);

		template<typename T>
		void from_json(const Json& json, AnimationPropertyKeyframe<T>& var)
		{
			const char* TIME_PROPERTY = "Time";
			const char* VALUE_PROPERTY = "Value";
			if (!Serialization::HasJsonProperties(json, { TIME_PROPERTY,  VALUE_PROPERTY }))
				return;

			try
			{
				float time = json.at(TIME_PROPERTY).get<float>();
				T value = json.at(VALUE_PROPERTY).get<T>();

				var = AnimationPropertyKeyframe<T>(value, time);
			}
			catch (const std::exception& e)
			{
				Assert(false, "Tried to deserialize animtion property keyframe but ran into error:{}", e.what());
			}
		}
		template<typename T>
		void to_json(Json& json, const AnimationPropertyKeyframe<T>& var)
		{
			json = { {"Time", var.GetTime()}, {"Value", var.GetValue()} };
		}

		template<typename T>
		void from_json(const Json& json, AnimationProperty<T>& property)
		{
			const char* KEYFRAMES_PROPERTY = "Keyframes";
			const char* FIELD_REF_PROPERTY = "FieldRef";
			if (!Serialization::HasJsonProperties(json, { KEYFRAMES_PROPERTY,  FIELD_REF_PROPERTY }))
				return;

			try
			{
				std::vector<AnimationPropertyKeyframe<T>> keyframes = json.at(KEYFRAMES_PROPERTY).get<std::vector<AnimationPropertyKeyframe<T>>>();
				property = AnimationProperty<T>(json.at(FIELD_REF_PROPERTY).get<ECS::ComponentFieldReference>(), keyframes);
			}
			catch (const std::exception& e)
			{
				Assert(false, "Tried to deserialize animtion property but ran into error:{}", e.what());
			}
		}
		template<typename T>
		void to_json(Json& json, const AnimationProperty<T>& property)
		{
			json = { {"Keyframes", property.m_Keyframes}, {"FieldRef", property.m_ComponentFieldRef} };
		}
	}
}

namespace std
{	
	using Json = nlohmann::json;

	template<typename T>
	void from_json(const Json& json, std::optional<T>& property)
	{
		if (json.is_string() && json.get<std::string>() == Engine::OPTIONAL_NULL_VALUE)
			property = std::nullopt;
		else property = json.get<T>();
	}

	template<typename T>
	void to_json(Json& json, const std::optional<T>& property)
	{
		if (!property.has_value())
			json = { Engine::OPTIONAL_NULL_VALUE };
		else json = property.value();
	}
}