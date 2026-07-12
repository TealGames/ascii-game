#pragma once
#include <array>
#include "Core/Primitives/ColorBase.hpp"
#include "Utils/HelperMacros.hpp"

namespace Engine
{
	enum class ColorChannel : std::uint8_t
	{
		None = 0,
		R	 = 0b1,
		G	 = 0b10,
		B	 = 0b100,
		A	 = 0b1000,
		All	 = 0xFF
	};
	FLAG_ENUM_OPERATORS(ColorChannel)

	using Col3 = Col<std::uint8_t, 3>;
	using ColHDR3 = Col<float, 3>;

	using Col4 = Col<std::uint8_t, 4>;
	using ColHDR4 = Col<float, 4>;

	inline constexpr std::uint8_t MAX_INT_COLOR_CHANNEL = 255;
	inline constexpr float MAX_FLOAT_COLOR_CHANNEL = 1.0f;

	inline constexpr ColHDR4 COLOR_LIGHT_GRAY = ColHDR4(200, 200, 200, 255);
	inline constexpr ColHDR4 COLOR_GRAY = ColHDR4(130, 130, 130, 255);
	inline constexpr ColHDR4 COLOR_DARK_GRAY = ColHDR4(80, 80, 80, 255);
	inline constexpr ColHDR4 COLOR_YELLOW = ColHDR4(253, 249, 0, 255);
	inline constexpr ColHDR4 COLOR_GOLD = ColHDR4(255, 203, 0, 255);
	inline constexpr ColHDR4 COLOR_ORANGE = ColHDR4(255, 161, 0, 255);
	inline constexpr ColHDR4 COLOR_PINK = ColHDR4(255, 109, 194, 255);
	inline constexpr ColHDR4 COLOR_RED = ColHDR4(230, 41, 55, 255);
	inline constexpr ColHDR4 COLOR_MAROON = ColHDR4(190, 33, 55, 255);
	inline constexpr ColHDR4 COLOR_GREEN = ColHDR4(0, 228, 48, 255);
	inline constexpr ColHDR4 COLOR_LIME = ColHDR4(0, 158, 47, 255);
	inline constexpr ColHDR4 COLOR_DARK_GREEN = ColHDR4(0, 117, 44, 255);
	inline constexpr ColHDR4 COLOR_SKYBLUE = ColHDR4(102, 191, 255, 255);
	inline constexpr ColHDR4 COLOR_BLUE = ColHDR4(0, 121, 241, 255);
	inline constexpr ColHDR4 COLOR_DARK_BLUE = ColHDR4(0, 82, 172, 255);
	inline constexpr ColHDR4 COLOR_PURPLE = ColHDR4(255, 122, 255, 255);
	inline constexpr ColHDR4 COLOR_VIOLET = ColHDR4(135, 60, 190, 255);
	inline constexpr ColHDR4 COLOR_DARK_PURPLE = ColHDR4(112, 31, 126, 255);
	inline constexpr ColHDR4 COLOR_BEIGE = ColHDR4(211, 176, 131, 255);
	inline constexpr ColHDR4 COLOR_BROWN = ColHDR4(127, 106, 79, 255);
	inline constexpr ColHDR4 COLOR_DARK_BROWN = ColHDR4(76, 63, 47, 255);
	inline constexpr ColHDR4 COLOR_WHITE = ColHDR4(255, 255, 255, 255);
	inline constexpr ColHDR4 COLOR_BLACK = ColHDR4(0, 0, 0, 255);
	inline constexpr ColHDR4 COLOR_BLANK = ColHDR4(0, 0, 0, 0);
	inline constexpr ColHDR4 COLOR_MAGENTA = ColHDR4(255, 0, 255, 255);

	template<typename T, size_t N, size_t ALIGN>
	constexpr bool IsNormalized(const Col<T, N, ALIGN>& col)
	{
		for (size_t i = 0; i < N; i++)
		{
			if (col.m_Channels[i] < 0 || col.m_Channels[i] > 1)
				return false;
		}
		return true;
	}

	template<typename T, size_t N, size_t ALIGN>
	constexpr bool IsHDR(const Col<T, N, ALIGN>& col)
	{
		if constexpr (!std::is_same_v<T, float>)
			return false;

		for (size_t i = 0; i < N; i++)
		{
			if (col.m_Channels[i] > 1.0f)
				return true;
		}
		return false;
	}

	template<typename T, size_t N, size_t ALIGN>
	Vec<T, N, ALIGN>& ReinterpretAsVec(Col<T, N, ALIGN>& col)
	{
		return reinterpret_cast<Vec<T, N, ALIGN>&>(col);
	}
	template<typename T, size_t N, size_t ALIGN>
	const Vec<T, N, ALIGN>& ReinterpretAsVec(const Col<T, N, ALIGN>& col)
	{
		return reinterpret_cast<const Vec<T, N, ALIGN>&>(col);
	}

	template<typename T, size_t N, size_t ALIGN>
	Col<T, N, ALIGN>& ReinterpretAsCol(Vec<T, N, ALIGN>& vec)
	{
		return reinterpret_cast<Col<T, N, ALIGN>&>(vec);
	}
	template<typename T, size_t N, size_t ALIGN>
	const Col<T, N, ALIGN>& ReinterpretAsCol(const Vec<T, N, ALIGN>& vec)
	{
		return reinterpret_cast<const Col<T, N, ALIGN>&>(vec);
	}

