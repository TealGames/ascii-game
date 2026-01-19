#pragma once
#include "Utils/Math/Vec4Type.hpp"

template<typename T>
requires std::is_arithmetic_v<T>
class Col
{
private:
public:
	union
	{
		struct { T m_R, m_G, m_B, m_A; };
		T m_Channels[4];
	};

	static constexpr T MAX_CHANNEL_VALUE = std::numeric_limits<T>::max();

public:
	//NOTE: the following general constructors must use other constructors to ensure
	//clamping of alpha occurs for floating point T types

	constexpr Col(const Vec<T, 2>& rg, const T b, const T a)
		: Col(rg.m_X, rg.m_Y, b, a) {}

	constexpr Col(const T r, const Vec<T, 2>& gb, const T a)
		: Col(r, gb.m_X, gb.m_Y, a) {}

	constexpr Col(const T r, const T g, const Vec<T, 2>& ba)
		: Col(r, g, ba.m_X, ba.m_Y) {}

	constexpr Col(const Vec<T, 3>& rgb, const T a)
		: Col(rgb.m_X, rgb.m_Y, rgb.m_Z, a) {}

	constexpr Col(const T r, const Vec<T, 3>& gba)
		: Col(r, gba.m_X, gba.m_Y, gba.m_Z) {}

	constexpr Col(const Vec<T, 4>& rgba)
		: Col(rgba.m_X, rgba.m_Y, rgba.m_Z, rgba.m_W) {}

	constexpr Col(const Col& rg, const T b, const T a)
		: Col(rg.m_R, rg.m_G, b, a) {}

	constexpr Col(const T r, const Col& gb, const T a)
		: Col(r, gb.m_G, gb.m_B, a) {}

	constexpr Col(const T r, const T g, const Col& ba)
		: Col(r, g, ba.m_B, ba.m_A) {}

	constexpr Col(const Col& rgb, const T a)
		: Col(rgb.m_R, rgb.m_G, rgb.m_B, a) {}

	constexpr Col(const T r, const Col& gba)
		: Col(r, gba.m_G, gba.m_B, gba.m_A) {}


	// ---------------------------------------- UINT8 ONLY CONSTRUCTORS -------------------------------------
	constexpr Col() requires std::is_same_v<T, std::uint8_t>
		: m_R(MAX_CHANNEL_VALUE), m_G(MAX_CHANNEL_VALUE), m_B(MAX_CHANNEL_VALUE), m_A(MAX_CHANNEL_VALUE) {}

	constexpr Col(std::uint8_t r, std::uint8_t g, std::uint8_t b) requires std::is_same_v<T, std::uint8_t>
		: m_R(r), m_G(g), m_B(b), m_A(MAX_CHANNEL_VALUE) {}

	constexpr Col(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a) requires std::is_same_v<T, std::uint8_t>
		: m_R(r), m_G(g), m_B(b), m_A(a) {}


	// ------------------------------------ FLOATING POINT ONLY CONSTRUCTORS --------------------------------
	constexpr Col() requires std::is_floating_point_v<T>
		: m_R(1.0f), m_G(1.0f),m_B(1.0f), m_A(1.0f) {}

	constexpr Col(const T r, const T g, const T b) 
		requires std::is_floating_point_v<T>
		: m_R(r), m_G(g), m_B(b), m_A(1.0f) {}

	constexpr Col(const T r, const T g, const T b, const T a)
		requires std::is_floating_point_v<T>
		: m_R(r), m_G(g), m_B(b), m_A(std::min(a, T(1))) {}

	constexpr Col(std::uint8_t r, std::uint8_t g, std::uint8_t b)
		requires (std::is_floating_point_v<T>)
		: m_R(r / 255.0f), m_G(g / 255.0f), m_B(b / 255.0f), m_A(1.0f) {}

