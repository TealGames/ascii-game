#include "pch.hpp"

#include <cctype>
#include <numbers>
#include <numeric>
#include <cmath>
#include <chrono>
#include <limits>
#include <fstream>
#include <filesystem>
#include <random>
#include <stacktrace>
#include "HelperFunctions.hpp"

namespace Utils
{
	constexpr bool IsCurrentVersion(const CPPVersion& version)
	{
		return __cplusplus == static_cast<long>(version);
	}

	LocalTime GetLocalTime(const SystemTime& time)
	{
		return std::chrono::zoned_time{ std::chrono::current_zone(), time };
	}
	LocalTime GetCurrentTime()
	{
		return GetLocalTime(std::chrono::system_clock::now());
	}
	std::string FormatTime(const LocalTime& time)
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

	std::string GetCurrentStackTrace() 
	{
		std::ostringstream stream;
		stream << std::stacktrace::current();
		return stream.str();
	}

	void ClearSTDCIN()
	{
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}

	bool ContainsIntegralValues(const std::string& input, const bool includeNegativeSign)
	{
		for (const auto& c : input)
		{
			if (std::isdigit(c) || (includeNegativeSign && c == '-')) 
				return true;
		}
		return false;
	}
	size_t GetFirstIngteralValueIndex(const std::string& input, const bool includeNegativeSign)
	{
		for (size_t i=0; i<input.size(); i++)
		{
			if (std::isdigit(input[i]) || (includeNegativeSign && input[i]== '-'))
				return i;
		}
		return std::string::npos;
	}

	std::string TryExtractInt(const std::string& input)
	{
		size_t firstIngegralIndex = GetFirstIngteralValueIndex(input, true);
		if (firstIngegralIndex == std::string::npos) return "";

		std::string result = "";
		char currentChar = '0';
		for (size_t i= firstIngegralIndex; i<input.size(); i++)
		{
			currentChar = input[i];
			if (std::isdigit(currentChar) || (result.empty() && currentChar == '-'))
			{
				result += currentChar;
			}
		}

		return result;
	}
	std::string TryExtractFloat(const std::string& input)
	{
		size_t firstIngegralIndex = GetFirstIngteralValueIndex(input, true);
		if (firstIngegralIndex == std::string::npos) return "";

		std::string result = "";
		bool foundDecimal = false;
		char currentChar = '0';
		for (size_t i = firstIngegralIndex; i < input.size(); i++)
		{
			currentChar = input[i];
			if (currentChar == '.')
			{
				if (!foundDecimal) result += currentChar;
				foundDecimal = true;
			}
			else if (std::isdigit(currentChar) || (result.empty() && currentChar == '-'))
			{
				result += currentChar;
			}
		}

		return result;
	}
	std::string TryExtractNonIntegralValues(const std::string& input)
	{
		std::string result = "";
		for (const auto& c : input)
		{
			if (!std::isdigit(c))
			{
				result += c;
			}
		}
		return result;
	}

	std::vector<std::string> Split(const std::string& str, const char& separator)
	{
		std::vector<std::string> tokens;
		for (auto part : std::views::split(str, separator))
		{
			tokens.emplace_back(part.begin(), part.end());
		}
		return tokens;
	}

	std::string ToStringLeadingZeros(const int& number, const std::uint8_t& maxDigits)
	{
		std::ostringstream stream;
		stream << std::setw(maxDigits) << std::setfill('0') << number;
		return stream.str();
	}

	double ToRadians(const double deg)
	{
		return deg * (std::numbers::pi / 180.0);
	}

	double ToDegrees(const double rad)
	{
		return rad * (180.0 / std::numbers::pi);
	}

	/// <summary>
	/// To handle approximate equivalence between floating numbers
	/// </summary>
	/// <param name="d1"></param>
	/// <param name="d2"></param>
	/// <returns></returns>
	bool ApproximateEquals(double d1, double d2)
	{
		double diff = std::fabs(d2 - d1);
		return diff < std::numeric_limits<double>().epsilon();
	}

	bool ApproximateEqualsF(float f1, float f2)
	{
		double diff = std::fabs(f2 - f1);
		return diff < std::numeric_limits<double>().epsilon();
	}

