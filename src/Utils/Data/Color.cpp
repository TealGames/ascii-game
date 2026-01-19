#include "Utils/Data/Color.hpp"
#include <format>

std::array<std::byte, 8> FromHDRColorToF16Bytes(const HDRColor& color)
{
	std::array<std::byte, 8> result = {};
	std::uint16_t f16BytesR = Utils::Float32ToFloat16Bytes(color.m_R);
	std::uint16_t f16BytesG = Utils::Float32ToFloat16Bytes(color.m_G);
	std::uint16_t f16BytesB = Utils::Float32ToFloat16Bytes(color.m_B);
	std::uint16_t f16BytesA = Utils::Float32ToFloat16Bytes(color.m_A);
	memcpy(&result[0], &f16BytesR, 2);
	memcpy(&result[0] + 2, &f16BytesG, 2);
	memcpy(&result[0] + 4, &f16BytesB, 2);
	memcpy(&result[0] + 6, &f16BytesA, 2);
	return result;
}
std::array<std::byte, 16> FromHDRColorToF32Bytes(const HDRColor& color)
{
	std::array<std::byte, 16> result = {};
	memcpy(&result[0], &color, sizeof(HDRColor));
	return result;
}

std::array<std::byte, 8> FromColorToF16Bytes(const Color& color)
{
	return FromHDRColorToF16Bytes(ToHDRColor(color));
}

HDRColor FromF16BytesToHDRColor(const std::array<std::byte, 8>& bytes)
{
	return FromF16BytesToHDRColor(&bytes[0]);
}
HDRColor FromF16BytesToHDRColor(const std::byte* bytes)
{
	std::uint16_t f16BytesR = 0, f16BytesG = 0, f16BytesB = 0, f16BytesA = 0;
	memcpy(&f16BytesR, bytes, 2);
	memcpy(&f16BytesG, bytes + 2, 2);
	memcpy(&f16BytesB, bytes + 4, 2);
	memcpy(&f16BytesA, bytes + 6, 2);

	return { Utils::Float16BytesToFloat32(f16BytesR),Utils::Float16BytesToFloat32(f16BytesG),
			 Utils::Float16BytesToFloat32(f16BytesB),Utils::Float16BytesToFloat32(f16BytesA) };
}
HDRColor FromF32BytesToHDRColor(const std::array<std::byte, 16>& bytes)
{
	return FromF32BytesToHDRColor(&bytes[0]);
}
HDRColor FromF32BytesToHDRColor(const std::byte* bytes)
{
	HDRColor result = {};
	memcpy(&result, bytes, sizeof(HDRColor));
	return result;
}
Color FromF16BytesToColor(const std::array<std::byte, 8>& bytes)
{
	return FromHDRColor(FromF16BytesToHDRColor(bytes));
}

Color FromBytesToColor(const std::array<std::byte, 4>& bytes)
{
	Color result = {};
	memcpy(&result, &bytes[0], sizeof(Color));
	return result;
}
Color FromBytesToColor(const std::array<std::byte, 3>& bytes, const std::uint8_t alpha)
{
	Color result = {};
	memcpy(&result, &bytes[0], 3);
	result.m_A = alpha;
	return result;
}