	constexpr Col(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a)
		requires (std::is_floating_point_v<T>)
		: m_R(r / 255.0f), m_G(g / 255.0f), m_B(b / 255.0f), m_A(a / 255.0f) {}

	constexpr Col(int r, int g, int b)
		requires (std::is_floating_point_v<T>)
		: m_R(std::max(r / 255.0f, 0.0f)), m_G(std::max(g / 255.0f, 0.0f)), m_B(std::max(b / 255.0f, 0.0f)), m_A(1.0f) {}

	constexpr Col(int r, int g, int b, int a)
		requires (std::is_floating_point_v<T>)
		: m_R(std::max(r / 255.0f, 0.0f)), m_G(std::max(g / 255.0f, 0.0f)),
		  m_B(std::max(b / 255.0f, 0.0f)), m_A(std::max(a / 255.0f, 0.0f)) {}


	Col(const Col&) = default;
	Col(Col&&) noexcept = default;

	static inline constexpr Col Zero() { return { 0, 0, 0, 0 }; } 
	static inline constexpr Col Max() 
	{ 
		return 
		{ 
			MAX_CHANNEL_VALUE, MAX_CHANNEL_VALUE, 
			MAX_CHANNEL_VALUE, MAX_CHANNEL_VALUE 
		}; 
	}

	constexpr Col GetR() const { return Vec(m_R, 0, 0, 0); }
	constexpr Col GetG() const { return Vec(0, m_G, 0, 0); }
	constexpr Col GetB() const { return Vec(0, 0, m_B, 0); }
	constexpr Col GetA() const { return Vec(0, 0, 0, m_A); }

	constexpr Vec<T, 2> GetRG() const { return Vec<T, 2>(m_R, m_G); }
	constexpr Vec<T, 2> GetGB() const { return Vec<T, 2>(m_G, m_B); }
	constexpr Vec<T, 2> GetBA() const { return Vec<T, 2>(m_B, m_A); }

	constexpr Vec<T, 3> GetRGB() const { return Vec<T, 3>(m_R, m_G, m_B); }
	constexpr Vec<T, 3> GetGBA() const { return Vec<T, 3>(m_G, m_B, m_A); }
	constexpr Vec<T, 4> GetRGBA() const { return Vec<T, 4>(m_R, m_G, m_B, m_A); }

	/// <summary>
	/// Returns a pointer to the first value in vector, 
	/// allowing user to treat vector components as array due to 
	/// them being packed right after one another in memory.
	/// </summary>
	/// <returns></returns>
	const T* GetMemPointer() const
	{
		return &m_R;
	}

	constexpr bool IsNormalized() const
	{
		return 0 < m_R && m_R < 1 && 0 < m_G && m_G < 1 
			&& 0 < m_B && m_B < 1 && 0 < m_A && m_A < 1;
	}

	Col<float> GetNormalized() const
		requires (!std::is_floating_point_v<T>)
	{
		return Col(float(m_R) / MAX_CHANNEL_VALUE, float(m_G) / MAX_CHANNEL_VALUE, 
					 float(m_B) / MAX_CHANNEL_VALUE, float(m_A) / MAX_CHANNEL_VALUE);
	}

	constexpr bool IsHDR() const
	{
		return m_R > 1.0f || m_G > 1.0f || m_B > 1.0f || m_A > 1.0f;
	}
	constexpr bool HasNonzeroRGB() const
	{
		return m_R > 0.0f || m_G > 0.0f || m_B > 0.0f;
	}
	constexpr bool HasNonzeroAlpha() const
	{
		return m_A > 0.0f;
	}
	constexpr bool HasVisibleNonzeroRGB() const
	{
		return HasNonzeroRGB() && HasNonzeroAlpha();
	}


