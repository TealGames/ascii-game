#pragma once
#include <string>
#include <format>
#include <functional>
#include <sstream>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <cstdint>
#include <filesystem>
#include <type_traits>

namespace Utils
{
	enum class CPPVersion : long
	{
		CPP11 = 201103L,
		CPP14 = 201402L,
		CPP17 = 201703L,
		CPP20 = 202002L,
	};
	constexpr bool IsCurrentVersion(const CPPVersion& version);

	//This is the fallback in case we supply incorrect type args
	template <typename, typename T>
	struct HasFunction
	{
		static_assert(std::integral_constant<T, false>::value,
			"Second template parameter needs to be of function type.");
	};

	template <typename C, typename Return, typename... Args>
	struct HasFunction<C, Return(Args...)>
	{
	private:
		//NOte: -> means return, which is mainly used for generic programming
		//This will check if the method were called would return the neccessary type
		//and if it does will store true value, otherwise will fallback to other template
		template <typename T>
		static constexpr auto CheckExists(int arg)
			-> typename std::is_same<decltype(std::declval<T>().
				method(std::declval<Args>()...)), Return>::type;

		//This is a fallback in case it does not exist, creating a false value
		//Note: ... mean any arguments
		template <typename>
		static constexpr std::false_type CheckExists(...);

		//This is where the result of true or false is stored by
		//evaluating the type of CheckExists with 0 (the value does not matter since
		//we only really need to have any args so we can first check the true function)
		typedef decltype(CheckExists<C>(0)) type;

	public:
		static constexpr bool VALUE = type::value;
	};

	enum class LogType
	{
		Error,
		Warning,
		Log,
	};

	void Log(const LogType& logType, const std::string& str);

	/// <summary>
	/// Logs a message as a default LOG type
	/// </summary>
	/// <param name="str"></param>
	void Log(const std::string& str);

	/// <summary>
	/// The same Log action but also logs the class
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="objPtr"></param>
	/// <param name="logType"></param>
	/// <param name="str"></param>
	template<typename T>
	void Log(const T* const objPtr, const LogType& logType, const std::string& str)
	{
		Log(logType, std::format("{}: {}", typeid(T).name(), str));
	}

	/// <summary>
	/// If condition is false will log error and return false, othersise return true and does nothing
	/// </summary>
	bool Assert(const bool condition, const std::string& errMessage);

	template<typename T>
	bool Assert(const T* const objPtr, const bool condition, const std::string& errMessage)
	{
		if (!condition) Utils::Log<T>(objPtr, LogType::Error, errMessage);
		return condition;
	}

	template <typename T>
	constexpr bool IS_NUMERIC = std::is_arithmetic_v<T>;

	template <typename T, typename = void>
	struct HasMemberToString : std::false_type {};
	template <typename T>
	struct HasMemberToString<T, std::void_t<decltype(std::declval<T>().ToString())>> : std::true_type {};

	template <typename T, typename = void>
	struct HasFreeToString : std::false_type {};
	template <typename T>
	struct HasFreeToString<T, std::void_t<decltype(ToString(std::declval<T>()))>> : std::true_type {};

	template <typename T, typename = void>
	struct HasOstreamOperator : std::false_type {};
	template <typename T>
	struct HasOstreamOperator<T, std::void_t<decltype(std::declval<std::ostringstream&>() << std::declval<T>())>> : std::true_type {};

	template <typename T>
	std::optional<std::string> TryToString(const T& obj)
	{
		//Arithmetic checks for ints and floats -> which are available with to_string
		if constexpr (IS_NUMERIC<T>)
		{
			return std::to_string(obj);
		}
		else if constexpr (HasMemberToString<T>::value)
		{
			return obj.ToString();
		}
		else if constexpr (HasFreeToString<T>::value) {
			return ToString(obj);
		}
		else if constexpr (HasOstreamOperator<T>::value) {
			std::ostringstream oss;
			oss << obj;
			return oss.str();
		}
		else {
			//throw std::invalid_argument("ToString: No suitable conversion available for the given type.");
			return std::nullopt;
		}
	}

	template <typename T, typename = void>
	struct IsIterableCheck : std::false_type {};

	//This gets used only when we can call std::begin() and std::end() on that type
	template <typename T>
	struct IsIterableCheck<T, std::void_t<decltype(std::begin(std::declval<T&>())),
		decltype(std::end(std::declval<T&>()))>> : std::true_type {};

	//Will return the value of the iterable check
	template <typename T>
	static constexpr bool IS_ITERABLE = IsIterableCheck<T>::value;

