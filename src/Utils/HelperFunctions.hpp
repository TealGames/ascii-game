#pragma once
#include "Utils/StringUtil.hpp"
#include "Utils/TemplateConcepts.hpp"
#include <string>
#include <string_view>
#include <format>
#include <functional>
#include <sstream>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <cstdint>
#include <filesystem>
#include <utility>
#include <array>
#include <random>


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


	template <std::size_t...Idxs>
	constexpr auto GetSubstringAsArray(std::string_view str, std::index_sequence<Idxs...>)
	{
		return std::array{ str[Idxs]..., '\n' };
	}

	/*template <typename T>
	constexpr auto GetTypeNameArray()
	{
#if defined(__clang__)
		constexpr auto prefix = std::string_view{ "[T = " };
		constexpr auto suffix = std::string_view{ "]" };
		constexpr auto function = std::string_view{ __PRETTY_FUNCTION__ };
#elif defined(__GNUC__)
		constexpr auto prefix = std::string_view{ "with T = " };
		constexpr auto suffix = std::string_view{ "]" };
		constexpr auto function = std::string_view{ __PRETTY_FUNCTION__ };
#elif defined(_MSC_VER)
		constexpr auto prefix = std::string_view{ "GetTypeNameArray<" };
		constexpr auto suffix = std::string_view{ ">(void)" };
		constexpr auto function = std::string_view{ __FUNCSIG__ };
#else
# error Unsupported compiler
#endif

		constexpr auto start = function.find(prefix) + prefix.size();
		constexpr auto end = function.rfind(suffix);

		static_assert(start < end);

		constexpr auto name = function.substr(start, (end - start));
		return GetSubstringAsArray(name, std::make_index_sequence<name.size()>{});
	}

	template <typename T>
	struct TypeNameHolder
	{
		static inline constexpr auto value = GetTypeNameArray<T>();
	};*/

	template <typename T, typename... Types>
	concept MatchesAtLeastOneType = std::disjunction_v<std::is_same<T, Types>...>;

	template <typename T, typename Variant, std::size_t... I>
	constexpr bool IsInVariantImpl(std::index_sequence<I...>) 
	{
		return (std::is_same_v<T, typename std::variant_alternative<I, Variant>::type> || ...);
	}

	template <typename T, typename Variant>
	concept InVariant = IsInVariantImpl<T, Variant>(std::make_index_sequence<std::variant_size_v<Variant>>{});

	std::string GetCurrentStackTrace();

	using SystemTime = std::chrono::time_point<std::chrono::system_clock>;
	using LocalTime= std::chrono::zoned_time<std::chrono::system_clock::duration>;
	LocalTime GetLocalTime(const SystemTime& time);
	LocalTime GetCurrentTime();

	/// <summary>
	/// Returns true if ANY `checkFlag` exists in `flagEnum`
	/// </summary>
	/// <typeparam name="TEnum"></typeparam>
	/// <typeparam name="...CheckFlagType"></typeparam>
	/// <param name="enumBits"></param>
	/// <param name="...checkFlags"></param>
	/// <returns></returns>
	template<typename TEnum, typename... CheckFlagType>
	requires std::is_enum_v<TEnum> && std::is_integral_v<std::underlying_type_t<TEnum>>
			 && HasBitwiseAnd<TEnum> && HasBitwiseOr<TEnum> &&
			 AllSameType<TEnum, CheckFlagType...> && HasAtLeastOneArg<CheckFlagType...>
	constexpr bool HasFlagAny(const TEnum flagEnum, const CheckFlagType... checkFlags)
	{
		TEnum flagsCombined = (checkFlags | ...);
		return (flagEnum & flagsCombined) != static_cast<TEnum>(0);
	}
	
	/// <summary>
	/// Returns true if ALL `checkFlags` exist in `flagEnum`
	/// </summary>
	/// <typeparam name="TEnum"></typeparam>
	/// <typeparam name="...CheckFlagType"></typeparam>
	/// <param name="enumBits"></param>
	/// <param name="...checkFlags"></param>
	/// <returns></returns>
	template<typename TEnum, typename... CheckFlagType>
	requires std::is_enum_v<TEnum> && std::is_integral_v<std::underlying_type_t<TEnum>>
		     && HasBitwiseAnd<TEnum> && HasBitwiseOr<TEnum> &&
			 AllSameType<TEnum, CheckFlagType...> && HasAtLeastOneArg<CheckFlagType...>
	constexpr bool HasFlagAll(const TEnum flagEnum, const CheckFlagType... checkFlags)
	{
		TEnum flagsCombined = (checkFlags | ...);
		return (flagEnum & flagsCombined) == flagsCombined;
	}

	bool HasFlag(unsigned int fullFlag, unsigned int hasFlag);

	template<typename TEnum>
	requires std::is_enum_v<TEnum> &&
			 std::is_integral_v<std::underlying_type_t<TEnum>> && HasBitwiseOr<TEnum>
	constexpr void AddFlags(TEnum& enumBits, const TEnum addFlags)
	{
		enumBits = enumBits | addFlags;
	}
	template<typename TEnum>
	requires std::is_enum_v<TEnum> && std::is_integral_v<std::underlying_type_t<TEnum>>
			 && HasBitwiseAnd<TEnum> && HasBitwiseNot<TEnum>
	constexpr void RemoveFlags(TEnum& enumBits, const TEnum removeFlags)
	{
		enumBits = enumBits & ~removeFlags;
	}

	template<typename TEnum>
	requires std::is_enum_v<TEnum>&& std::is_integral_v<std::underlying_type_t<TEnum>>
	constexpr TEnum SetAllFlags()
	{
		return static_cast<TEnum>(static_cast<std::underlying_type_t<TEnum>>(~0));
	}

	template <typename Variant>
	auto GetVariantValue(const Variant& variant) 
	{
		return std::visit([](auto&& value) -> decltype(value) 
			{
				return std::get_if<std::decay_t<decltype(value)>>(&variant);
			}, variant);
	}

	void ClearSTDCIN();

	bool ContainsIntegralValues(const std::string& input, const bool includeNegativeSign=false);
	size_t GetFirstIngteralValueIndex(const std::string& input, const bool includeNegativeSign = false);
	/// <summary>
	/// Will extract all of the integers in the input string as a string containing the int
	/// Note: this behavior is NOT the same as std::stoi which would find the first integer 
	/// (or throw if first char is a non-numeric value)
	/// This function behavior Examples:
	/// "1234" -> "1234"
	/// "12Hello34" -> "1234"
	/// "e1e2e3e4e" -> "1234"
	/// </summary>
	/// <param name="input"></param>
	/// <returns></returns>
	std::string TryExtractInt(const std::string& input);

	/// <summary>
	/// Will extract the float value in the input string as a string containing it
	/// This function behavior Examples:
	/// "12.34" -> "12.34"
	/// "12.Hello34" -> "12.34"
	/// "12.3.4"-> "12.34"
	/// </summary>
	/// <param name="input"></param>
	/// <returns></returns>
	std::string TryExtractFloat(const std::string& input);
	std::string TryExtractNonIntegralValues(const std::string& input);
	std::string TryExtractHexadecimal(const std::string& input);

	std::vector<std::string> Split(const std::string& str, const char& separator);
	std::string GetDiff(const std::string& originalStr, const std::string& newStr);

	template<typename T>
	inline bool IsCollectionIterable(const T& collection)
	{
		return IsIterable<T>();
	}

	/// <summary>
	///	This function means it will return bool if this function is iterable
	/// </summary>
	/// <typeparam name="T1"></typeparam>
	/// <typeparam name="T2"></typeparam>
	/// <param name="collection"></param>
	/// <param name="findElement"></param>
	/// <returns></returns>
	template <typename T1, typename T2>
	requires IsIterable<T1>
	bool IterableHas(const T1& collection, const T2& findElement)
	{
		if (collection.size() <= 0) 
			return false;

		auto startElement = collection.begin();
		if (!std::is_same_v<T2, decltype(startElement)>)
			return false;

		auto endElement = collection.end();

		//If they are the same, this means the size is 0
		if (endElement == startElement) 
			return false;

		auto result = std::find(startElement, endElement, findElement);
		return result != endElement;
	}

	template <typename T1, typename T2>
	requires IsIterable<T1>
	int GetIndexOfValue(const T1& collection, const T2& findElement)
	{
		if (collection.size() <= 0) 
			return -1;

		auto startElement = collection.begin();
		if (!std::is_same_v<T1, decltype(startElement)>) 
			return -1;

		auto endElement = collection.end();
		//If they are the same, this means the size is 0
		if (endElement == startElement) 
			return -1;

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
			throw std::invalid_argument(err);
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

	template <typename T, std::size_t... Is, typename... Args>
	constexpr std::array<T, sizeof...(Is)> ConstructArrayImpl(std::index_sequence<Is...>, Args&&... args) 
	{
		// Expand the pack N times by repeating construction with the same args
		return { ((void)Is, T(std::forward<Args>(args)...))... };
	}

	template <typename T, std::size_t N, typename... Args>
	constexpr std::array<T, N> ConstructArray(Args&&... args) 
	{
		return ConstructArrayImpl<T>(std::make_index_sequence<N>{}, std::forward<Args>(args)...);
	}

	bool IsNumber(char);
	bool IsLetter(char);
	bool IsLetterOrNumber(char);

	int GenerateRandomInt(int minInclusive, int maxInclusive);
	double GenerateRandomDouble(double minInclusive, double maxExclusive);
	float GenerateRandomFloat(float minInclusive, float maxExclusive);

	template<typename T>
	requires std::is_floating_point_v<T>
	T GenerateRandomRealNum(const T& minInclusive, const T& maxExclusive)
	{
		std::random_device rd;
		std::mt19937 engine(rd());
		std::uniform_real_distribution<T> dist(minInclusive, maxExclusive);
		return dist(engine);
	}
	template<typename T>
	requires std::is_integral_v<T>
	T GenerateRandomIntNum(const T& minInclusive, const T& maxExclusive)
	{
		std::random_device rd;
		std::mt19937 engine(rd());
		std::uniform_int_distribution<T> dist(minInclusive, maxExclusive);
		return dist(engine);
	}

	/// <summary>
	/// Will returns the minimum value considering their ABSOLUTE VALUE (disregarding sign)
	///	The returned value WILL include the sign as given by argument
	/// </summary>
	/// <param name="num1"></param>
	/// <param name="num2"></param>
	/// <returns></returns>
	float MinAbs(const float& num1, const float& num2);

	constexpr std::uint8_t DEFAULT_PARSE_BASE = 10;
	template<typename T>
	std::optional<T> TryParse(const std::string& str, const std::uint8_t& base= DEFAULT_PARSE_BASE)
	{
		const std::type_info& tType = typeid(T);

		/*if (tType == typeid(unsigned int) || tType == typeid(uint16_t))
		{
			std::string error = std::format("Tried to parse string {} to unsigned int or uint16_t, "
				"which is not supported in C++ STL (there is no stoui function)", str);
			Log(LogType::Error, error);
			return std::nullopt;
		}*/
		if (tType == typeid(long))
		{
			const std::string error = std::format("Tried to parse string {} to long, "
				"which is not supported since most systems only guarantee 64 "
				"bits with long long (use long long or uint64_t instead)", str);
			throw std::invalid_argument(error);
		}
		/*if (tType== typeid(unsigned int) || tType == typeid(unsigned long) || tType == typeid(unsigned long long))
		{
			const std::string error = std::format("Tried to parse string {} to unsigned long, unsigned int or "
				"unsigned long long, which is not supported since uint32_t, uint64_t and uint128_t "
				"offer better platform independence", str);
			Log(LogType::Error, error);
			return std::nullopt;
		}*/

		if (std::is_floating_point_v<T> && base != DEFAULT_PARSE_BASE)
		{
			const std::string err= std::format("Tried to parse string {} to floating point precision type "
				"with base:{} that does not equal the default base; {}, which is not supported",
				str, std::to_string(base), std::to_string(DEFAULT_PARSE_BASE));
			throw std::invalid_argument(err);
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
				throw std::invalid_argument(error);
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
			throw std::invalid_argument(err);
		}
		return parsedVal;
	}

	template<typename T>
	std::optional<T> TryParseView(const std::string_view& str, const std::uint8_t& base = DEFAULT_PARSE_BASE)
	{
		T convertedT;
		auto [parseFailPtr, result] = std::from_chars(str.data(), str.data() + str.size(), convertedT);
		if (result == std::errc{}) return convertedT;
		else return std::nullopt;
	}

	template<typename T>
	std::optional<T> TryParseHex(const std::string& str)
	{
		std::optional<T> parsedVal = std::nullopt;
		if (std::is_signed_v<T>)
		{
			std::string error = std::format("Tried to parse hex string {} but signed "
				"integer values are not supported", str);
			throw std::invalid_argument(error);
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
}