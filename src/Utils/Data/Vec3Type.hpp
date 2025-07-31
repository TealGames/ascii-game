#pragma once
#include "Utils/Data/VecBase.hpp"
#include <cmath>
#include <numbers>
#include "VectorEnums.hpp"
#include "Utils/Math.hpp"
#include "Vec2Type.hpp"
#include "Utils/ToStringFunctions.hpp"

template<typename T>
requires std::is_arithmetic_v<T>
class Vec<T, 3>
{
public:
	union
	{
		struct { T m_X, m_Y, m_Z; };
		T m_Components[3];
	};

public:
	constexpr Vec() : Vec(0, 0, 0) {}
	constexpr Vec(const T x, const T y, const T z) : m_X(x), m_Y(y), m_Z(z) {}

	constexpr Vec(const Vec<T, 2>& xy, const T zComp)
		: Vec(xy.m_X, xy.m_Y, zComp) {}

	constexpr Vec(const T xComp, const Vec<T, 2>& yz)
		: Vec(xComp, yz.m_X, yz.m_Y) {}

	Vec(const Vec&) = default;
	Vec(Vec&&) noexcept = default;

	static inline constexpr Vec One() { return Vec{ 1, 1, 1 }; }
	static inline constexpr Vec Zero() { return Vec{ 0, 0, 0 }; }

	Vec GetX() const { return Vec(m_X, 0, 0); }
	Vec GetY() const { return Vec(0, m_Y, 0); }
	Vec GetZ() const { return Vec(0, 0, m_Z); }

	Vec<T, 2> GetXY() const { return Vec<T, 2>(m_X, m_Y); }
	Vec<T, 2> GetYZ() const { return Vec<T, 2>(m_Y, m_Z); }
	Vec<T, 2> GetXZ() const { return Vec<T, 2>(m_X, m_Z); }

	void SetXY(const Vec<T, 2> vec)
	{
		m_X = vec.m_X;
		m_Y = vec.m_Y;
	}

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
	float GetMagnitudeSquared() const { return m_X * m_X + m_Y * m_Y + m_Z * m_Z; }

	Vec<T, 3> GetNormalized() const
	{
		const float magnitude = GetMagnitude();
		if (Utils::ApproximateEqualsF(magnitude, 0))
			return Vec::Zero();

		return Vec<T, 3>(m_X / magnitude, m_Y / magnitude, m_Z / magnitude);
	}

	bool IsUnitVector() const { return Utils::ApproximateEqualsF(GetMagnitude(), 1); }

	std::string ToString(const std::uint8_t& decimalPlaces = 5, VectorForm form = VectorForm::Component) const
	{
		std::string str;
		std::string xRounded = Utils::ToString(Utils::Roundf(m_X, decimalPlaces), decimalPlaces);
		std::string yRounded = Utils::ToString(Utils::Roundf(m_Y, decimalPlaces), decimalPlaces);
		std::string zRounded = Utils::ToString(Utils::Roundf(m_Z, decimalPlaces), decimalPlaces);
		switch (form)
		{
		case VectorForm::Component:
			str = std::format("({},{},{})", xRounded, yRounded, zRounded);
			break;

		case VectorForm::Unit:
			//Double braces needed on the outside to escape format {}
			str = std::format("{{ {}i{}j{}k}}", xRounded, Utils::GetSignSymbol(m_Y) + yRounded, Utils::GetSignSymbol(m_Z) + zRounded);
			break;

		case VectorForm::MagnitudeDirection:
			str = std::format("{}@ {}", std::to_string(Utils::Roundf(GetMagnitude(), decimalPlaces)),
				GetNormalized().ToString(decimalPlaces, VectorForm::Component), decimalPlaces);
			break;

		default:
			throw std::invalid_argument(std::format("Tried to convert vector ({},{},{}) to string "
				"with undefined form {}", m_X, m_Y, m_Z, ::ToString(form)));
			break;
		}
		return str;
	}

