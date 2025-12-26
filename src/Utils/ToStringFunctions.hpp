#pragma once
#include <string>
#include <vector>
#include <type_traits>
#include <sstream>
#include <optional>
#include <functional>
#include <chrono>
#include "Utils/TemplateConcepts.hpp"
#include "Utils/Data/Enums.hpp"
#include "Utils/HelperMacros.hpp"
#include "Utils/Debug.hpp"

namespace Utils
{
	inline constexpr const char* TREE_BRANCH_STR = "|-";
	inline constexpr const char* TREE_BRANCH_END_STR = "--";
	inline constexpr const char* TREE_VERTICAL_STR = "| ";

	std::string ToString(const double& d, const std::uint8_t decimalPlaces);
	std::string ToString(const char c);
	std::string ToString(const std::uint8_t u8);
	std::string ToString(const bool b);

	template<typename T>
	requires (!std::is_pointer_v<T>)
	std::string ToStringPointerAddress(const T* pointer)
	{
		if (pointer == nullptr) return "[NULL PTR ADDRESS]";

		std::ostringstream oss;
		oss << static_cast<const void*>(pointer);
		return oss.str();
	}

	std::string ToStringMemory(const std::byte* bytePtr, const std::size_t byteSize);
	std::string ToString(const std::exception& exception);
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

	template<typename T, typename... TStringFuncArgs>
	requires IsIterable<T>
	std::string ToStringIterable(const T& collection, const TStringFuncArgs&... toStringArgs);

	template<typename TKey, typename TValue, typename... TStringFuncArgs>
	std::string ToStringPair(const std::pair<TKey, TValue>& pair, const TStringFuncArgs&... toStringArgs);

	template<typename T, typename... TStringFuncArgs>
	requires IsPairType<T>
	std::string ToStringPairAuto(const T& pair, const TStringFuncArgs&... toStringArgs);

	template <typename T, typename... TStringFuncArgs>
	std::optional<std::string> TryToString(const T& obj, const TStringFuncArgs&... funcArgs)
	{
		//Arithmetic checks for ints and floats -> which are available with to_string
		if constexpr (std::is_same_v<T, std::string>)
			return obj;
		if constexpr (std::is_same_v<T, char> || std::is_same_v<T, bool> || Utils::IsExceptionType<T>)
			return ToString(obj);
		if constexpr (std::is_arithmetic_v<T>)
			return std::to_string(obj);
		if constexpr (std::is_pointer_v<decltype(obj)>)
			return TryToString(*obj, funcArgs...);
		if constexpr (HasNamedFunctionToString<T, std::string, TStringFuncArgs...>)
			return obj.ToString(funcArgs...);
		if constexpr (HasFreeNamedFunctionToString<std::string, TStringFuncArgs...>)
			return ToString(obj, funcArgs...);
		if constexpr (HasOstreamOperator<T>::value) 
		{
			std::ostringstream oss;
			oss << obj;
			return oss.str();
		}
		if constexpr (IsIterable<T>)
			return ToStringIterable(obj, funcArgs...);
		if constexpr (IsPairType<T>)
		{
			return ToStringPairAuto(obj, funcArgs...);
		}

		return std::nullopt;
	}
	template <typename T, typename... TStringFuncArgs>
	std::string ToStringForced(const T& obj, const TStringFuncArgs&... funcArgs)
	{
		return TryToString(obj, funcArgs...).value_or("[Stringify Failed]");
	}

	template<typename... TArgs>
	std::string ToStringTuple(const std::tuple<TArgs...>& tuple)
	{
		return std::apply([](const auto&... args) 
			{
				return ((ToStringForced(args) + " ") + ...);
			}, tuple);
	}

	template<typename... TArgs>
	std::string ToStringVariadicArgs(TArgs&&... args)
	{
		return ToStringTuple(std::tuple<TArgs...>(args...));
	}
	
	/// <summary>
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="collection"></param>
	/// <param name="toStringFunction"></param>
	/// <returns></returns>
	//template <typename TCollection, typename TElement>
	template<typename T, typename... TStringFuncArgs>
	requires IsIterable<T>
	std::string ToStringIterable(const T& collection, const TStringFuncArgs&... toStringArgs)
	{
		using Info = ContainerTypeInfo<T>;
		using ContainerTemplate = typename Info::ContainerTemplateType;
		using ElementType = typename Info::ElementType;

		std::string str = "[";
		std::optional<std::string> elementStr = std::nullopt;
		int index = 0;
		for (const auto& element : collection)
		{
			elementStr = Utils::TryToString(element, toStringArgs...);

			if (!elementStr.has_value())
				return "[Stringify FAILED]";
			
			str += elementStr.value();

			if (index < collection.size() - 1)
				str += ", ";
			index++;
		}
		str += "]";
		return std::format("(Size:{}) ", collection.size()) + str;
	}
	
	/*template<typename TKey, typename TValue, typename... TStringFuncArgs>
	requires IsIterable<std::unordered_map<TKey, TValue>>
	std::string ToStringIterable(const std::unordered_map<TKey, TValue>& collection, TStringFuncArgs&&... toStringArgs)
	{
		for (const auto& element : collection)
		{

		}
	}*/
	