	void ApplyRangeClamp(const bool clampNegatives, const bool clampGreaterThan1)
	{
		for (std::uint8_t i = 0; i < 4; i++)
		{
			ApplyRangeClamp(i, clampNegatives, clampGreaterThan1);
		}
	}
	Col GetRangeClamp(const bool clampNegatives, const bool clampGreaterThan1) const
	{
		Vec<T, 4> newChannels = Vec<T, 4>();
		for (std::uint8_t i = 0; i < 4; i++)
		{
			newChannels[i] = GetRangeClampSingular(i, clampNegatives, clampGreaterThan1);
		}
		return Col(newChannels);
	}

	void ApplyRangeClamp(const std::uint8_t channelIndex, const bool clampNegative, const bool clampGreaterThan1)
		requires std::is_floating_point_v<T>
	{
		if (channelIndex >= 4)
		{
			LogError(std::format("Attempted to clamp invalid color channel index:{}", channelIndex));
			return;
		}

		if (m_Channels[channelIndex] < 0 && clampNegative)
			m_Channels[channelIndex] = 0;
		else if (m_Channels[channelIndex] > 1.0f && clampGreaterThan1)
			m_Channels[channelIndex] = 1f;
	}
	Col GetRangeClamp(const std::uint8_t channelIndex, const bool clampNegative, const bool clampGreaterThan1) const
		requires std::is_floating_point_v<T>
	{
		if (channelIndex >= 4)
		{
			LogError(std::format("Attempted to clamp invalid color channel index:{}", channelIndex));
			return Col();
		}
		
		Vec<T, 4> channels = Vec<T, 4>();
		if (channels[channelIndex] < 0 && clampNegative)
			channels[channelIndex] = 0;
		else if (channels[channelIndex] > 1.0f && clampGreaterThan1)
			channels[channelIndex] = 1f;

		return Col(channels);
	}
	T GetRangeClampSingular(const std::uint8_t channelIndex, const bool clampNegative, const bool clampGreaterThan1) const
		requires std::is_floating_point_v<T>
	{
		if (channelIndex >= 4)
		{
			LogError(std::format("Attempted to clamp invalid color channel index:{}", channelIndex));
			return T();
		}

		T newChannelValue = T(m_Channels[channelIndex]);
		if (m_Channels[channelIndex] < 0 && clampNegative)
			newChannelValue = 0;
		else if (m_Channels[channelIndex] > 1.0f && clampGreaterThan1)
			newChannelValue = 1.0f;

		return newChannelValue;
	}

	/// <summary>
	/// Will clamp each rgb channel to [0, infinity) (NOTE: since this assumes it is hdr, it will NOT
	/// clamp the rgb channels with max of 1 since hdr can have >1), but alpha will be clamped to [0,1]
	/// </summary>
	/// <returns></returns>
	Col GetRangeClampAsHDR() const
	{
		Vec<T, 4> newChannels = Vec<T, 4>();
		for (std::uint8_t i = 0; i < 3; i++)
		{
			newChannels[i] = GetRangeClampSingular(i, true, false);
		}
		newChannels[3] = GetRangeClampSingular(3, true, false);
		return Col(newChannels);
	}

	std::string ToString(const std::uint8_t& decimalPlaces = 5) const requires std::is_floating_point_v<T>
	{
		return std::format("(Col {},{},{},{})",
			Utils::ToString(Utils::Roundf(m_R, decimalPlaces), decimalPlaces),
			Utils::ToString(Utils::Roundf(m_G, decimalPlaces), decimalPlaces),
			Utils::ToString(Utils::Roundf(m_B, decimalPlaces), decimalPlaces),
			Utils::ToString(Utils::Roundf(m_A, decimalPlaces), decimalPlaces));
	}
	std::string ToString() const requires std::is_same_v<T, std::uint8_t>
	{
		return std::format("[Col {},{},{},{}]", m_R, m_G, m_B, m_A);
	}

