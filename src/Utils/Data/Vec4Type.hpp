#pragma once
#include <cmath>
#include "VecBase.hpp"
#include "Utils/Math.hpp"
#include "Vec2Type.hpp"
#include "Vec3Type.hpp"
#include "Utils/ToStringFunctions.hpp"
#include "Utils/Debug.hpp"

template<typename T>
requires std::is_arithmetic_v<T>
class Vec<T, 4>
{
private:
public:
	union
	{
		struct { T m_X, m_Y, m_Z, m_W; };
		T m_Components[4];
	};

public:
	constexpr Vec() : Vec(0, 0, 0, 0) {}

	constexpr Vec(const T xComp, const T yComp, const T zComp, const T wComp)
		: m_X(xComp), m_Y(yComp), m_Z(zComp), m_W(wComp) {}

	constexpr Vec(const Vec<T, 2>& xy, const T zComp, const T wComp)
		: Vec(xy.m_X, xy.m_Y, zComp, wComp) {}

	constexpr Vec(const T xComp, const Vec<T, 2>& yz, const T wComp)
		: Vec(xComp, yz.m_X, yz.m_Y, wComp) {}

	constexpr Vec(const T xComp, const T yComp, const Vec<T, 2>& zw)
		: Vec(xComp, yComp, zw.m_X, zw.m_Y) {}

	constexpr Vec(const Vec<T, 3>& xyz, const T wComp)
		: Vec(xyz.m_X, xyz.m_Y, xyz.m_Z, wComp) {}

	constexpr Vec(const T xComp, const Vec<T, 3>& yzw)
		: Vec(xComp, yzw.m_X, yzw.m_Y, yzw.m_W) {}

	Vec(const Vec&) = default;
	Vec(Vec&&) noexcept = default;

	static inline constexpr Vec Zero() { return { 0, 0, 0, 0 }; }
	static inline constexpr Vec One() { return { 1, 1, 1, 1 }; }

	Vec GetX() const { return Vec(m_X, 0, 0, 0); }
	Vec GetY() const { return Vec(0, m_Y, 0, 0); }
	Vec GetZ() const { return Vec(0, 0, m_Z, 0); }
	Vec GetW() const { return Vec(0, 0, 0, m_W); }

	Vec<T, 2> GetXY() const { return Vec<T, 2>(m_X, m_Y); }
	Vec<T, 2> GetYZ() const { return Vec<T, 2>(m_Y, m_Z); }
	Vec<T, 2> GetXZ() const { return Vec<T, 2>(m_X, m_Z); }

	Vec<T, 3> GetXYZ() const { return Vec<T, 3>(m_X, m_Y, m_Z); }
	Vec<T, 3> GetYZW() const { return Vec<T, 3>(m_Y, m_Z, m_W); }

	/// <summary>
	/// Returns a pointer to the first value in vector, 
	/// allowing user to treat vector components as array due to 
	/// them being packed right after one another in memory.
	/// </summary>
	/// <returns></returns>
	const T* GetMemPointer() const
	{
		return &m_X;
	}

	float GetMagnitude() const { return std::sqrt(GetMagnitudeSquared()); }
	/// <summary>
	/// Computes the magnitude squared. 
	/// Better for performance because avoids expensive sqrt.
	/// Use when comparing magnitudes and don't need real value.
	/// Same as dot product of itself
	/// </summary>
	/// <returns></returns>
	float GetMagnitudeSquared() const { return m_X * m_X + m_Y * m_Y + m_Z * m_Z + m_W * m_W; }

	Vec GetNormalized() const
	{
		const float magnitude = GetMagnitude();
		if (Utils::ApproximateEqualsF(magnitude, 0))
			return Vec::Zero();

		return Vec(m_X / magnitude, m_Y / magnitude, m_Z / magnitude, m_W / magnitude);
	}

	bool IsUnitVector() const { return Utils::ApproximateEqualsF(GetMagnitude(), 1); }

	std::string ToString(const std::uint8_t& decimalPlaces = 5) const
	{
		return std::format("({},{},{},{})", 
			Utils::ToString(Utils::Roundf(m_X, decimalPlaces), decimalPlaces),
			Utils::ToString(Utils::Roundf(m_Y, decimalPlaces), decimalPlaces),
			Utils::ToString(Utils::Roundf(m_Z, decimalPlaces), decimalPlaces),
			Utils::ToString(Utils::Roundf(m_W, decimalPlaces), decimalPlaces));
	}

	T& operator[](const size_t index)
	{
		if (index >= 4)
		{
			LogError(std::format("Invalid vec4 index:{}", index));
			throw std::invalid_argument(std::format("Invalid vec4 index:{}", index));
		}
			
		return m_Components[index];
	}
	const T& operator[](const size_t index) const
	{
		if (index >= 4)
		{
			LogError(std::format("Invalid vec4 index:{}", index));
			throw std::invalid_argument(std::format("Invalid vec4 index:{}", index));
		}
		return m_Components[index];
	}