	T& operator[](const size_t index)
	{
		if (index >= 3)
			throw std::invalid_argument(std::format("Invalid vec3 index:{}", index));
		return m_Components[index];
	}
	const T& operator[](const size_t index) const
	{
		if (index >= 3)
			throw std::invalid_argument(std::format("Invalid vec3 index:{}", index));
		return m_Components[index];
	}

	Vec operator+(const Vec& otherVec) const
	{
		return Vec{ m_X + otherVec.m_X, m_Y + otherVec.m_Y, m_Z + otherVec.m_Z };
	}
	Vec& operator+=(const Vec& other)
	{
		m_X += other.m_X;
		m_Y += other.m_Y;
		m_Z += other.m_Z;
		return *this;
	}

	Vec operator-() const
	{
		return Vec{ -m_X, -m_Y, -m_Z };
	}
	Vec operator-(const Vec& otherVec) const
	{
		return Vec{ m_X - otherVec.m_X, m_Y - otherVec.m_Y, m_Z - otherVec.m_Z };
	}
	Vec& operator-=(const Vec& other)
	{
		m_X -= other.m_X;
		m_Y -= other.m_Y;
		m_Z -= other.m_Z;
		return *this;
	}

	Vec operator*(const Vec& otherVec) const
	{
		return Vec{ m_X * otherVec.m_X, m_Y * otherVec.m_Y, m_Z * otherVec.m_Z };
	}

	Vec operator*(const float scalar) const
	{
		return Vec{ m_X * scalar, m_Y * scalar, m_Z * scalar };
	}
	Vec operator*(const int scalar) const
	{
		return Vec{ m_X * scalar, m_Y * scalar, m_Z * scalar };
	}
	Vec operator/(const Vec& other) const
	{
		if (Utils::ApproximateEqualsF(other.m_X, 0) || Utils::ApproximateEqualsF(other.m_Y, 0) || Utils::ApproximateEqualsF(other.m_Z, 0))
		{
			throw std::invalid_argument(std::format("Tried to divide a vector: {} by a 0-value vector:{}", ToString(), other.ToString()));
			return *this;
		}

		return Vec{ m_X / other.m_X, m_Y / other.m_Y, m_Z / other.m_Z };
	}
	Vec operator/(const float scalar) const
	{
		if (Utils::ApproximateEqualsF(scalar, 0))
		{
			throw std::invalid_argument(std::format("Tried to divide a vector: {} by a 0 value scalar", ToString()));
			return *this;
		}

		return Vec{ m_X / scalar, m_Y / scalar, m_Z / scalar };
	}
	Vec operator/(const int scalar) const
	{
		if (scalar == 0)
		{
			throw std::invalid_argument(std::format("Tried to divide a vector: {} by a 0 value scalar", ToString()));
			return *this;
		}

		return Vec{ m_X / scalar, m_Y / scalar, m_Z / scalar };
	}

	bool operator==(const Vec& otherVec) const
	{
		return Utils::ApproximateEqualsF(m_X, otherVec.m_X) &&
			Utils::ApproximateEqualsF(m_Y, otherVec.m_Y) &&
			Utils::ApproximateEqualsF(m_Z, otherVec.m_Z);
	}
	bool operator!=(const Vec& other) const
	{
		return !(*this == other);
	}
	bool operator>(const Vec& other) const
	{
		return m_X > other.m_X && m_Y > other.m_Y && m_Z > other.m_Z;
	}
	bool operator>=(const Vec& other) const
	{
		return m_X >= other.m_X && m_Y >= other.m_Y && m_Z >= other.m_Z;
	}
	bool operator<(const Vec& other) const
	{
		return m_X < other.m_X && m_Y < other.m_Y && m_Z < other.m_Z;
	}
	bool operator<=(const Vec& other) const
	{
		return m_X <= other.m_X && m_Y <= other.m_Y && m_Z <= other.m_Z;
	}

	Vec& operator=(const Vec& other)
	{
		if (this == &other)
			return *this;

		m_X = other.m_X;
		m_Y = other.m_Y;
		m_Z = other.m_Z;
		return *this;
	}

