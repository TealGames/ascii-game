#pragma once
#include "Core/Primitives/Color.hpp"
#include "Core/Asset/Asset.hpp"
#include "Fig/Fig.hpp"
#include "Core/Serialization/SerializedObjects.hpp"
#include "Utils/StringUtil.hpp"
#include "Core/Serialization/SerializationUtils.hpp"

namespace Engine::Rendering { class Texture; }
namespace FigFormat
{
	template<typename T, size_t N>
	requires (N >= 1)
	struct FigSerializer<Engine::Col<T,N>>
	{
		static void FromFigValue(const std::string& value, Engine::Col<T, N>& outVal)
		{
			FigSerializer<std::array<T, N>>::FromFigValue(value, reinterpret_cast<std::array<T, N>&>(outVal));
			/*
			std::string channelStr = "";
			std::uint8_t currentChannelIndex = 0;
			for (size_t i=0; i<value.size(); i++)
			{
				char c = value[i];
				const bool isSpace = (c == ' ');
				//NOTE: if the last char is not a space and we have one component left, we still want to
				//update and parse that component even if we do not find a space after it
				if (isSpace || i == value.size() - 1)
				{
					outVal[currentChannelIndex] = std::max(FigFormat::ToFloat(channelStr), 0.0f);
					//If the result has no decimal -> it means we were in [0, 255] scale so we adjust to [0, 1]
					if (channelStr.find('.') == std::string::npos)
						outVal[currentChannelIndex] /= MAX_INT_COLOR_CHANNEL;

					currentChannelIndex++;
					if (currentChannelIndex >= N)
						break;

					channelStr = "";
				}
				else channelStr += c;
			}
			*/
		}

		static void ToFigValue(const Engine::Col<T, N>& value, std::string& outVal)
		{
			FigSerializer<std::array<T,N>>::ToFigValue(reinterpret_cast<const std::array<T, N>&>(value), outVal);
		}
	};
	

	template<typename T, size_t N>
		requires (N >= 1)
	Engine::Col<T, N> ToColor(const std::string& value)
	{
		Engine::Col<T,N> color = {};
		FigSerializer<Engine::Col<T, N>>::FromFigValue(value, color);
		return color;
	}

	template<typename T>
	requires (std::is_pointer_v<T>&& Engine::Assets::IsAssetType<std::remove_pointer_t<T>>)
	struct FigSerializer<T>
	{
		static void FromFigValue(const std::string& value, T& outVal)
		{
			std::string trimmedValue = Utils::StringUtil(value).TrimAnySpaceChar().ToString();
			if (FigFormat::IsNull(trimmedValue, false))
				return;

			Engine::Serialization::SerializedAsset serializedAsset = { trimmedValue };
			outVal = Engine::Serialization::TryDeserializeTypeAsset<std::remove_pointer_t<T>>(serializedAsset);
			ENGINE_ASSERT(outVal != nullptr, "Failed to deserialize asset for Fig: {}", trimmedValue);
		}

		static void ToFigValue(const T& value, std::string& outVal)
		{
			if (value == nullptr)
			{
				outVal += Fig::NULL_VALUE;
				return;
			}
			const Engine::Assets::Asset* assetPtr = static_cast<const Engine::Assets::Asset*>(value);
			
			std::string pathStr = assetPtr->GetAbsolutePath().string();
			outVal += pathStr;
		}
	};

	template<typename T>
		requires Engine::Assets::IsAssetType<T>
	T* ToAsset(const std::string& value)
	{
		T* assetPtr = nullptr;
		FigSerializer<T*>::FromFigValue(value, assetPtr);
		return assetPtr;
	}
}