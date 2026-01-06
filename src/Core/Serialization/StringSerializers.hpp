#pragma once
#include <string_view>
#include <string>
#include <vector>
#include <array>

namespace StringSerializers
{
	template<typename T>
	T Deserialize(const std::string_view& val);

	template<typename T, std::uint8_t N>
	requires (std::is_default_constructible_v<T> && 0 < N && N < 255)
	std::array<T, N> Deserialize(const std::string_view& val, const char separatorChar)
	{
		std::array<T, N> parsedArray = {};
		const char* currPtr = val.data();
		size_t currSize = 0;
		std::uint8_t arrI = 0;
		size_t i = 0;
		while (i < val.size())
		{
			if (val[i] == separatorChar)
			{
				parsedArray[arrI++] = Deserialize<T>(std::string_view(currPtr, currSize));
				currSize = 0;
				if (arrI >= N) break;

				if (i < val.size() - 1)
				{
					currPtr = &val[i + 1];
				}
			}
			else currSize++;
			i++;
		}

		if (currSize > 0 && arrI < N) 
			parsedArray[arrI] = Deserialize<T>(std::string_view(currPtr, currSize));

		return parsedArray;
	}
	template<typename T>
	requires (std::is_default_constructible_v<T>)
	std::vector<T> Deserialize(const std::string_view& val, const char separatorChar)
	{
		std::vector<T> parsedArray = {};
		const char* currPtr = val.data();
		size_t currSize = 0;
		size_t i = 0;
		while (i < val.size())
		{
			if (val[i] == separatorChar)
			{
				parsedArray.emplace_back(Deserialize<T>(std::string_view(currPtr, currSize)));
				currSize = 0;
				if (i < val.size() - 1)
				{
					currPtr = &val[i + 1];
				}
			}
			else currSize++;
			i++;
		}

		if (currSize > 0)
			parsedArray.emplace_back(Deserialize<T>(std::string_view(currPtr, currSize)));

		return parsedArray;
	}

	template<typename T>
	std::string Serialize(const T val);

	template<typename T, std::uint8_t N>
	requires (std::is_default_constructible_v<T> && 0 < N && N < 255)
	std::string Serialize(const T* arrPtr, const char separatorChar)
	{
		if (arrPtr == nullptr)
			return "";

		std::string result = Serialize<T>(arrPtr[0]);
		for (size_t i = 1; i < N; i++)
		{
			result += separatorChar + Serialize<T>(arrPtr[i]);
		}
		return result;
	}

	template<typename T>
	requires (std::is_default_constructible_v<T>)
	std::string Serialize(const std::vector<T>& arr, const char separatorChar)
	{
		if (arr.empty())
			return "";

		std::string result = Serialize<T>(arr[0]);
		for (size_t i = 1; i < arr.size(); i++)
		{
			result += separatorChar + Serialize<T>(arr[i]);
		}
		return result;
	}
}
