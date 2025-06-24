#pragma once
#include "nlohmann/json.hpp"
#include <string>
#include <optional>

using Json = nlohmann::json;
namespace JsonUtils
{
	std::string ToStringProperties(const Json& json);
	bool HasProperty(const Json& json, const std::string& propertyName);

	template<typename T>
	std::optional<T> TryGet(const Json& json)
	{
		std::optional<T> maybeT = std::nullopt;
		try
		{
			maybeT = json.get<T>();
		}
		catch(const std::exception& e)
		{
			return std::nullopt;
		}
		return maybeT;
	}
}

