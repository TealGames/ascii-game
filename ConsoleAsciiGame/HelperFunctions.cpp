#include "pch.hpp"

#include <cctype>
#include <numbers>
#include <numeric>
#include <cmath>
#include <limits>
#include <fstream>
#include <filesystem>
#include <random>
#include "HelperFunctions.hpp"
#include "StringUtil.hpp"

#ifdef LOG_WX_WIDGETS
#include <wx/wx.h>
#endif

namespace Utils
{
	constexpr bool IsCurrentVersion(const CPPVersion& version)
	{
		return __cplusplus == static_cast<long>(version);
	}

	const std::string LOG_ONLY_MESSAGE = "";
	const std::optional<LogType> LOG_ONLY_TYPE = std::nullopt;
	const bool LOG_MESSAGES = true;

	static const std::string ANSI_COLOR_ERROR = "\033[1;31m";
	static const std::string ANSI_COLOR_WARNING = "\033[1;33m";
	static const std::string ANSI_COLOR_DEFAULT = "\033[1;37m";
	static const std::string ANSI_COLOR_CLEAR = "\033[0m";

	void Log(const LogType& logType, const std::string& str)
	{
		if (!LOG_MESSAGES) return;
		if (LOG_ONLY_TYPE.has_value() && logType != LOG_ONLY_TYPE) return;

		if (!LOG_ONLY_MESSAGE.empty() &&
			str.substr(0, LOG_ONLY_MESSAGE.size()) != LOG_ONLY_MESSAGE) return;

		std::string logTypeMessage;
		switch (logType)
		{
		case LogType::Error:
			logTypeMessage = std::format("{}[{}!{}]{} ERROR:", ANSI_COLOR_DEFAULT, ANSI_COLOR_ERROR, ANSI_COLOR_DEFAULT, ANSI_COLOR_ERROR);
			break;
		case LogType::Warning:
			logTypeMessage = std::format("{}[{}!{}]{} WARNING:", ANSI_COLOR_DEFAULT, ANSI_COLOR_WARNING, ANSI_COLOR_DEFAULT, ANSI_COLOR_WARNING);
			break;
		case LogType::Log:
			logTypeMessage = std::format("{}LOG:", ANSI_COLOR_DEFAULT);
			break;
		default:
			std::string errMessage = "Tried to log message of message type "
				"that is not defined: ";
			Log(LogType::Error, errMessage);
			return;
		}
		std::string fullMessage = "\n" + logTypeMessage + str +ANSI_COLOR_CLEAR;

#ifdef LOG_WX_WIDGETS
		wxLogMessage(fullMessage.c_str());
#else
		std::cout << fullMessage << std::endl;
#endif
	}

	void Log(const std::string& str)
	{
		Log(LogType::Log, str);
	}

	bool Assert(const bool condition, const std::string& errMessage)
	{
		if (!condition) Utils::Log(LogType::Error, errMessage);
		return condition;
	}

	void ClearSTDCIN()
	{
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
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
			Utils::Log(Utils::LogType::Error, err);
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
			Utils::Log(Utils::LogType::Error, err);
			return;
		}

		file << content;
		file.close();
	}
}