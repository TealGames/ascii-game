#include "pch.hpp"
#include "JsonUtils.hpp"

namespace JsonUtils
{
	std::string ToStringProperties(const Json& json)
	{
		return json.dump();
	}
}
