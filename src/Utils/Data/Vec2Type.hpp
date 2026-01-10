#pragma once
#include <cmath>
#include <array>
#include <numbers>
#include "Utils/Data/VecBase.hpp"
#include "Utils/Math.hpp"
#include "Utils/ToStringFunctions.hpp"

template<typename T, size_t ALIGN_BYTES>
requires (std::is_arithmetic_v<T>)
class alignas(ALIGN_BYTES) Vec<T, 2, ALIGN_BYTES>
{
private:
public:
	union
	{
		struct { T m_X, m_Y; };
		T m_Components[2];
	};

public:
	constexpr Vec() : Vec(0, 0) {}
	//constexpr Vec(const T xy) : m_X(xy), m_Y(xy) {}
	constexpr Vec(const T xComp, const T yComp)
		: m_X(xComp), m_Y(yComp) {}

	Vec(const std::array<T, 2>& arr) : m_X(arr[0]), m_Y(arr[1]) {}

	Vec(const Vec&) = default;
	Vec(Vec&&) noexcept = default;

	constexpr Vec GetX() const { return Vec(m_X, 0); }
	constexpr Vec GetY() const { return Vec(0, m_Y); }

	constexpr Vec<int, 2> AsInt() const requires (std::is_floating_point_v<T>)
	{
		return Vec<int, 2>(m_X, m_Y);
	}
	constexpr Vec<float, 2> AsFloat() const requires (std::is_integral_v<T>)
	{
		return Vec<float, 2>(m_X, m_Y);
	}

	static constexpr Vec One() { return Vec{ 1, 1 }; }
	static constexpr Vec Zero() { return Vec{ 0, 0 }; }

	const T* GetMemPointer() const { return &m_X; }
	T* GetMemPointerMutable() { return &m_X; }

	float GetAngle(const AngleMode& angleMode) const
	{
		//Just in case to prevent implementations returning undefined
		if (Utils::ApproximateEqualsF(GetMagnitude(), 0))
			return 0;

		float rad = std::atan2(m_Y, m_X);
		//Since atan2 gives you result in [-pi/2, pi/2) we can flip negaative rads
		//to their positive by doing full rotation other way

		//TODO: i think this is wrong because we get negative angles in both the second and fourth quadrants
		//which would not make sense to add 2pi to second quadrant angle. Also, this means all are in terms of -90, 90
		//which would also make the third quadrant wrong as well
		if (rad < 0) rad += 2 * std::numbers::pi;
		if (angleMode == AngleMode::Degrees) 
			return Utils::ToDegrees(rad);

		return rad;
	}

	float GetMagnitude() const
	{
		return std::sqrt(GetMagnitudeSquared());
	}
	/// <summary>
	/// Computes the magnitude squared. 
	/// Better for performance because avoids expensive sqrt. 
	/// Use when comparing magnitudes and don't need real value.
	/// Same as dot product of itself
	/// </summary>
	/// <returns></returns>
	float GetMagnitudeSquared() const { return m_X * m_X + m_Y * m_Y; }

	Vec GetNormalized() const
	{
		const float magnitude = GetMagnitude();
		if (Utils::ApproximateEqualsF(magnitude, 0))
			return {};

		return Vec(m_X / magnitude, m_Y / magnitude);
	}
	void Normalize()
	{
		const float magnitude = GetMagnitude();
		if (Utils::ApproximateEqualsF(magnitude, 0))
			return;

		m_X /= magnitude;
		m_Y /= magnitude;
	}

	bool IsUnitVector() const
	{
		return Utils::ApproximateEqualsF(GetMagnitude(), 1);
	}

	std::string ToString(const std::uint8_t& decimalPlaces = 5, VectorForm form= VectorForm::Component) const
	{
		std::string str;
		std::string xRounded = Utils::ToString(Utils::Roundf(m_X, decimalPlaces), decimalPlaces);
		std::string yRounded = Utils::ToString(Utils::Roundf(m_Y, decimalPlaces), decimalPlaces);
		switch (form)
		{
		case VectorForm::Component:
			str = std::format("({},{})", xRounded, yRounded);
			break;

		case VectorForm::Unit:
			//Double braces needed on the outside to escape format {}
			str = std::format("{{ {}i{}j }}", xRounded, Utils::GetSignSymbol(m_Y)+yRounded);
			break;

		case VectorForm::MagnitudeDirection:
			str = std::format("{}@ {}°", std::to_string(Utils::Roundf(GetMagnitude(), decimalPlaces)),
				std::to_string(Utils::Roundf(GetAngle(AngleMode::Degrees), decimalPlaces)));
			break;

		default:
			throw std::invalid_argument(std::format("Tried to convert vector ({},{}) to string "
				"with undefined form {}", m_X, m_Y, ::ToString(form)));
			break;
		}
		return str;
	}

	T& operator[](const size_t index)
	{
		if (index >= 2)
			throw std::invalid_argument(std::format("Invalid vec2 index:{}", index));
		return m_Components[index];
	}
	const T& operator[](const size_t index) const
	{
		if (index >= 2)
			throw std::invalid_argument(std::format("Invalid vec2 index:{}", index));
		return m_Components[index];
	}

	constexpr Vec operator+(const Vec& otherVec) const
	{
		return Vec{ m_X + otherVec.m_X, m_Y + otherVec.m_Y };
	}
	Vec& operator+=(const Vec& other)
	{
		m_X += other.m_X;
		m_Y += other.m_Y;
		return *this;
	}

	constexpr Vec operator-() const
	{
		return Vec{ -m_X, -m_Y };
	}
	constexpr Vec operator-(const Vec& otherVec) const
	{
		return { m_X - otherVec.m_X, m_Y - otherVec.m_Y };
	}
	Vec& operator-=(const Vec& other)
	{
		m_X -= other.m_X;
		m_Y -= other.m_Y;
		return *this;
	}

