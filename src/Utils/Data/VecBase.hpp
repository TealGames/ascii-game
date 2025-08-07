#pragma once
#include <type_traits>
#include <format>
#include <string>

template<typename T, size_t N>
requires (std::is_arithmetic_v<T> && N>0)
struct Vec
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