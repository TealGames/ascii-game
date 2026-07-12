#include <numeric>
#include <string_view>
#include "Utils/ToStringFunctions.hpp"
#include <Utils/StringUtil.hpp>
#include "Utils/Math.hpp"

namespace Utils
{
	static const char* HEX_VALUES = "0123456789ABCDEF";

	std::string ToString(const double d, const std::uint8_t decimalPlaces)
	{
		char buffer[64];
		//NOTE: this is the proper way to convert floating number to string without approximation and is fast
		//FORMAT: fixed -> the length of the decimal is constant 
		auto [stopPtr, result] = std::to_chars(buffer, buffer + sizeof(buffer), d, std::chars_format::fixed, decimalPlaces);
		if (result != std::errc{})
			return "[DOUBLE_TOSTRING_FAILED]";

		return std::string(buffer, std::size_t(stopPtr - buffer));
	}
	std::string ToStringRoundTrip(const double d)
	{
		char buffer[64];
		//FORMAT: general -> may use scientific notation in string representation
		auto [stopPtr, result] = std::to_chars(buffer, buffer + sizeof(buffer), d, std::chars_format::general, 
			std::numeric_limits<double>::max_digits10);

		if (result != std::errc{})
			return "[DOUBLE_RT_TOSTRING_FAILED]";

		return std::string(buffer, std::size_t(stopPtr - buffer));
	}
	std::string ToStringRoundTrip(const float f)
	{
		char buffer[32];
		//FORMAT: general->may use scientific notation in string representation
		auto [stopPtr, result] = std::to_chars(buffer, buffer + sizeof(buffer), f, std::chars_format::general, 
			std::numeric_limits<float>::max_digits10);

		if (result != std::errc{})
			return "[FLOAT__RT_TOSTRING_FAILED]";

		return std::string(buffer, std::size_t(stopPtr - buffer));
	}
	std::string ToString(const char c)
	{
		return std::string(1, c);
	}
	std::string ToString(const std::uint8_t u8)
	{
		return std::to_string(static_cast<int>(u8));
	}
	std::string ToString(const bool b)
	{
		return (b == 0) ? "false" : "true";
	}

	std::string ToStringMemory(const std::byte* bytePtr, const std::size_t byteSize, const bool asHex)
	{
		std::string result = "";
		result.reserve(byteSize * 2);

		unsigned char number = 0;
		for (size_t i = 0; i < byteSize; i++)
		{
			number = std::to_integer<unsigned char>(bytePtr[i]);
			if (asHex)
			{
				result.push_back(HEX_VALUES[number >> 4]);
				result.push_back(HEX_VALUES[number & 0xF]);
			}
			else result += std::to_string(number);
		}
		return result;
	}
	std::string ToString(const std::exception& exception)
	{
		//NOTE: since what is virtual we can just use exception as an arg
		//and due to polymorphism it should invoke most derived what() call
		return exception.what();
	}

	std::string ToStringLeadingZeros(const int& number, const std::uint8_t& maxDigits)
	{
		std::ostringstream stream;
		stream << std::setw(maxDigits) << std::setfill('0') << number;
		return stream.str();
	}

	std::string FormatTypeName(const std::string& typeName)
	{
		const std::string STRUCT_NAME = "struct";
		const std::string CLASS_NAME = "class";
		std::string result = typeName;

		if (typeName.substr(0, STRUCT_NAME.size()) == STRUCT_NAME)
			result = result.substr(STRUCT_NAME.size());

		if (typeName.substr(0, CLASS_NAME.size()) == CLASS_NAME)
			result = result.substr(CLASS_NAME.size());

		return StringUtil::StringUtil(result).TrimSpaces().ToString();
	}
	std::string ToStringTime(const LocalTime& time)
	{
		std::ostringstream oss;
		oss << time;
		std::string timeString = oss.str();

		std::size_t nanosecondIndex = timeString.find('.');
		if (nanosecondIndex != std::string::npos)
		{
			timeString = timeString.substr(0, nanosecondIndex);
		}
		return timeString;
	}

	std::string CollapseToSingleString(const std::vector<std::string>& vec)
	{
		return std::accumulate(vec.begin(), vec.end(), std::string());
	}
	//std::string ToStringIterable(const std::vector<std::string>& strings)
	//{
	//	return ToStringIterable<std::vector<std::string>, std::string>(strings);
	//}
}