	constexpr Vec operator*(const Vec& otherVec) const 
	{
		return Vec{ m_X * otherVec.m_X, m_Y * otherVec.m_Y };
	}

	constexpr Vec operator*(const float scalar) const requires (std::same_as<T, float>)
	{
		return Vec(m_X * scalar, m_Y * scalar);
	}
	constexpr Vec operator*(const int scalar) const requires (std::same_as<T, int>)
	{
		return Vec{ m_X * scalar, m_Y * scalar };
	}
	constexpr Vec operator/(const Vec& other) const
	{
		if constexpr (std::is_floating_point_v<T>)
		{
			ENGINE_ASSERT(!Utils::ApproximateEqualsF(other.m_X, 0) && !Utils::ApproximateEqualsF(other.m_Y, 0), 
				"Tried to divide a vector2: {} by a 0-value vector:{}", ToString(), other.ToString());
		}
		else
		{
			ENGINE_ASSERT(other.m_X != 0 && other.m_Y != 0,
				"Tried to divide a vector2: {} by a 0-value vector:{}", ToString(), other.ToString());
		}

		return { m_X / other.m_X, m_Y / other.m_Y };
	}
	constexpr Vec operator/(const float scalar) const requires (std::same_as<T, float>)
	{
		ENGINE_ASSERT(!Utils::ApproximateEqualsF(scalar, 0), "Tried to divide a vector2: {} by a 0 value scalar", ToString());
		return Vec(m_X / scalar, m_Y / scalar);
	}
	constexpr Vec operator/(const int scalar) const requires (std::same_as<T, int>)
	{
		ENGINE_ASSERT(scalar != 0, "Tried to divide a vector2: {} by a 0 value scalar", ToString());
		return Vec{ m_X / scalar, m_Y / scalar };
	}

	bool operator==(const Vec& otherVec) const
	{
		return Utils::ApproximateEqualsF(m_X, otherVec.m_X) &&
			Utils::ApproximateEqualsF(m_Y, otherVec.m_Y);
	}
	bool operator!=(const Vec& other) const
	{
		return !(*this == other);
	}
	bool operator>(const Vec& other) const
	{
		return m_X > other.m_X && m_Y > other.m_Y;
	}
	bool operator>=(const Vec& other) const
	{
		return m_X >= other.m_X && m_Y >= other.m_Y;
	}
	bool operator<(const Vec& other) const
	{
		return m_X < other.m_X && m_Y < other.m_Y;
	}
	bool operator<=(const Vec& other) const
	{
		return m_X <= other.m_X && m_Y <= other.m_Y;
	}

	bool AnyAxisGreaterThan(const Vec& other) const
	{
		return m_X > other.m_X || m_Y > other.m_Y;
	}
	bool AnyAxisLessThan(const Vec& other) const
	{
		return m_X < other.m_X || m_Y < other.m_Y;
	}

	Vec& operator=(const Vec& other)
	{
		if (this == &other)
			return *this;

		m_X = other.m_X;
		m_Y = other.m_Y;
		return *this;
	}

	Vec& operator=(Vec&& other) noexcept
	{
		m_X = std::exchange(other.m_X, 0.0);
		m_Y = std::exchange(other.m_Y, 0.0);
		return *this;
	}
};

using Vec2 = Vec<float, 2>;
using Vec2Int = Vec<int, 2>;

extern template class Vec<float, 2>;
extern template class Vec<int, 2>;

//	Returns the scalar dot product. 
// -> This essentially finds how much one vector is aligned in terms of the other (where order does NOT matter and produces the same result)
// -> If one vector is a unit vector, it essnetially then can find how much of a vector is in the same direction as the unit vector
// -> THIS IS MOST USEFUL FOR GETTING THE SCALAR FOR VECTORS IN PARALLEL DIRECTIONS (and extracting the parts in that direction)
// -> Note: <0 result means they are in opposite directions, >0 means they are <90 degrees from each other and =0 means they are at 90 degrees
template<typename T>
T DotProduct(const Vec<T, 2>& vecA, const Vec<T, 2>& vecB)
{
	return (vecA.m_X * vecB.m_X) + (vecA.m_Y * vecB.m_Y);
}

template<typename T>
Vec<T, 2> Abs(const Vec<T, 2>& vec)
{
	return Vec<T, 2>(std::abs(vec.m_X), std::abs(vec.m_Y));
}

template<typename T>
Vec2Int GetSign(const Vec<T, 2>& vec)
{
	return Vec2(Utils::GetSign(vec.m_X), Utils::GetSign(vec.m_Y));
}

template<typename T>
Vec<T, 2> GetNormal(const Vec<T, 2>& vec, const RotationDirection dir)
{
	if (dir == RotationDirection::Clockwise)
		return Vec<T, 2>(vec.m_Y, -vec.m_X);
	else
		return Vec<T, 2>(-vec.m_Y, vec.m_X);
}

/// <summary>
/// Return true if the point is wtihin the bounding volume formed by the min (bottom left pos)
/// and the max (top right pos)
/// </summary>
/// <param name="point"></param>
/// <param name="min"></param>
/// <param name="max"></param>
/// <returns></returns>
template<typename T>
bool IsPointWithinBounds(const Vec<T, 2>& point, const Vec<T, 2>& min, const Vec<T, 2>& max) 
{
	return (point.m_X >= min.m_X && point.m_X <= max.m_X) &&
		(point.m_Y >= min.m_Y && point.m_Y <= max.m_Y);
}

inline Vec2 GetDirVector(const float radianAngle)
{
	return Vec2(std::cosf(radianAngle), std::sinf(radianAngle));
}
