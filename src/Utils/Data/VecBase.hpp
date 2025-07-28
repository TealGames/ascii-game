#pragma once
#include <type_traits>
#include <format>

template<typename T, size_t N>
requires std::is_arithmetic_v<T>
struct Vec
{
	T m_elements[N];

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
};