	T& operator[](const size_t index)
	{
		if (index >= 4)
		{
			LogError(std::format("Invalid Col index:{}", index));
			throw std::invalid_argument(std::format("Invalid Col index:{}", index));
		}

		return m_Channels[index];
	}
	const T& operator[](const size_t index) const
	{
		if (index >= 4)
		{
			LogError(std::format("Invalid Col index:{}", index));
			throw std::invalid_argument(std::format("Invalid Col index:{}", index));
		}
		return m_Channels[index];
	}

	Col operator+(const Col& otherVec) const
	{
		return Col{ m_R + otherVec.m_R, m_G + otherVec.m_G, m_B + otherVec.m_B, m_A + otherVec.m_A };
	}
	Col& operator+=(const Col& other)
	{
		m_R += other.m_R;
		m_G += other.m_G;
		m_B += other.m_B;
		m_A += other.m_A;
		return *this;
	}

	Col operator-() const
	{
		return Col{ -m_R, -m_G, -m_B, -m_A };
	}
	Col operator-(const Col& otherVec) const
	{
		return Col{ m_R - otherVec.m_R, m_G - otherVec.m_G, m_B - otherVec.m_B, m_A - otherVec.m_A };
	}
	Col& operator-=(const Col& other)
	{
		m_R -= other.m_R;
		m_G -= other.m_G;
		m_B -= other.m_B;
		m_A -= other.m_A;
		return *this;
	}

	Col operator*(const Col& otherVec) const
	{
		return Col{ m_R * otherVec.m_R, m_G * otherVec.m_G, m_B * otherVec.m_B, m_A * otherVec.m_A };
	}
	Col operator*(const float scalar) const
	{
		return Col{ m_R * scalar, m_G * scalar, m_B * scalar, m_A * scalar };
	}
	Col operator*(const int scalar) const
	{
		return Col{ m_R * scalar, m_G * scalar, m_B * scalar, m_A * scalar };
	}

	Col operator/(const Col& other) const
	{
		if constexpr (std::is_floating_point_v<T>)
		{
			if (Utils::ApproximateEqualsF(m_R, 0) || Utils::ApproximateEqualsF(other.m_G, 0)
				|| Utils::ApproximateEqualsF(other.m_B, 0), Utils::ApproximateEqualsF(other.m_A, 0))
			{
				LogError(std::format("Tried to divide a Col: {} by a 0-value Col:{}", ToString(), other.ToString()));
				throw std::invalid_argument("Divide Col by 0");
			}
		}
		else
		{
			if (m_R == 0 || m_G == 0 || m_B == 0 || m_A == 0)
			{
				LogError(std::format("Tried to divide a Col: {} by a 0-value Col:{}", ToString(), other.ToString()));
				throw std::invalid_argument("Divide Col by 0");
			}
		}

		return Col{ m_R / other.m_R, m_G / other.m_G, m_B / other.m_B, m_A / other.m_A };
	}
	Col operator/(const float scalar) const
	{
		if (Utils::ApproximateEqualsF(scalar, 0))
		{
			LogError(std::format("Tried to divide a Col: {} by a 0 float scalar:{}", ToString(), scalar));
			throw std::invalid_argument("Divide Col by 0");
		}

		return Col{ m_R / scalar, m_G / scalar, m_B / scalar, m_A / scalar };
	}
	Col operator/(const int scalar) const
	{
		if (scalar == 0)
		{
			LogError(std::format("Tried to divide a Col: {} by a 0 int scalar:{}", ToString(), scalar));
			throw std::invalid_argument("Divide Col by 0");
		}

		return Col{ m_R / scalar, m_G / scalar, m_B / scalar, m_A / scalar };
	}
	Col& operator/=(const float scalar)
	{
		return *this = *this / scalar;
	}
	Col& operator/=(const int scalar)
	{
		return *this = *this / scalar;
	}

