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
#include "HelperFunctions.hpp"
#include "StringUtil.hpp"

namespace Utils
{
	inline static const std::string LOG_ONLY_MESSAGE = "";
	inline static constexpr LogType LOG_MESSAGE_TYPES = LogType::Log | LogType::Error | LogType::Warning;
	inline constexpr bool LOG_MESSAGES = true;

	inline static const char* ANSI_COLOR_ERROR = "\033[1;31m";
	inline static const char* ANSI_COLOR_WARNING = "\033[1;33m";
	inline static const char* ANSI_COLOR_DEFAULT = "\033[1;37m";

	constexpr bool IsCurrentVersion(const CPPVersion& version)
	{
		return __cplusplus == static_cast<long>(version);
	}

	void Log(const LogType& logType, const std::string& str, const bool& logTime)
	{
		if (!LOG_MESSAGES) return;
		//if ((LOG_MESSAGE_TYPES & logType) != LogType::None) return;
		if (!HasFlagAny(LOG_MESSAGE_TYPES, logType)) return;

		if (!LOG_ONLY_MESSAGE.empty() &&
			str.substr(0, LOG_ONLY_MESSAGE.size()) != LOG_ONLY_MESSAGE) return;

		std::string logTypeMessage;
		std::string timeFormatted = "";
		if (logTime) timeFormatted = std::format("{}[{}{}{}]{}", ANSI_COLOR_DEFAULT, ANSI_COLOR_GRAY, FormatTime(GetCurrentTime()), ANSI_COLOR_DEFAULT, ANSI_COLOR_CLEAR);

		switch (logType)
		{
		case LogType::Error:
			logTypeMessage = std::format("{}[{}!{}] {} {}ERROR:", ANSI_COLOR_DEFAULT,
				ANSI_COLOR_ERROR, ANSI_COLOR_DEFAULT, timeFormatted, ANSI_COLOR_ERROR);
			break;
		case LogType::Warning:
			logTypeMessage = std::format("{}[{}!{}] {} {}WARNING:", ANSI_COLOR_DEFAULT,
				ANSI_COLOR_WARNING, ANSI_COLOR_DEFAULT, timeFormatted, ANSI_COLOR_WARNING);
			break;
		case LogType::Log:
			logTypeMessage = std::format("{} {}LOG:", timeFormatted, ANSI_COLOR_DEFAULT);
			break;
		default:
			std::string errMessage = "Tried to log message of message type "
				"that is not defined: ";
			Log(LogType::Error, errMessage);
			return;
		}
		std::string fullMessage = "\n" + logTypeMessage + str + ANSI_COLOR_CLEAR;
		std::cout << fullMessage << std::endl;
	}

	void Log(const std::string& str, const bool& logTime)
	{
		Log(LogType::Log, str, logTime);
	}
	void LogWarning(const std::string& str, const bool& logTime)
	{
		Log(LogType::Warning, str, logTime);
	}
	void LogError(const std::string& str, const bool& logTime)
	{
		Log(LogType::Error, str, logTime);
	}


	bool Assert(const bool condition, const std::string& errMessage)
	{
		if (!condition) LogError(errMessage);
		return condition;
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

	void ClearSTDCIN()
	{
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
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

	std::string ToStringDouble(const double& d, const std::streamsize& precision)
	{
		std::ostringstream oss;
		oss.precision(precision);
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

	std::filesystem::path CleanPath(const std::filesystem::path& path)
	{
		Utils::StringUtil cleaner(path.string());
		const std::string cleaned = cleaner.Trim().ToString();
		return cleaned;
	}

	bool DoesPathExist(const std::filesystem::path& path)
	{
		if (!std::filesystem::exists(path))
		{
			std::string err = std::format("File path {} does not exist", path.string());
			Log(LogType::Error, err);
			return false;
		}
		return true;
	}

	std::string ReadFile(const std::filesystem::path& path)
	{
		const std::filesystem::path cleanedPath = CleanPath(path);
		if (!DoesPathExist(cleanedPath)) return "";

		std::string content;
		std::ifstream file(cleanedPath);
		if (!file.is_open())
		{
			std::string err = std::format("Tried to read file at path {} but it could not be opened",
				cleanedPath.string());
			Log(Utils::LogType::Error, err);
			return "";
		}

		std::string line;
		while (std::getline(file, line))
		{
			content += line;
		}
		file.close();
		return content;
	}

	void WriteFile(const std::filesystem::path& path, const std::string content)
	{
		const std::filesystem::path cleanedPath = CleanPath(path);
		if (!DoesPathExist(cleanedPath)) return;

		std::ofstream file(cleanedPath);
		if (!file.is_open())
		{
			std::string err = std::format("Tried to write {} to file at path {} "
				"but it could not be opened", content, cleanedPath.string());
			Log(Utils::LogType::Error, err);
			return;
		}

		file << content;
		file.close();
	}
}