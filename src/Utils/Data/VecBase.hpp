#pragma once
#include <type_traits>
#include <format>
#include <string>
#include "Utils/TemplateConcepts.hpp"
#include "Utils/Math.hpp"

template<typename T, size_t N, size_t ALIGN_BYTES = 0>
requires (std::is_arithmetic_v<T> && N>0)
struct alignas(ALIGN_BYTES) Vec
{
	T m_Components[N];

	T& operator[](const size_t index)
	{
		if (index >= N)
			throw std::invalid_argument(std::format("Invalid vec2 index:{}", index));
		return m_Components[index];
	}
	const T& operator[](const size_t index) const
	{
		if (index >= N)
			throw std::invalid_argument(std::format("Invalid vec2 index:{}", index));
		return m_Components[index];
	}

	std::string ToString() const
	{
		std::string result = '(' + std::to_string(m_Components[0]);
		for (size_t i = 1; i < m_Components.size(); i++)
		{
			result += ',' + std::to_string(m_Components[i]);
		}
		result += ')';
		return result;
	}
};

template<typename T, size_t N>
requires std::is_arithmetic_v<T>
T Sum(const Vec<T, N>& vec)
{
	T result = 0;
	for (const auto& component : vec.m_Components)
	{
		result += component;
	}
	return result;
}

template<typename AxisT, size_t N, typename... OtherT>
requires Utils::AllSameType<Vec<AxisT, N>, OtherT...>
AxisT MaxAxis(const std::uint8_t axisIndex, const Vec<AxisT, N>& first, const OtherT& ...other)
{
	return Utils::Max(first[axisIndex], other[axisIndex]...);
}
/// <summary>
/// Creates a vector with a MAX component value from all vectors in args for all axes
/// </summary>
/// <typeparam name="AxisT"></typeparam>
/// <typeparam name="...OtherT"></typeparam>
/// <typeparam name="N"></typeparam>
/// <param name="first"></param>
/// <param name="...other"></param>
/// <returns></returns>
template<typename AxisT, size_t N, typename... OtherT>
requires Utils::AllSameType<Vec<AxisT, N>, OtherT...>
Vec<AxisT, N> Max(const Vec<AxisT, N>& first, const OtherT& ...other)
{
	Vec<AxisT, N> result = {};
	for (std::uint8_t i = 0; i < N; i++)
	{
		result[i] = Utils::Max(first[i], other[i]...);
	}
	return result;
}

template<typename AxisT, size_t N, typename... OtherT>
requires Utils::AllSameType<Vec<AxisT, N>, OtherT...>
AxisT MinAxis(const std::uint8_t axisIndex, const Vec<AxisT, N>& first, const OtherT& ...other)
{
	return Utils::Min(first[axisIndex], other[axisIndex]...);
}
/// <summary>
/// Creates a vector with a MIN component value from all vectors in args for all axes
/// </summary>
/// <typeparam name="AxisT"></typeparam>
/// <typeparam name="...OtherT"></typeparam>
/// <typeparam name="N"></typeparam>
/// <param name="first"></param>
/// <param name="...other"></param>
/// <returns></returns>
template<typename AxisT, size_t N, typename... OtherT>
requires Utils::AllSameType<Vec<AxisT, N>, OtherT...>
Vec<AxisT, N> Min(const Vec<AxisT, N>& first, const OtherT& ...other)
{
	Vec<AxisT, N> result = {};
	for (std::uint8_t i = 0; i < N; i++)
	{
		result[i] = Utils::Min(first[i], other[i]...);
	}
	return result;
}