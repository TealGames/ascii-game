#include "pch.hpp"
#include "JsonUtils.hpp"

namespace JsonUtils
{
	std::string ToStringProperties(const Json& json)
	{
		return json.dump();
	}

	bool HasProperty(const Json& json, const std::string& propertyName)
	{
		return json.contains(propertyName);
	}
}
