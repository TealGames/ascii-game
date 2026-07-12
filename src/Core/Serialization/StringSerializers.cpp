#include "Core/Serialization/StringSerializers.hpp"
#include "Utils/HelperFunctions.hpp"
#include "Utils/ToStringFunctions.hpp"
#include "Utils/Math.hpp"

namespace StringSerializers
{
	template<>
	bool Deserialize<bool>(const std::string_view& val)
	{
		return val[0] == '1';
	}
	template<>
	int Deserialize<int>(const std::string_view& val)
	{
		return ::Utils::TryParseView<int>(val).value_or(0);
	}
	template<>
	float Deserialize<float>(const std::string_view& val)
	{
		return ::Utils::TryParseView<float>(val).value_or(0.0f);
	}
	template<>
	std::uint8_t Deserialize<std::uint8_t>(const std::string_view& val)
	{
		return ::Utils::TryParseView<std::uint8_t>(val).value_or(0);
	}
	template<>
	std::uint32_t Deserialize<std::uint32_t>(const std::string_view& val)
	{
		return ::Utils::TryParseView<std::uint32_t>(val).value_or(0);
	}

	template<>
	std::string Serialize<bool>(const bool val)
	{
		if (val) return "1";
		else return "0";
	}
	template<>
	std::string Serialize<int>(const int val)
	{
		return std::to_string(val);
	}
	template<>
	std::string Serialize<float>(const float val)
	{
		return ::Utils::ToStringRoundTrip(val);
	}
	template<>
	std::string Serialize<std::uint8_t>(const uint8_t val)
	{
		return std::to_string(val);
	}
	template<>
	std::string Serialize<std::uint32_t>(const uint32_t val)
	{
		return std::to_string(val);
	}
}