	Vec& operator=(Vec&& other) noexcept
	{
		m_X = std::exchange(other.m_X, 0.0);
		m_Y = std::exchange(other.m_Y, 0.0);
		m_Z = std::exchange(other.m_Z, 0.0);
		return *this;
	}
};

/*
template<typename IntegralType>
requires std::is_arithmetic_v<IntegralType>
class Vec3Type
{
private:
public:
	IntegralType m_X;
	IntegralType m_Y;
	IntegralType m_Z;

public:
	constexpr Vec3Type() : Vec3Type(0, 0, 0) {}

	constexpr Vec3Type(const IntegralType xComp, const IntegralType yComp, const IntegralType zComp)
		: m_X(xComp), m_Y(yComp), m_Z(zComp) {}
	
	constexpr Vec3Type(const Vec2Type<IntegralType>& xy, const IntegralType zComp)
		: Vec3Type(xy.m_X, xy.m_Y, zComp) {}

	constexpr Vec3Type(const IntegralType xComp, const Vec2Type<IntegralType>& yz)
		: Vec3Type(xComp, yz.m_X, yz.m_Y) {}

	Vec3Type(const Vec3Type&) = default;
	Vec3Type(Vec3Type&&) noexcept = default;

	static inline constexpr Vec3Type One() { return Vec3Type{ 1, 1, 1 }; }
	static inline constexpr Vec3Type Zero() { return Vec3Type{ 0, 0, 0 }; }

	Vec3Type GetX() const { return Vec3Type(m_X, 0, 0); }
	Vec3Type GetY() const { return Vec3Type(0, m_Y, 0); }
	Vec3Type GetZ() const { return Vec3Type(0, 0, m_Z); }

	Vec2Type<IntegralType> GetXY() const { return Vec2Type<IntegralType>(m_X, m_Y); }
	Vec2Type<IntegralType> GetYZ() const { return Vec2Type<IntegralType>(m_Y, m_Z); }
	Vec2Type<IntegralType> GetXZ() const { return Vec2Type<IntegralType>(m_X, m_Z); }

	/// <summary>
	/// Returns a pointer to the first value in vector, 
	/// allowing user to treat vector components as array due to 
	/// them being packed right after one another in memory.
	/// </summary>
	/// <returns></returns>
	const IntegralType* GetMemPointer() const
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
	float GetMagnitudeSquared() const { return m_X * m_X + m_Y * m_Y + m_Z * m_Z; }

	Vec3Type GetNormalized() const
	{
		const float magnitude = GetMagnitude();
		if (Utils::ApproximateEqualsF(magnitude, 0))
			return Vec3Type::Zero();

		return Vec3Type(m_X / magnitude, m_Y / magnitude, m_Z/magnitude);
	}

	bool IsUnitVector() const { return Utils::ApproximateEqualsF(GetMagnitude(), 1); }

	std::string ToString(const std::uint8_t& decimalPlaces = 5, VectorForm form = VectorForm::Component) const
	{
		std::string str;
		std::string xRounded = Utils::ToString(Utils::Roundf(m_X, decimalPlaces), decimalPlaces);
		std::string yRounded = Utils::ToString(Utils::Roundf(m_Y, decimalPlaces), decimalPlaces);
		std::string zRounded = Utils::ToString(Utils::Roundf(m_Z, decimalPlaces), decimalPlaces);
		switch (form)
		{
		case VectorForm::Component:
			str = std::format("({},{},{})", xRounded, yRounded, zRounded);
			break;

		case VectorForm::Unit:
			//Double braces needed on the outside to escape format {}
			str = std::format("{{ {}i{}j{}k}}", xRounded, Utils::GetSignSymbol(m_Y)+yRounded, Utils::GetSignSymbol(m_Z)+zRounded);
			break;

		case VectorForm::MagnitudeDirection:
			str = std::format("{}@ {}", std::to_string(Utils::Roundf(GetMagnitude(), decimalPlaces)),
				GetNormalized().ToString(decimalPlaces, VectorForm::Component), decimalPlaces);
			break;

		default:
			throw std::invalid_argument(std::format("Tried to convert vector ({},{},{}) to string "
				"with undefined form {}", m_X, m_Y, m_Z, ::ToString(form)));
			break;
		}
		return str;
	}

	Vec3Type operator+(const Vec3Type& otherVec) const
	{
		return Vec3Type{ m_X + otherVec.m_X, m_Y + otherVec.m_Y, m_Z+ otherVec.m_Z };
	}
	Vec3Type& operator+=(const Vec3Type& other)
	{
		m_X += other.m_X;
		m_Y += other.m_Y;
		m_Z += other.m_Z;
		return *this;
	}

	Vec3Type operator-() const
	{
		return Vec3Type{ -m_X, -m_Y, -m_Z };
	}
	Vec3Type operator-(const Vec3Type& otherVec) const
	{
		return { m_X - otherVec.m_X, m_Y - otherVec.m_Y, m_Z - otherVec.m_Z};
	}
	Vec3Type& operator-=(const Vec3Type& other)
	{
		m_X -= other.m_X;
		m_Y -= other.m_Y;
		m_Z -= other.m_Z;
		return *this;
	}

	Vec3Type operator*(const Vec3Type& otherVec) const
	{
		return { m_X * otherVec.m_X, m_Y * otherVec.m_Y, m_Z * otherVec.m_Z };
	}

	Vec3Type operator*(const float scalar) const
	{
		return { m_X * scalar, m_Y * scalar, m_Z * scalar };
	}
	Vec3Type operator*(const int scalar) const
	{
		return { m_X * scalar, m_Y * scalar, m_Z * scalar };
	}
	Vec3Type operator/(const Vec3Type& other) const
	{
		if (Utils::ApproximateEqualsF(other.m_X, 0) || Utils::ApproximateEqualsF(other.m_Y, 0) || Utils::ApproximateEqualsF(other.m_Z, 0))
		{
			throw std::invalid_argument(std::format("Tried to divide a vector: {} by a 0-value vector:{}", ToString(), other.ToString()));
			return *this;
		}

		return { m_X / other.m_X, m_Y / other.m_Y, m_Z / other.m_Z };
	}
	Vec3Type operator/(const float scalar) const
	{
		if (Utils::ApproximateEqualsF(scalar, 0))
		{
			throw std::invalid_argument(std::format("Tried to divide a vector: {} by a 0 value scalar", ToString()));
			return *this;
		}

		return { m_X / scalar, m_Y / scalar, m_Z / scalar };
	}
	Vec3Type operator/(const int scalar) const
	{
		if (scalar == 0)
		{
			throw std::invalid_argument(std::format("Tried to divide a vector: {} by a 0 value scalar", ToString()));
			return *this;
		}

		return { m_X / scalar, m_Y / scalar, m_Z / scalar };
	}

	bool operator==(const Vec3Type& otherVec) const
	{
		return Utils::ApproximateEqualsF(m_X, otherVec.m_X) &&
			Utils::ApproximateEqualsF(m_Y, otherVec.m_Y) &&
			Utils::ApproximateEqualsF(m_Z, otherVec.m_Z);
	}
	bool operator!=(const Vec3Type& other) const
	{
		return !(*this == other);
	}
	bool operator>(const Vec3Type& other) const
	{
		return m_X > other.m_X && m_Y > other.m_Y && m_Z > other.m_Z;
	}
	bool operator>=(const Vec3Type& other) const
	{
		return m_X >= other.m_X && m_Y >= other.m_Y && m_Z >= other.m_Z;
	}
	bool operator<(const Vec3Type& other) const
	{
		return m_X < other.m_X && m_Y < other.m_Y && m_Z < other.m_Z;
	}
	bool operator<=(const Vec3Type& other) const
	{
		return m_X <= other.m_X && m_Y <= other.m_Y && m_Z <= other.m_Z;
	}

	Vec3Type& operator=(const Vec3Type& other)
	{
		if (this == &other)
			return *this;

		m_X = other.m_X;
		m_Y = other.m_Y;
		m_Z = other.m_Z;
		return *this;
	}

	Vec3Type& operator=(Vec3Type&& other) noexcept
	{
		m_X = std::exchange(other.m_X, 0.0);
		m_Y = std::exchange(other.m_Y, 0.0);
		m_Z = std::exchange(other.m_Z, 0.0);
		return *this;
	}
};
*/
using Vec3 = Vec<float, 3>;
using Vec3Int = Vec<int, 3>;

