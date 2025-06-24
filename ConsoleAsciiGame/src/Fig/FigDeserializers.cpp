#include "pch.hpp"
#include "FigDeserializers.hpp"
#include "StringUtil.hpp"
#include "HelperFunctions.hpp"

bool ToBool(const std::string& value)
{
	return Utils::StringUtil(value).ToLowerCase().ToString() == "true";
}

int ToInt32(const std::string& value)
{
	return Utils::TryParse<int>(value).value_or(0);
}
float ToFloat(const std::string& value)
{
	return Utils::TryParse<float>(value).value_or(0);
}
std::uint8_t ToUint8(const std::string& value)
{
	return Utils::TryParse<std::uint8_t>(value).value_or(0);
}