	template<typename T>
	inline bool IsIterable(const T& collection)
	{
		return IS_ITERABLE<T>();
	}

	/// <summary>
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="collection"></param>
	/// <param name="toStringFunction"></param>
	/// <returns></returns>
	template <typename TCollection, typename TElement>
	auto ToStringIterable(const TCollection& collection, const std::function<std::string(const TElement&)>
		toStringFunction = nullptr)
		-> typename std::enable_if<IS_ITERABLE<TCollection>, std::string>::type
	{
		bool hasOverrideToString = toStringFunction != nullptr;

		std::string str = "[";
		std::optional<std::string> elementStr = std::nullopt;
		int index = 0;
		for (const TElement element : collection)
		{
			if (hasOverrideToString) str += toStringFunction(element);
			else
			{
				if constexpr (std::is_pointer_v<decltype(element)>) elementStr = Utils::TryToString(*element);
				else elementStr = Utils::TryToString(element);

				if (elementStr.has_value()) str += elementStr.value();
				else return "[PARSE FAILED: element could not convert to string]";
			}

			if (index < collection.size() - 1)
				str += ", ";
			index++;
		}
		str += "]";
		return str;
	}

	template <typename TKey, typename TValue>
	std::string ToStringIterable(const std::unordered_map<TKey, TValue> collection,
		const std::function<std::string(const std::pair<TKey, TValue>&)> toStringFunction = nullptr)
	{
		bool hasOverrideToString = toStringFunction != nullptr;

		std::string str = "[";
		int index = 0;
		std::optional<std::string> maybeKeyStr = std::nullopt;
		std::optional<std::string> maybeValueStr = std::nullopt;
		for (const std::pair<TKey, TValue>& pair : collection)
		{
			if (hasOverrideToString) str += toStringFunction(pair);
			else
			{
				//TODO: what if the pair key or value is another unordered map??
				if constexpr (IS_ITERABLE<TKey>)
				{
					auto keyIt = pair.first.begin();
					if (pair.first.empty() || keyIt == pair.first.end()) maybeKeyStr = "{}";
					else maybeKeyStr = ToStringIterable<TKey, decltype(*keyIt)>(pair.first);
				}
				else if constexpr (std::is_pointer_v<decltype(pair.first)>)
					maybeKeyStr = Utils::TryToString(*(pair.first));
				else maybeKeyStr = Utils::TryToString(pair.first);

				if constexpr (IS_ITERABLE<TValue>)
				{
					auto valueIt = pair.second.begin();
					if (pair.second.empty() || valueIt == pair.second.end()) maybeValueStr = "{}";
					else maybeValueStr = ToStringIterable<TValue, decltype(*valueIt)>(pair.second);
				}
				else if constexpr (std::is_pointer_v<decltype(pair.second)>)
					maybeValueStr = Utils::TryToString(*(pair.second));
				else maybeValueStr = Utils::TryToString(pair.second);

				str += std::format("({},{})", maybeKeyStr.has_value() ? maybeKeyStr.value() : "[INVALID KEY: could not stringify]",
					maybeValueStr.has_value() ? maybeValueStr.value() : "[INVALID VALUE: could not stringify]");
			}

			if (index < collection.size() - 1)
				str += ", ";
			index++;
		}
		str += "]";
		return str;
	}

	template<typename T>
	std::string ToStringEnum()
	{
		const bool isEnum = std::is_enum<T>::value;
		if (!isEnum)
		{
			const std::string err = std::format();
			Utils::Log(Utils::LogType::Error, err);
			return "";
		}
	}

	std::string ToString(const char& c);

	/// <summary>
	///	This function means it will return bool if this function is iterable
	/// </summary>
	/// <typeparam name="T1"></typeparam>
	/// <typeparam name="T2"></typeparam>
	/// <param name="collection"></param>
	/// <param name="findElement"></param>
	/// <returns></returns>
	template <typename T1, typename T2>
	auto IterableHas(const T1& collection, const T2& findElement)
		-> typename std::enable_if<IS_ITERABLE<T1>, bool>::type
	{
		if (collection.size() <= 0) return false;

		auto startElement = collection.begin();
		if (!std::is_same_v<T2, decltype(startElement)>) return -1;

		auto endElement = collection.end();

		//If they are the same, this means the size is 0
		if (endElement == startElement) return false;

		auto result = std::find(startElement, endElement, findElement);
		return result != endElement;
	}

