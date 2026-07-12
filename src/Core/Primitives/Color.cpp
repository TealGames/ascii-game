#include <format>
#include "Core/Primitives/Color.hpp"

namespace Engine
{
	std::array<std::byte, 8> FromColHDR4ToF16Bytes(const ColHDR4& color)
	{
		std::array<std::byte, 8> result = {};
		std::uint16_t f16BytesR = ::Math::Float32ToFloat16Bytes(color.m_R);
		std::uint16_t f16BytesG = ::Math::Float32ToFloat16Bytes(color.m_G);
		std::uint16_t f16BytesB = ::Math::Float32ToFloat16Bytes(color.m_B);
		std::uint16_t f16BytesA = ::Math::Float32ToFloat16Bytes(color.m_A);
		memcpy(&result[0], &f16BytesR, 2);
		memcpy(&result[0] + 2, &f16BytesG, 2);
		memcpy(&result[0] + 4, &f16BytesB, 2);
		memcpy(&result[0] + 6, &f16BytesA, 2);
		return result;
	}
	std::array<std::byte, 16> FromColHDR4ToF32Bytes(const ColHDR4& color)
	{
		std::array<std::byte, 16> result = {};
		memcpy(&result[0], &color, sizeof(ColHDR4));
		return result;
	}

	std::array<std::byte, 8> FromColorToF16Bytes(const Col4& color)
	{
		return FromColHDR4ToF16Bytes(ToColHDR4(color));
	}

	ColHDR4 FromF16BytesToColHDR4(const std::array<std::byte, 8>& bytes)
	{
		return FromF16BytesToColHDR4(&bytes[0]);
	}
	ColHDR4 FromF16BytesToColHDR4(const std::byte* bytes)
	{
		std::uint16_t f16BytesR = 0, f16BytesG = 0, f16BytesB = 0, f16BytesA = 0;
		memcpy(&f16BytesR, bytes, 2);
		memcpy(&f16BytesG, bytes + 2, 2);
		memcpy(&f16BytesB, bytes + 4, 2);
		memcpy(&f16BytesA, bytes + 6, 2);

		return { ::Math::Float16BytesToFloat32(f16BytesR), ::Math::Float16BytesToFloat32(f16BytesG),
				 ::Math::Float16BytesToFloat32(f16BytesB), ::Math::Float16BytesToFloat32(f16BytesA) };
	}
	ColHDR4 FromF32BytesToColHDR4(const std::array<std::byte, 16>& bytes)
	{
		return FromF32BytesToColHDR4(&bytes[0]);
	}
	ColHDR4 FromF32BytesToColHDR4(const std::byte* bytes)
	{
		ColHDR4 result = {};
		memcpy(&result, bytes, sizeof(ColHDR4));
		return result;
	}
	Col4 FromF16BytesToColor(const std::array<std::byte, 8>& bytes)
	{
		return FromColHDR4(FromF16BytesToColHDR4(bytes));
	}

	Col4 FromBytesToColor(const std::array<std::byte, 4>& bytes)
	{
		Col4 result = {};
		memcpy(&result, &bytes[0], sizeof(Col4));
		return result;
	}
	Col4 FromBytesToColor(const std::array<std::byte, 3>& bytes, const std::uint8_t alpha)
	{
		Col4 result = {};
		memcpy(&result, &bytes[0], 3);
		result.m_A = alpha;
		return result;
	}
}