	template<typename T, size_t N, size_t ALIGN>
	T GetRangeClampSingular(const Col<T, N, ALIGN>& col, const std::uint8_t channelIndex, const bool clampNegative, const bool clampGreaterThan1)
	{
		ENGINE_ASSERT(channelIndex < N, "Attempted to clamp invalid color channel index:{} of col{}", channelIndex, N);

		T newChannelValue = col.m_Channels[channelIndex];
		if (col.m_Channels[channelIndex] < 0 && clampNegative)
			newChannelValue = 0;
		else if (col.m_Channels[channelIndex] > 1.0f && clampGreaterThan1)
			newChannelValue = 1.0f;

		return newChannelValue;
	}

	template<typename T, size_t N, size_t ALIGN>
	void ApplyRangeClamp(const Col<T, N, ALIGN>& col, const bool clampNegatives, const bool clampGreaterThan1)
	{
		for (size_t i = 0; i < N; i++)
		{
			col.m_Channels[i] = GetRangeClampSingular(col, i, clampNegatives, clampGreaterThan1);
		}
	}

	/// <summary>
	/// Will clamp each rgb channel to [0, infinity) (NOTE: since this assumes it is hdr, it will NOT
	/// clamp the rgb channels with max of 1 since hdr can have >1), but alpha will be clamped to [0,1]
	/// </summary>
	/// <returns></returns>
	template<typename T, size_t N, size_t ALIGN>
		requires (std::is_floating_point_v<T>)
	Col<T, N, ALIGN> ClampRangeAsHDR(const Col<T, N, ALIGN>& col)
	{
		Col<T, N, ALIGN> newChannels;
		for (std::uint8_t i = 0; i < 3 && i < N; i++)
		{
			newChannels[i] = GetRangeClampSingular(col, i, true, false);
		}
		newChannels[3] = GetRangeClampSingular(col, 3, true, false);
		return Col(newChannels);
	}

	constexpr ColHDR4 ConstructColorFromHex(const std::uint32_t& hexNumber)
	{
		return
			ColHDR4(
				//We can just move the corresponding rgba value and then mask it
				//to only include that segment, and convert to unsigned char as needed
				static_cast<std::uint8_t>((hexNumber >> 24) & 0xFF),
				static_cast<std::uint8_t>((hexNumber >> 16) & 0xFF),
				static_cast<std::uint8_t>((hexNumber >> 8) & 0xFF),
				static_cast<std::uint8_t>(hexNumber & 0xFF)
			);
	}
	constexpr Col4 FromColHDR4(const ColHDR4& color)
	{
		return Col4(std::uint8_t(color.m_R), std::uint8_t(color.m_G),
			std::uint8_t(color.m_B), std::uint8_t(color.m_A));
	}
	constexpr ColHDR4 ToColHDR4(const Col4& color)
	{
		return ColHDR4(color.m_R, color.m_G, color.m_B, color.m_A);
	}

	/// <summary>
	/// Converts an ColHDR4 (4 channels each with 1 float32 = 16 total bytes)
	/// -> binary format with 4 channels each with float16 representation (8 total bytes)
	/// </summary>
	/// <param name="color"></param>
	/// <returns></returns>
	std::array<std::byte, 8> FromColHDR4ToF16Bytes(const ColHDR4& color);
	/// <summary>
	/// Converts an ColHDR4 (4 channels each with 1 float32 = 16 total bytes)
	/// -> binary format with 4 channels each with float32 (same layout, just in byte form)
	/// </summary>
	/// <param name="color"></param>
	/// <returns></returns>
	std::array<std::byte, 16> FromColHDR4ToF32Bytes(const ColHDR4& color);
	/// <summary>
	/// Converts a Color (4 channels each with 1 byte = 4 total bytes)
	/// -> binary format with 4 channels each with float16 (8 total bytes)
	/// </summary>
	/// <param name="color"></param>
	/// <returns></returns>
	std::array<std::byte, 8> FromColorToF16Bytes(const Col4& color);

	/// <summary>
	/// Converts binary format with 4 channels each with float16 representation (8 total bytes)
	/// -> an ColHDR4 (4 channels each with 1 float32 = 16 total bytes)
	/// </summary>
	/// <param name="bytes"></param>
	/// <returns></returns>
	ColHDR4 FromF16BytesToColHDR4(const std::array<std::byte, 8>& bytes);
	ColHDR4 FromF16BytesToColHDR4(const std::byte* bytes);
	/// <summary>
	/// Converts binary format with 4 channels each with float32 (16 total bytes)
	/// -> an ColHDR4 (4 channels each with 1 float32 = 16 total bytes)
	/// </summary>
	/// <param name="bytes"></param>
	/// <returns></returns>
	ColHDR4 FromF32BytesToColHDR4(const std::array<std::byte, 16>& bytes);
	ColHDR4 FromF32BytesToColHDR4(const std::byte* bytes);
	/// <summary>
	/// Converts binary format with 4 channels each with float16 (8 total bytes)
	/// -> a Color (4 channels each with 1 byte = 4 total bytes)
	/// </summary>
	/// <param name="bytes"></param>
	/// <returns></returns>
	Col4 FromF16BytesToColor(const std::array<std::byte, 8>& bytes);
	Col4 FromBytesToColor(const std::array<std::byte, 4>& bytes);
	Col4 FromBytesToColor(const std::array<std::byte, 3>& bytes, const std::uint8_t alpha);
}




