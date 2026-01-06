#include "pch.hpp"
#include <cctype>
#include <numbers>
#include <numeric>
#include <cmath>
#include <chrono>
#include <limits>
#include <fstream>
#include <filesystem>
#include "Utils/HelperFunctions.hpp"
#include "AnsiCodes.hpp"
#include "Utils/Math.hpp"

#if _HAS_CXX23
#include <stacktrace>
#endif

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

	std::string GetCurrentStackTrace() 
	{
#if !_HAS_CXX23
		throw std::invalid_argument("Attempted to get current stack trace on a C++ version that does not support it (Need C++23)");
		return "[INVALID OPERATION StackStrace needs C++23]";
#else
		std::ostringstream stream;
		stream << std::stacktrace::current();
		return stream.str();
#endif
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
			//std::cout << "DOING C:" << Utils::ToString(c);
			if (!std::isdigit(c))
			{
				result += c;
			}
		}
		return result;
	}
	std::string TryExtractHexadecimal(const std::string& input) 
	{
		std::string result = "";
		char lowerChar = '0';
		const char a = 'a';
		const char f = 'f';

		for (const auto& c : input)
		{
			lowerChar = std::tolower(c);
			if (std::isdigit(lowerChar) || (lowerChar >= a && lowerChar <= f))
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
	std::string GetDiff(const std::string& originalStr, const std::string& newStr)
	{
		//TODO: implement
		return "DIFF_NOT_IMPLEMENTED";

		size_t i = 0;
		size_t j = 0;
		while (i < originalStr.size() && j < newStr.size())
		{
			char ci = originalStr[i];
			char cj = originalStr[j];
		}
	}

	bool IsNumber(char c)
	{
		return std::isdigit(c);
	}

	bool IsLetter(char c)
	{
		return std::isalpha(c);
	}

	bool IsLetterOrNumber(char c)
	{
		return std::isalnum(c);
	}

	int GenerateRandomInt(int minInclusive, int maxExclusive)
	{
		return GenerateRandomIntNum<int>(minInclusive, maxExclusive);
	}
	double GenerateRandomDouble(double minInclusive, double maxExclusive)
	{
		return GenerateRandomRealNum<double>(minInclusive, maxExclusive);
	}
	float GenerateRandomFloat(float minInclusive, float maxExclusive)
	{
		return GenerateRandomRealNum<float>(minInclusive, maxExclusive);
	}

	float MinAbs(const float& num1, const float& num2)
	{
		if (std::abs(num1) < std::abs(num2)) return num1;
		return num2;
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