	bool operator==(const Col& other) const requires std::is_floating_point_v<T>
	{
		return Utils::ApproximateEqualsF(m_R, other.m_R) &&
			   Utils::ApproximateEqualsF(m_G, other.m_G) &&
			   Utils::ApproximateEqualsF(m_B, other.m_B) &&
			   Utils::ApproximateEqualsF(m_A, other.m_A);
	}
	bool operator==(const Col& other) const requires std::is_same_v<T, std::uint8_t>
	{
		return m_R == other.m_R && m_G == other.m_G && m_B == other.m_B && m_A == other.m_A;
	}
	bool operator!=(const Col& other) const
	{
		return !(*this == other);
	}
	bool operator>(const Col& other) const
	{
		return m_R > other.m_R && m_G > other.m_G && m_B > other.m_B && m_A > other.m_A;
	}
	bool operator>=(const Col& other) const
	{
		return m_R >= other.m_R && m_G >= other.m_G && m_B >= other.m_B && m_A >= other.m_A;
	}
	bool operator<(const Col& other) const
	{
		return m_R < other.m_R && m_G < other.m_G && m_B < other.m_B && m_A < other.m_A;
	}
	bool operator<=(const Col& other) const
	{
		return m_R <= other.m_R && m_G <= other.m_G && m_B <= other.m_B && m_A <= other.m_A;
	}

	Col& operator=(const Col& other)
	{
		if (this == &other)
			return *this;

		m_R = other.m_R;
		m_G = other.m_G;
		m_B = other.m_B;
		m_A = other.m_A;
		return *this;
	}

	Col& operator=(Col&& other) noexcept
	{
		m_R = std::exchange(other.m_R, 0.0);
		m_G = std::exchange(other.m_G, 0.0);
		m_B = std::exchange(other.m_B, 0.0);
		m_A = std::exchange(other.m_A, 0.0);
		return *this;
	}

	explicit operator Vec<T, 4>() const
	{
		return GetRGBA();
	}
};

using HDRColor = Col<float>;
using Color = Col<std::uint8_t>;

constexpr HDRColor ConstructColorFromHex(const std::uint32_t& hexNumber)
{
	return
		HDRColor(
			//We can just move the corresponding rgba value and then mask it
			//to only include that segment, and convert to unsigned char as needed
			static_cast<std::uint8_t>((hexNumber >> 24) & 0xFF),
			static_cast<std::uint8_t>((hexNumber >> 16) & 0xFF),
			static_cast<std::uint8_t>((hexNumber >> 8) & 0xFF),
			static_cast<std::uint8_t>(hexNumber & 0xFF)
		);
}
constexpr Color FromHDRColor(const HDRColor& color)
{
	return Color(std::uint8_t(color.m_R), std::uint8_t(color.m_G), 
		std::uint8_t(color.m_B), std::uint8_t(color.m_A));
}
constexpr HDRColor ToHDRColor(const Color& color)
{
	return HDRColor(color.m_R, color.m_G, color.m_B, color.m_A);
}

/// <summary>
/// Converts an HDRColor (4 channels each with 1 float32 = 16 total bytes)
/// -> binary format with 4 channels each with float16 representation (8 total bytes)
/// </summary>
/// <param name="color"></param>
/// <returns></returns>
std::array<std::byte, 8> FromHDRColorToF16Bytes(const HDRColor& color);
/// <summary>
/// Converts an HDRColor (4 channels each with 1 float32 = 16 total bytes)
/// -> binary format with 4 channels each with float32 (same layout, just in byte form)
/// </summary>
/// <param name="color"></param>
/// <returns></returns>
std::array<std::byte, 16> FromHDRColorToF32Bytes(const HDRColor& color);
/// <summary>
/// Converts a Color (4 channels each with 1 byte = 4 total bytes)
/// -> binary format with 4 channels each with float16 (8 total bytes)
/// </summary>
/// <param name="color"></param>
/// <returns></returns>
std::array<std::byte, 8> FromColorToF16Bytes(const Color& color);

