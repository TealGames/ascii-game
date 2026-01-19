#pragma once
#include "Utils/Debug.hpp"

template<typename T, size_t N, size_t ALIGN_BYTES = 0>
requires (N > 0)
struct alignas(ALIGN_BYTES) Group
{
	T m_Elements[N];

	T& operator[](const size_t index) noexcept
	{
		ENGINE_ASSERT("Attempted to access invalid group index:{} for Group<{}>", index, N);
		return m_Elements[index];
	}
	const T& operator[](const size_t index) const noexcept
	{
		ENGINE_ASSERT("Attempted to access invalid group index:{} for Group<{}>", index, N);
		return m_Elements[index];
	}

	std::string ToString() const
	{
		std::string result = '(' + std::to_string(m_Elements[0]);
		for (size_t i = 1; i < N; i++)
		{
			result += ',' + std::to_string(m_Elements[i]);
		}
		result += ')';
		return result;
	}
};