	Vec operator+(const Vec& otherVec) const
	{
		return Vec{ m_X + otherVec.m_X, m_Y + otherVec.m_Y, m_Z + otherVec.m_Z, m_W + otherVec.m_W };
	}
	Vec& operator+=(const Vec& other)
	{
		m_X += other.m_X;
		m_Y += other.m_Y;
		m_Z += other.m_Z;
		m_W += other.m_W;
		return *this;
	}

	Vec operator-() const
	{
		return Vec{ -m_X, -m_Y, -m_Z, -m_W };
	}
	Vec operator-(const Vec& otherVec) const
	{
		return Vec{ m_X - otherVec.m_X, m_Y - otherVec.m_Y, m_Z - otherVec.m_Z, m_W - otherVec.m_W };
	}
	Vec& operator-=(const Vec& other)
	{
		m_X -= other.m_X;
		m_Y -= other.m_Y;
		m_Z -= other.m_Z;
		m_W -= other.m_W;
		return *this;
	}

	Vec operator*(const Vec& otherVec) const
	{
		return Vec{ m_X * otherVec.m_X, m_Y * otherVec.m_Y, m_Z * otherVec.m_Z, m_W * otherVec.m_W };
	}
	Vec operator*(const float scalar) const
	{
		return Vec{ m_X * scalar, m_Y * scalar, m_Z * scalar, m_W * scalar };
	}
	Vec operator*(const int scalar) const
	{
		return Vec{ m_X * scalar, m_Y * scalar, m_Z * scalar, m_W * scalar };
	}

	Vec operator/(const Vec& other) const
	{
		if (Utils::ApproximateEqualsF(m_X,0) || Utils::ApproximateEqualsF(other.m_Y,0) 
			|| Utils::ApproximateEqualsF(other.m_Z,0), Utils::ApproximateEqualsF(other.m_W, 0))
		{
			LogError(std::format("Tried to divide a vector: {} by a 0-value vector:{}", ToString(), other.ToString()));
			throw std::invalid_argument("Divide vec4 by 0");
		}

		return Vec{ m_X / other.m_X, m_Y / other.m_Y, m_Z / other.m_Z, m_W / other.m_W };
	}
	Vec operator/(const float scalar) const
	{
		if (Utils::ApproximateEqualsF(scalar, 0))
		{
			LogError(std::format("Tried to divide a vector: {} by a 0 float scalar:{}", ToString(), scalar));
			throw std::invalid_argument("Divide vec4 by 0");
		}

		return Vec{ m_X / scalar, m_Y / scalar, m_Z / scalar, m_W / scalar };
	}
	Vec operator/(const int scalar) const
	{
		if (scalar == 0)
		{
			LogError(std::format("Tried to divide a vector: {} by a 0 int scalar:{}", ToString(), scalar));
			throw std::invalid_argument("Divide vec4 by 0");
		}

		return Vec{ m_X / scalar, m_Y / scalar, m_Z / scalar, m_W / scalar };
	}
	Vec& operator/=(const float scalar)
	{
		return *this = *this / scalar;
	}
	Vec& operator/=(const int scalar)
	{
		return *this = *this / scalar;
	}

	bool operator==(const Vec& otherVec) const
	{
		return Utils::ApproximateEqualsF(m_X, otherVec.m_X) &&
			Utils::ApproximateEqualsF(m_Y, otherVec.m_Y) &&
			Utils::ApproximateEqualsF(m_Z, otherVec.m_Z) &&
			Utils::ApproximateEqualsF(m_W, otherVec.m_W);
	}
	bool operator!=(const Vec& other) const
	{
		return !(*this == other);
	}
	bool operator>(const Vec& other) const
	{
		return m_X > other.m_X && m_Y > other.m_Y && m_Z > other.m_Z && m_W > other.m_W;
	}
	bool operator>=(const Vec& other) const
	{
		return m_X >= other.m_X && m_Y >= other.m_Y && m_Z >= other.m_Z && m_W >= other.m_W;
	}
	bool operator<(const Vec& other) const
	{
		return m_X < other.m_X && m_Y < other.m_Y && m_Z < other.m_Z && m_W < other.m_W;
	}
	bool operator<=(const Vec& other) const
	{
		return m_X <= other.m_X && m_Y <= other.m_Y && m_Z <= other.m_Z && m_W <= other.m_W;
	}

	Vec& operator=(const Vec& other)
	{
		if (this == &other)
			return *this;

		m_X = other.m_X;
		m_Y = other.m_Y;
		m_Z = other.m_Z;
		m_W = other.m_W;
		return *this;
	}

	Vec& operator=(Vec&& other) noexcept
	{
		m_X = std::exchange(other.m_X, 0.0);
		m_Y = std::exchange(other.m_Y, 0.0);
		m_Z = std::exchange(other.m_Z, 0.0);
		m_W = std::exchange(other.m_W, 0.0);
		return *this;
	}
};

using Vec4 = Vec<float, 4>;
using Vec4Int = Vec<int, 4>;