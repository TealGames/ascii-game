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

	inline constexpr bool INCLUDE_SPACES_BETWEEN_ELEMENTS = true;

	/// <summary>
	/// Converts a floating point number to string using fast to_chars() function
	/// This version uses fixed decimal point numbers and should NOT be used for serialization
	/// as round trip property requires a high amount of decimal places. 
	/// </summary>
	/// <param name="d"></param>
	/// <param name="decimalPlaces"></param>
	/// <returns></returns>
	std::string ToString(const double d, const std::uint8_t decimalPlaces);
	std::string ToStringRoundTrip(const double d);
	std::string ToStringRoundTrip(const float f);

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
			{
				str += ',';
				if constexpr (INCLUDE_SPACES_BETWEEN_ELEMENTS)
					str += ' ';
			}
			index++;
		}
		str += "]";
		return std::format("(Size:{}) ", collection.size()) + str;
	}

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