/// <summary>
/// Returns the scalar dot product. 
/// -> This essentially finds how much one vector is aligned in terms of the other (where order does NOT matter and produces the same result)
/// -> If one vector is a unit vector, it essnetially then can find how much of a vector is in the same direction as the unit vector
/// -> THIS IS MOST USEFUL FOR GETTING THE SCALAR FOR VECTORS IN PARALLEL DIRECTIONS (and extracting the parts in that direction)
/// -> Note: <0 result means they are in opposite directions, >0 means they are <90 degrees from each other and =0 means they are at 90 degrees
/// 
/// NOTE: SAME AS magnitude(vecA) * magnitude(vecB) * cos(angle of A and B)
/// </summary>
/// <typeparam name="IntegralType"></typeparam>
/// <param name="vecA"></param>
/// <param name="vecB"></param>
/// <returns></returns>
template<typename T>
T DotProduct(const Vec<T, 3>& vecA, const Vec<T, 3>& vecB)
{
	return (vecA.m_X * vecB.m_X) + (vecA.m_Y * vecB.m_Y) + (vecA.m_Z * vecB.m_Z);
}

/// <summary>
/// Calculates a 3rd perpendiclar vector to the existing vectors. 
/// To figure out where the vector will be -> use RIGHT HAND RULE:
/// ->Index finger points in direction of vecA
/// ->Middle finger points in direction of vecB
/// ->Thumb points in direction of resulting vector
/// 
/// NOTE: ORDER MATTERS -> vecA x vecB = -(vecB x VecA)
/// NOTE: **MAGNITUDE ONLY** can be calculated as magnitude(vecA) * magnitude(vecB) * sin(angle of A and B)
/// </summary>
/// <typeparam name="IntegralType"></typeparam>
/// <param name="vecA"></param>
/// <param name="vecB"></param>
/// <returns></returns>
template<typename T>
Vec<T, 3> CrossProduct(const Vec<T, 3>& vecA, const Vec<T, 3>& vecB)
{
	return Vec<T, 3>(vecA.m_Y * vecB.m_Z - vecA.m_Z * vecB.m_Y, 
								  vecA.m_Z * vecB.m_X - vecA.m_X * vecB.m_Z,
								  vecA.m_X * vecB.m_Y - vecA.m_Y * vecB.m_X);
}