	//template<typename TCollection, typename TElement>
	//std::string ToStringIterable(const TCollection& collection)
	//{
	//	return ToStringIterable<TCollection>(collection);
	//}
	//template<typename TKey, typename TValue>
	//std::string ToStringIterable(const std::unordered_map<TKey, TValue>& collection)
	//{
	//	return ToStringIterable<std::unordered_map<TKey, TValue>>(collection);
	//}

	//std::string ToStringIterable(const std::vector<std::string>& strings);

	/*
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
				if constexpr (Utils::IsUnorderedMapType<TKey>)
				{
					using KeyTypeInfo = Utils::UnorderedMapTypeInfo<std::remove_cv_t<TKey>>;
					maybeKeyStr = ToStringIterable<KeyTypeInfo::KeyType, KeyTypeInfo::ValueType>(pair.first);
				}
				else if constexpr (IsIterable<TKey>())
				{
					auto keyIt = pair.first.begin();
					maybeKeyStr = ToStringIterable<TKey, decltype(*keyIt)>(pair.first);
				}
				else if constexpr (std::is_pointer_v<decltype(pair.first)>)
					maybeKeyStr = Utils::TryToString(*(pair.first));
				else maybeKeyStr = Utils::TryToString(pair.first);

				if constexpr (Utils::IsUnorderedMapType<TValue>)
				{
					using ValueTypeInfo = Utils::UnorderedMapTypeInfo<std::remove_cv_t<TValue>>;
					maybeKeyStr = ToStringIterable<ValueTypeInfo::KeyType, ValueTypeInfo::ValueType>(pair.second);
				}
				else if constexpr (IsIterable<TValue>())
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
	*/

	template<typename TKey, typename TValue, typename... TStringFuncArgs>
	std::string ToStringPair(const std::pair<TKey, TValue>& pair, const TStringFuncArgs&... toStringArgs)
	{
		std::string keyStr = "";
		if constexpr (IsIterable<TKey>) keyStr = ToStringIterable(pair.first, toStringArgs...);
		else keyStr = Utils::TryToString(pair.first, toStringArgs...).value_or("");

		std::string valueStr = "";
		if constexpr (IsIterable<TValue>) valueStr = ToStringIterable(pair.second, toStringArgs...);
		else valueStr = Utils::TryToString(pair.second, toStringArgs...).value_or("");

		return std::format("[{},{}]", keyStr, valueStr);
	}

	template<typename T, typename... TStringFuncArgs>
	requires IsPairType<T>
	std::string ToStringPairAuto(const T& pair, const TStringFuncArgs&... toStringArgs)
	{
		using PairInfo = PairTypeInfo<std::remove_cv_t<T>>;
		return ToStringPair<typename PairInfo::KeyType, typename PairInfo::ValueType, TStringFuncArgs...>(pair, 
			toStringArgs...);
	}

	template<typename TKey, typename TValue>
	std::string ToStringKeyValue(const TKey& key, const TValue& value)
	{
		return std::format("[{},{}]", Utils::TryToString<TKey>(key).value_or("KEY STRINGIFY FAILED"), 
									  Utils::TryToString<TValue>(value).value_or("VAL STRINGIFY FAILED"));
	}

	template<typename TNode, typename... TStringFuncArgs>
	std::string ToStringTreeHelper(const TNode& node, const TNode* parentNode, std::string prefixStr,
		const std::function<const TNode*(const TNode& root, const size_t childIndex)>& getChildFunc, 
		const std::function<std::string(const TNode& node, const TNode* parentNode)> overrideToStringFunc,
		const TStringFuncArgs&... toStringArgs)
	{
		std::string resultStr = "";
		if (overrideToStringFunc != nullptr) resultStr = overrideToStringFunc(node, parentNode);
		else resultStr = TryToString(node, toStringArgs...).value_or("[FAILED STRINGIFY]");

		size_t i = 0;
		const TNode* currentNode = getChildFunc(node, i);
		bool isLastChild = getChildFunc(node, i + 1) == nullptr;
		while (currentNode != nullptr)
		{
			//LogWarning(std::format("Doing indices of next node: {} {} ", currentNode->m_IndexChild0, currentNode->m_IndexChild1));
			resultStr += "\n" + prefixStr + (isLastChild ? Utils::TREE_BRANCH_END_STR : Utils::TREE_BRANCH_STR) +
				ToStringTreeHelper(*currentNode, &node, prefixStr + (isLastChild ? "  " : Utils::TREE_VERTICAL_STR), 
					getChildFunc, overrideToStringFunc, toStringArgs...);
			if (isLastChild) break;

			i++;
			currentNode = getChildFunc(node, i);
			isLastChild = getChildFunc(node, i + 1) == nullptr;
		}
		return resultStr;
	}

	template<typename TNode>
	std::string ToStringTree(const TNode& root,
		const std::function<const TNode* (const TNode& root, const size_t childIndex)>& getChildFunc, 
		const std::function<std::string(const TNode& node, const TNode* parentNode)> overrideToStringFunc)
	{
		return ToStringTreeHelper<TNode>(root, nullptr, "", getChildFunc, overrideToStringFunc);
	}
}