	bool IsPosInifinity(double value)
	{
		//Note: sign bit returns true if negative value
		return std::isinf(value) && !std::signbit(value);
	}

	bool IsNegInifinity(double value)
	{
		//Note: sign bit returns true if negative value
		return std::isinf(value) && std::signbit(value);
	}

	int GetSign(double num)
	{
		if (ApproximateEquals(num, 0)) return 0;
		return num >= 0 ? 1 : -1;
	}

	inline bool IsNumber(char c)
	{
		return std::isdigit(c);
	}

	inline bool IsLetter(char c)
	{
		return std::isalpha(c);
	}

	inline bool IsLetterOrNumber(char c)
	{
		return std::isalnum(c);
	}

	float Roundf(const float& decimal, const std::uint8_t& places)
	{
		float factor = std::pow(10.0f, places);
		return std::round(decimal * factor) / factor;
	}

	std::string ToString(const double& decimal)
	{
		//This trick will esssnetially display only sig fits when used without std::to_string
		return std::format("{}", decimal);
	}

	size_t GetDigitPlaces(const double& decimal)
	{
		std::string sigFigStr = ToString(decimal);
		if (sigFigStr.find(".") != std::string::npos) return sigFigStr.size() - 1;
		return sigFigStr.size();
	}

	size_t GetDecimalPlaces(const double& decimal)
	{
		std::string sigFigStr = ToString(decimal);
		size_t decimalPos = sigFigStr.find(".");

		if (decimalPos == std::string::npos) return 0;
		return sigFigStr.size() - decimalPos - 1;
	}

	int GenerateRandomInt(int minInclusive, int maxInclusive)
	{
		std::random_device rd;
		std::mt19937 engine(rd());
		std::uniform_int_distribution<int> dist(minInclusive, maxInclusive);
		return dist(engine);
	}

	double GenerateRandomDouble(double minInclusive, double maxExclusive)
	{
		std::random_device rd;
		std::mt19937 engine(rd());
		std::uniform_real_distribution<double> dist(minInclusive, maxExclusive);
		return dist(engine);
	}

	float MinAbs(const float& num1, const float& num2)
	{
		if (std::abs(num1) < std::abs(num2)) return num1;
		return num2;
	}

	std::string CollapseToSingleString(const std::vector<std::string>& vec)
	{
		return std::accumulate(vec.begin(), vec.end(), std::string());
	}

	std::string ToString(const char& c)
	{
		return std::string(1, c);
	}

	std::string ToStringDouble(const double& d, const std::streamsize& precision, const bool& decimalPlacePrecision)
	{
		std::ostringstream oss;
		std::streamsize finalPrecision = precision;
		//If we do deciaml precision we must add the non-decimal places since by default
		//string stream precision meants total number of sig figs
		if (decimalPlacePrecision)
		{
			finalPrecision += (GetDigitPlaces(d) - GetDecimalPlaces(d));
			//Assert(false, std::format("Trying to:{} prec:{} actual:{}", std::to_string(d), std::to_string(precision), std::to_string()));
		}
		
		oss.precision(finalPrecision);
		oss << d;
		return oss.str();
	}

	bool HasFlag(unsigned int fullFlag, unsigned int hasFlag)
	{
		return (fullFlag & hasFlag) != 0;
	}

	bool ExecuteIfTrue(const std::function<void()>& function, const std::function<bool()>& predicate)
	{
		bool executeFunc = predicate();
		if (executeFunc) function();
		return executeFunc;
	}

	bool ExecuteIfTrue(const std::function<void()>& function, const bool condition)
	{
		return ExecuteIfTrue(function, [&condition]() -> bool {return condition; });
	}

	bool ExecuteFromCondition(const std::function<bool()>& predicate,
		const std::function<void()>& trueFunc, const std::function<void()>& falseFunc)
	{
		bool isTrue = predicate();
		isTrue ? trueFunc() : falseFunc();
		return isTrue;
	}

	bool ExecuteFromCondition(const bool condition, const std::function<void()>& trueFunc,
		const std::function<void()>& falseFunc)
	{
		return ExecuteFromCondition([&condition]()-> bool {return condition; }, trueFunc, falseFunc);
	}
}