template<typename T>
Vec<T, 3> Abs(const Vec<T, 3>& vec)
{
	return Vec<T, 3>(std::abs(vec.m_X), std::abs(vec.m_Y), std::abs(vec.m_Z));
}

template<typename T>
Vec3Int GetSign(const Vec<T, 3>& vec)
{
	return Vec3Int(Utils::GetSign(vec.m_X), Utils::GetSign(vec.m_Y), Utils::GetSign(vec.m_Z));
}

template<typename T>
Vec<T, 3> Round(const Vec<T, 3>& vec, const std::uint8_t& decimalPlaces = 5)
{
	return Vec<T, 3>(Utils::Roundf(vec.m_X, decimalPlaces), 
		Utils::Roundf(vec.m_Y, decimalPlaces), Utils::Roundf(vec.m_Z, decimalPlaces));
}

/// <summary>
/// Return true if the point is wtihin the bounding volume formed by the min (forwardmost bottom left pos)
/// and the max (backmost top right pos)
/// </summary>
/// <param name="point"></param>
/// <param name="min"></param>
/// <param name="max"></param>
/// <returns></returns>
bool IsPointWithinBounds(const Vec3& point, const Vec3& min, const Vec3& max) {
	return (point.m_X >= min.m_X && point.m_X <= max.m_X) &&
		   (point.m_Y >= min.m_Y && point.m_Y <= max.m_Y) &&
		   (point.m_Z >= min.m_Z && point.m_Z <= max.m_Z);
}