/// <summary>
/// Converts binary format with 4 channels each with float16 representation (8 total bytes)
/// -> an HDRColor (4 channels each with 1 float32 = 16 total bytes)
/// </summary>
/// <param name="bytes"></param>
/// <returns></returns>
HDRColor FromF16BytesToHDRColor(const std::array<std::byte, 8>& bytes);
HDRColor FromF16BytesToHDRColor(const std::byte* bytes);
/// <summary>
/// Converts binary format with 4 channels each with float32 (16 total bytes)
/// -> an HDRColor (4 channels each with 1 float32 = 16 total bytes)
/// </summary>
/// <param name="bytes"></param>
/// <returns></returns>
HDRColor FromF32BytesToHDRColor(const std::array<std::byte, 16>& bytes);
HDRColor FromF32BytesToHDRColor(const std::byte* bytes);
/// <summary>
/// Converts binary format with 4 channels each with float16 (8 total bytes)
/// -> a Color (4 channels each with 1 byte = 4 total bytes)
/// </summary>
/// <param name="bytes"></param>
/// <returns></returns>
Color FromF16BytesToColor(const std::array<std::byte, 8>& bytes);
Color FromBytesToColor(const std::array<std::byte, 4>& bytes);
Color FromBytesToColor(const std::array<std::byte, 3>& bytes, const std::uint8_t alpha);

inline constexpr std::uint8_t MAX_INT_COLOR_CHANNEL = 255;
inline constexpr float MAX_FLOAT_COLOR_CHANNEL = 1.0;

inline constexpr HDRColor COLOR_LIGHT_GRAY = HDRColor(200, 200, 200, 255);
inline constexpr HDRColor COLOR_GRAY = HDRColor(130, 130, 130, 255);
inline constexpr HDRColor COLOR_DARK_GRAY = HDRColor(80, 80, 80, 255);
inline constexpr HDRColor COLOR_YELLOW = HDRColor(253, 249, 0, 255);
inline constexpr HDRColor COLOR_GOLD = HDRColor(255, 203, 0, 255);
inline constexpr HDRColor COLOR_ORANGE = HDRColor(255, 161, 0, 255);
inline constexpr HDRColor COLOR_PINK = HDRColor(255, 109, 194, 255);
inline constexpr HDRColor COLOR_RED = HDRColor(230, 41, 55, 255);
inline constexpr HDRColor COLOR_MAROON = HDRColor(190, 33, 55, 255);
inline constexpr HDRColor COLOR_GREEN = HDRColor(0, 228, 48, 255);
inline constexpr HDRColor COLOR_LIME = HDRColor(0, 158, 47, 255);
inline constexpr HDRColor COLOR_DARK_GREEN = HDRColor(0, 117, 44, 255);
inline constexpr HDRColor COLOR_SKYBLUE = HDRColor(102, 191, 255, 255);
inline constexpr HDRColor COLOR_BLUE = HDRColor(0, 121, 241, 255);
inline constexpr HDRColor COLOR_DARK_BLUE = HDRColor(0, 82, 172, 255);
inline constexpr HDRColor COLOR_PURPLE = HDRColor(200, 122, 255, 255);
inline constexpr HDRColor COLOR_VIOLET = HDRColor(135, 60, 190, 255);
inline constexpr HDRColor COLOR_DARK_PURPLE = HDRColor(112, 31, 126, 255);
inline constexpr HDRColor COLOR_BEIGE = HDRColor(211, 176, 131, 255);
inline constexpr HDRColor COLOR_BROWN = HDRColor(127, 106, 79, 255);
inline constexpr HDRColor COLOR_DARK_BROWN = HDRColor(76, 63, 47, 255);
inline constexpr HDRColor COLOR_WHITE = HDRColor(255, 255, 255, 255);
inline constexpr HDRColor COLOR_BLACK = HDRColor(0, 0, 0, 255);
inline constexpr HDRColor COLOR_BLANK = HDRColor(0, 0, 0, 0);
inline constexpr HDRColor COLOR_MAGENTA = HDRColor(255, 0, 255, 255);



