#include "Utils/ToStringFunctions.hpp
#include "Utils/Math.hpp"

namespace Utils
{
	std::string ToString(const double& d, const std::uint8_t decimalPlaces)
	{
		std::ostringstream oss;

		//The precision is the total number of digits, so we do 
		// decimal places + number of non-decimal digit places (total digits - decimal digits)
		oss.precision(decimalPlaces + (GetDigitPlaces(d)- GetDecimalPlaces(d));
		oss << d;
		return oss.str();
	}
	std::string ToString(const char c)
	{
		return std::string(1, c);
	}
	std::string ToString(const std::uint8_t u8)
	{
		return std::to_string(static_cast<int>(u8));
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
	std::string ToStringIterable(const std::vector<std::string>& strings)
	{
		return ToStringIterable<std::vector<std::string>, std::string>(strings);
	}
}