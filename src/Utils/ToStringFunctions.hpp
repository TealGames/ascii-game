#pragma once
#include <string>
#include <vector>
#include <type_traits>
#include <sstream>
#include <optional>
#include <functional>
#include "Utils/TemplateConcepts.hpp"
#include <chrono>

namespace Utils
{
	std::string ToString(const double& d, const std::uint8_t decimalPlaces);
	std::string ToString(const char c);
	std::string ToString(const std::uint8_t u8);

	template<typename T>
	requires (!std::is_pointer_v<T>)
	std::string ToStringPointerAddress(const T* pointer)
	{
		if (pointer == nullptr) return "[NULL PTR ADDRESS]";

		std::ostringstream oss;
		oss << static_cast<const void*>(pointer);
		return oss.str();
	}

	std::string ToStringLeadingZeros(const int& number, const std::uint8_t& maxDigits);

	template <typename T>
	constexpr std::string ToStringTypeName()
	{
		if (typeid(T) == typeid(std::string)) return "string";

		return FormatTypeName(std::string(typeid(T).name()));
	}
	std::string FormatTypeName(const std::string& typeName);

	using LocalTime = std::chrono::zoned_time<std::chrono::system_clock::duration>;
	std::string ToStringTime(const LocalTime& time);

	std::string CollapseToSingleString(const std::vector<std::string>& collection);

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
		//else if constexpr (HasFreeToString<T>::value) {
		//	return ToString(obj);
		//}
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
		for (const TElement& element : collection)
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

	std::string ToStringIterable(const std::vector<std::string>& strings);

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
	std::string ToStringIterable(const std::vector<std::vector<T>>& vec2d, const bool newLineOnEveryRow = false)
	{
		std::vector<std::string> vecStrs = {};
		for (const auto& vec : vec2d)
		{
			vecStrs.push_back(newLineOnEveryRow ? "\n" : "" + Utils::ToStringIterable<std::vector<T>, T>(vec));
		}

		return Utils::ToStringIterable<std::vector<std::string>, std::string>(vecStrs);
	}

	template<typename KType, typename VType>
	std::string ToStringPair(const KType& key, const VType& value)
	{
		return std::format("[K:{}, V:{}]", Utils::TryToString<KType>(key).value_or(""),
			Utils::TryToString<VType>(value).value_or(""));
	}
}