	template <typename T1, typename T2>
	auto GetIndexOfValue(const T1& collection, const T2& findElement)
		-> typename std::enable_if<IS_ITERABLE<T1>, size_t>::type
	{
		if (collection.size() <= 0) return -1;

		auto startElement = collection.begin();
		if (!std::is_same_v<T1, decltype(startElement)>) return -1;

		auto endElement = collection.end();
		//If they are the same, this means the size is 0
		if (endElement == startElement) return -1;

		auto findElementIt = std::find(startElement, endElement, findElement);
		return std::distance(startElement, findElementIt);
	}

	template<typename KType, typename VType>
	std::unordered_map<KType, VType> GetMapFromVectors(const std::vector<KType>& keys,
		const std::vector<VType> vals)
	{
		std::unordered_map<KType, VType> result;
		if (keys.size() != vals.size())
		{
			std::string err = std::format("Tried to get map from vectors but sizes do not match."
				"Argument 1 size: {}, Argument 2 size : {}", keys.size(), vals.size());
			Utils::Log(Utils::LogType::Error, err);
			return result;
		}

		for (size_t i = 0; i < keys.size(); i++)
		{
			if (i < vals.size()) result.emplace(keys[i], vals[i]);
		}
		return result;
	}

	template<typename KType, typename VType>
	std::vector<KType> GetKeysFromMap(const typename std::unordered_map<KType, VType>::const_iterator start,
		const typename std::unordered_map<KType, VType>::const_iterator end)
	{
		std::vector<KType> keys;
		for (auto it = start; it != end; it++)
		{
			keys.push_back(it->first);
		}
		return keys;
	}

	template<typename KType, typename VType>
	std::vector<VType> GetValuesFromMap(const typename std::unordered_map<KType, VType>::const_iterator start,
		const typename std::unordered_map<KType, VType>::const_iterator end)
	{
		std::vector<VType> values;
		for (auto it = start; it != end; it++)
		{
			values.push_back(it->second);
		}
		return values;
	}

	//By default the interesection methods in std require sorting so this is a custom version
	//Note: intersection is what both of them have in common
	template<typename T>
	std::unordered_set<T> GetUnorderedIntersection(const std::vector<T>& vec1, const std::vector<T>& vec2)
	{
		if (vec1.empty() || vec2.empty()) return {};

		const std::vector<T>& smallerVec = vec2.size() > vec1.size() ? vec1 : vec2;
		const std::vector<T>& largerVec = vec2.size() > vec1.size() ? vec2 : vec1;
		std::unordered_set<T> intersection;

		for (const auto& element : largerVec)
		{
			//The element must be in both vectors but can not already be in the intersection list
			if (std::find(smallerVec.begin(), smallerVec.end(), element) != smallerVec.end())
			{
				intersection.insert(element);
			}
		}
		return intersection;
	}

	//By default the interesection methods in std require sorting so this is a custom version
	//Note: intersection is what both of them have in common
	//Note: this is a more optimized version of the vector version since it does not require to check for duplicates
	template<typename T>
	std::unordered_set<T> GetUnorderedIntersection(const std::unordered_set<T>& vec1, const std::unordered_set<T>& vec2)
	{
		return GetUnorderedIntersection(std::vector<T>(vec1), std::vector<T>(vec2));
	}

	bool HasFlag(unsigned int fullFlag, unsigned int hasFlag);

	double ToRadians(const double);
	double ToDegrees(const double);
	bool ApproximateEquals(double, double);
	bool ApproximateEqualsF(float, float);

	//Returns the sign of the number, except for 0
	//example: 5 -> 1, -5 -> -1, 0 -> 0
	int GetSign(double);

	bool IsPosInifinity(double);
	bool IsNegInifinity(double);

	inline bool IsNumber(char);
	inline bool IsLetter(char);
	inline bool IsLetterOrNumber(char);

	int GenerateRandomInt(int minInclusive, int maxInclusive);
	double GenerateRandomDouble(double minInclusive, double maxExclusive);

	std::string CollapseToSingleString(const std::vector<std::string>& collection);

