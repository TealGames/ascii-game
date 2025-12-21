#pragma once
#include<cstdint>

struct MemoryInterval
{
	std::uint32_t m_Offset = 0;
	std::uint32_t m_Size = 0;

	inline uint32_t GetEndOffset() const
	{
		return m_Offset + m_Size;
	}
};

struct ArrayInterval
{
	std::uint32_t m_StartIndex = 0;
	std::uint32_t m_Size = 0;

	inline uint32_t GetEndIndex() const
	{
		return m_StartIndex + m_Size;
	}
};