	constexpr std::uint8_t DEFAULT_PARSE_BASE = 10;
	template<typename T>
	std::optional<T> TryParse(const std::string& str, const std::uint8_t& base= DEFAULT_PARSE_BASE)
	{
		const std::type_info& tType = typeid(T);

		/*if (tType == typeid(unsigned int) || tType == typeid(uint16_t))
		{
			std::string error = std::format("Tried to parse string {} to unsigned int or uint16_t, "
				"which is not supported in C++ STL (there is no stoui function)", str);
			Utils::Log(Utils::LogType::Error, error);
			return std::nullopt;
		}*/
		if (tType == typeid(long))
		{
			const std::string error = std::format("Tried to parse string {} to long, "
				"which is not supported since most systems only guarantee 64 "
				"bits with long long (use long long or uint64_t instead)", str);
			Utils::Log(Utils::LogType::Error, error);
			return std::nullopt;
		}
		/*if (tType== typeid(unsigned int) || tType == typeid(unsigned long) || tType == typeid(unsigned long long))
		{
			const std::string error = std::format("Tried to parse string {} to unsigned long, unsigned int or "
				"unsigned long long, which is not supported since uint32_t, uint64_t and uint128_t "
				"offer better platform independence", str);
			Utils::Log(Utils::LogType::Error, error);
			return std::nullopt;
		}*/

		if (std::is_floating_point_v<T> && base != DEFAULT_PARSE_BASE)
		{
			const std::string err= std::format("Tried to parse string {} to floating point precision type "
				"with base:{} that does not equal the default base; {}, which is not supported",
				str, std::to_string(base), std::to_string(DEFAULT_PARSE_BASE));
			Utils::Log(Utils::LogType::Error, err);
			return std::nullopt;
		}

		T parsedVal;
		try
		{
			if (tType == typeid(short)) parsedVal = static_cast<short>(std::stoi(str, nullptr, base));
			else if (tType == typeid(int)) parsedVal = std::stoi(str, nullptr, base);
			else if (tType == typeid(long long)) parsedVal = std::stoll(str, nullptr, base);

			else if (tType == typeid(long double)) parsedVal = std::stold(str);
			else if (tType == typeid(float)) parsedVal = std::stof(str);
			else if (tType == typeid(double)) parsedVal = std::stod(str);
			else if (tType == typeid(long double)) parsedVal = std::stold(str);

			else if (tType == typeid(uint8_t)) parsedVal = static_cast<uint8_t>(std::stoul(str, nullptr, base));
			else if (tType == typeid(uint16_t)) parsedVal = static_cast<uint16_t>(std::stoul(str, nullptr, base));
			else if (tType == typeid(uint32_t)) parsedVal = static_cast<uint32_t>(std::stoul(str, nullptr, base));
			else if (tType == typeid(uint64_t)) parsedVal = static_cast<uint64_t>(std::stoull(str, nullptr, base));
			else
			{
				std::string error = std::format("Tried to parse string {} to type {} that can not "
					"be converted to string", str, tType.name());
				Utils::Log(Utils::LogType::Error, error);
				return std::nullopt;
			}
		}
		catch (const std::invalid_argument& e)
		{
			return std::nullopt;
		}
		catch (const std::out_of_range& e)
		{
			return std::nullopt;
		}
		catch (...)
		{
			std::string err = std::format("Tried to parse string {} to int but encountered unknwon error", str);
			Utils::Log(Utils::LogType::Error, err);
			return std::nullopt;
		}
		return parsedVal;
	}

	template<typename T>
	std::optional<T> TryParseHex(const std::string& str)
	{
		std::optional<T> parsedVal = std::nullopt;
		if (std::is_signed_v<T>)
		{
			std::string error = std::format("Tried to parse hex string {} but signed "
				"integer values are not supported", str);
			Utils::Log(Utils::LogType::Error, error);
			return std::nullopt;
		}

		parsedVal = TryParse<T>(str, 16);
		return parsedVal;
	}

	template<typename T>
	size_t GetTypeBitSize()
	{
		return sizeof(T) * 8;
	}

	bool ExecuteIfTrue(const std::function<void()>& function, const std::function<bool()>& predicate);
	bool ExecuteIfTrue(const std::function<void()>& function, const bool condition);

	bool ExecuteFromCondition(const std::function<bool()>& predicate,
		const std::function<void()>& trueFunc, const std::function<void()>& falseFunc);

	bool ExecuteFromCondition(const bool condition, const std::function<void()>&
		trueFunc, const std::function<void()>& falseFunc);

	template<typename Type1, typename Type2>
	bool AreSameType()
	{
		return std::is_same<Type1, Type2>::value;
	}

	template<typename T>
	bool IsValidPointer(const T* const* const ptr)
	{
		return ptr != nullptr && *ptr != nullptr;
	}

	template<typename T>
	bool IsValidPointer(const T* const ptr)
	{
		return ptr != nullptr;
	}

	std::string ReadFile(const std::filesystem::path& path);
	void WriteFile(const std::filesystem::path&, const std::string content);
}