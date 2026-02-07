#pragma once
#include "Utils/Math/VecBase.hpp"
#include <cmath>
#include <numbers>
#include <array>
#include "Vec2Type.hpp"
#include "Utils/ToStringFunctions.hpp"
#include "Utils/Debug.hpp"

template<typename T, size_t ALIGN_BYTES>
requires (std::is_arithmetic_v<T>)
class alignas(ALIGN_BYTES) Vec<T, 3, ALIGN_BYTES>
{
public:
	union
	{
		struct { T m_X, m_Y, m_Z; };
		T m_Components[3];
	};

public:
	constexpr Vec() : Vec(0, 0, 0) {}
	constexpr Vec(const T xyz) : m_X(xyz), m_Y(xyz), m_Z(xyz) {}
	constexpr Vec(const T x, const T y, const T z) : m_X(x), m_Y(y), m_Z(z) {}

	Vec(const std::array<T, 3>& arr) : m_X(arr[0]), m_Y(arr[1]), m_Z(arr[2]) {}

	constexpr Vec(const Vec<T, 2>& xy, const T zComp)
		: Vec(xy.m_X, xy.m_Y, zComp) {}

	constexpr Vec(const T xComp, const Vec<T, 2>& yz)
		: Vec(xComp, yz.m_X, yz.m_Y) {}

	Vec(const Vec&) = default;
	Vec(Vec&&) noexcept = default;

	template<size_t OTHER_ALIGN>
	requires (OTHER_ALIGN != ALIGN_BYTES)
	Vec(const Vec<T, 3, OTHER_ALIGN>& other)
		: m_X(other.m_X), m_Y(other.m_Y), m_Z(other.m_Z) {}

	template<size_t OTHER_ALIGN>
	requires (OTHER_ALIGN != ALIGN_BYTES && std::is_default_constructible_v<T>)
	Vec(Vec<T, 3, OTHER_ALIGN>&& other) noexcept
		: m_X(std::move(other.m_X)), m_Y(std::move(other.m_Y)), m_Z(std::move(other.m_Z)) {}

	static inline constexpr Vec One() { return Vec{ 1, 1, 1 }; }
	static inline constexpr Vec Zero() { return Vec{ 0, 0, 0 }; }

	/// <summary>
	/// Returns a vector where each component is the MIN
	/// possible value for the specified T type
	/// </summary>
	/// <returns></returns>
	static inline constexpr Vec Min()
	{
		T tMin = std::numeric_limits<T>::min();
		return { tMin, tMin, tMin };
	}
	/// <summary>
	/// Returns a vector where each component is the MAX
	/// possible value for the specified T type
	/// </summary>
	/// <returns></returns>
	static inline constexpr Vec Max()
	{
		T tMax = std::numeric_limits<T>::max();
		return { tMax, tMax, tMax };
	}

	constexpr Vec GetX() const { return Vec(m_X, 0, 0); }
	constexpr Vec GetY() const { return Vec(0, m_Y, 0); }
	constexpr Vec GetZ() const { return Vec(0, 0, m_Z); }

	/// <summary>
	/// Since Pitch is a rotation on the YZ plane, this 
	/// effectively returns the X-component of this vector
	/// </summary>
	/// <returns></returns>
	constexpr float GetPitch() const { return m_X; }
	/// <summary>
	/// Since YAW is a rotation on the XZ plane, this 
	/// effectively returns the Y-component of this vector
	/// </summary>
	/// <returns></returns>
	constexpr float GetYaw() const { return m_Y; }
	/// <summary>
	/// Since ROLL is a rotation on the XY plane, this 
	/// effectively returns the Z-component of this vector
	/// </summary>
	/// <returns></returns>
	constexpr float GetRoll() const { return m_Z; }

	constexpr Vec<T, 2> GetXY() const { return Vec<T, 2>(m_X, m_Y); }
	constexpr Vec<T, 2> GetYZ() const { return Vec<T, 2>(m_Y, m_Z); }
	constexpr Vec<T, 2> GetXZ() const { return Vec<T, 2>(m_X, m_Z); }

	constexpr Vec<int, 3> AsInt() const requires (std::is_floating_point_v<T>)
	{
		return Vec<int, 3>(m_X, m_Y, m_Z);
	}
	constexpr Vec<float, 3> AsFloat() const requires (std::is_integral_v<T>)
	{
		return Vec<float, 3>(m_X, m_Y, m_Z);
	}

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
	const T* GetMemPointer() const { return &m_X; }
	T* GetMemPointerMutable() { return &m_X; }

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
			return {};

		return Vec<T, 3>(m_X / magnitude, m_Y / magnitude, m_Z / magnitude);
	}
	Vec& Normalize()
	{
		const float magnitude = GetMagnitude();
		if (Utils::ApproximateEqualsF(magnitude, 0))
			return *this;

		m_X /= magnitude;
		m_Y /= magnitude;
		m_Z /= magnitude;
		return *this;
	}

	bool IsUnitVector() const
	{
		return Utils::ApproximateEqualsF(GetMagnitude(), 1);
	}
	bool IsUniform() const
	{
		if constexpr (std::is_floating_point_v<T>)
			return Utils::ApproximateEqualsF(m_X, m_Y) && Utils::ApproximateEqualsF(m_Y, m_Z);
		return m_X == m_Y && m_Y == m_Z;
	}

	float GetMaxComponentValue() const
	{
		return std::max(std::max(m_X, m_Y), m_Z);
	}
	std::uint8_t GetMaxComponentIndex() const
	{
		if (m_X >= m_Y && m_X >= m_Z)
			return 0;
		if (m_Y >= m_X && m_Y >= m_Z)
			return 1;
		return 2;
	}
	float GetMinComponentValue() const
	{
		return std::min(std::min(m_X, m_Y), m_Z);
	}

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
			LogError(std::format("Tried to convert vector({}, {}, {}) to string "
				"with undefined form {}", m_X, m_Y, m_Z, ::ToString(form)));
			break;
		}
		return str;
	}

	T& operator[](const size_t index)
	{
		ENGINE_ASSERT(index < 3, "Invalid vec3 index:{}", index);
		return m_Components[index];
	}
	const T& operator[](const size_t index) const
	{
		ENGINE_ASSERT(index < 3, "Invalid vec3 index:{}", index);
		return m_Components[index];
	}

	constexpr Vec operator+(const Vec& otherVec) const
	{
		return Vec{ m_X + otherVec.m_X, m_Y + otherVec.m_Y, m_Z + otherVec.m_Z };
	}
	template<typename OtherT, size_t OTHER_ALIGN>
	requires (std::is_arithmetic_v<OtherT>)
	constexpr auto operator+(const Vec<OtherT, 3>& otherVec) -> Vec<std::common_type_t<OtherT, T>, 3>
	{
		using CommonType = std::common_type_t<OtherT, T>;
		return Vec<CommonType, 3>(m_X + otherVec.m_X, m_Y + otherVec.m_Y, m_Z + otherVec.m_Z);
	}


	Vec& operator+=(const Vec& other)
	{
		m_X += other.m_X;
		m_Y += other.m_Y;
		m_Z += other.m_Z;
		return *this;
	}

	constexpr Vec operator-() const
	{
		return Vec{ -m_X, -m_Y, -m_Z };
	}
	constexpr Vec operator-(const Vec& otherVec) const
	{
		return Vec{ m_X - otherVec.m_X, m_Y - otherVec.m_Y, m_Z - otherVec.m_Z };
	}
	template<typename OtherT, size_t OTHER_ALIGN>
	requires (std::is_arithmetic_v<OtherT>)
	constexpr Vec operator-(const Vec<OtherT, 3, OTHER_ALIGN>& otherVec)
	{
		return Vec(m_X - otherVec.m_X, m_Y - otherVec.m_Y, m_Z - otherVec.m_Z);
	}
	Vec& operator-=(const Vec& other)
	{
		m_X -= other.m_X;
		m_Y -= other.m_Y;
		m_Z -= other.m_Z;
		return *this;
	}

	constexpr Vec operator*(const Vec& otherVec) const
	{
		return Vec{ m_X * otherVec.m_X, m_Y * otherVec.m_Y, m_Z * otherVec.m_Z };
	}
	template<typename OtherT, size_t OTHER_ALIGN>
	requires (std::is_arithmetic_v<OtherT>)
	constexpr Vec operator*(const Vec<OtherT, 3, OTHER_ALIGN>& otherVec)
	{
		return Vec(m_X * otherVec.m_X, m_Y * otherVec.m_Y, m_Z * otherVec.m_Z);
	}

	constexpr Vec operator*(const float scalar) const
	{
		return Vec(m_X * scalar, m_Y * scalar, m_Z * scalar);
	}
	constexpr Vec operator*(const double scalar) const
	{
		return Vec(m_X * scalar, m_Y * scalar, m_Z * scalar);
	}
	constexpr Vec operator*(const int scalar) const
	{
		return Vec{ m_X * scalar, m_Y * scalar, m_Z * scalar };
	}
	Vec& operator*=(const float scalar) noexcept
	{
		m_X *= scalar;
		m_Y *= scalar;
		m_Z *= scalar;
		return *this;
	}
	template<typename OtherT, size_t OTHER_ALIGN>
	requires (std::is_arithmetic_v<OtherT>)
	Vec& operator*=(const Vec<OtherT, 3, OTHER_ALIGN>& other) noexcept
	{
		m_X *= other.m_X;
		m_Y *= other.m_Y;
		m_Z *= other.m_Z;
		return *this;
	}

	Vec operator/(const Vec& other) const
	{
		if constexpr (std::is_floating_point_v<T>)
		{
			ENGINE_ASSERT(!Utils::ApproximateEqualsF(other.m_X, 0) && !Utils::ApproximateEqualsF(other.m_Y, 0) && !Utils::ApproximateEqualsF(other.m_Z, 0),
				"Tried to divide a vec3: {} by a 0 value vector:{}", ToString(), other.ToString());
		}
		else
		{
			ENGINE_ASSERT(other.m_X != 0 && other.m_Y != 0 && other.m_Z != 0,
				"Tried to divide a vec3: {} by a 0 value vector:{}", ToString(), other.ToString());
		}

		return Vec{ m_X / other.m_X, m_Y / other.m_Y, m_Z / other.m_Z };
	}
	Vec operator/(const float scalar) const
	{
		//ENGINE_ASSERT(!Utils::ApproximateEqualsF(scalar, 0), "Tried to divide a vec3: {} by a 0 value float scalar", ToString());
		return Vec(m_X / scalar, m_Y / scalar, m_Z / scalar);
	}
	constexpr Vec operator/(const int scalar) const
	{
		ENGINE_ASSERT(scalar != 0, "Tried to divide a vec3: {} by a 0 value int scalar", ToString());
		return Vec{ m_X / scalar, m_Y / scalar, m_Z / scalar };
	}
	Vec& operator/=(const float scalar)
	{
		*this = *this / scalar;
		return *this;
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

	bool AnyAxisGreaterThan(const Vec& other) const
	{
		return m_X > other.m_X || m_Y > other.m_Y || m_Z > other.m_Z;
	}
	bool AnyAxisLessThan(const Vec& other) const
	{
		return m_X < other.m_X || m_Y < other.m_Y || m_Z < other.m_Z;
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

template<typename TScalar, typename TVec>
requires (std::is_arithmetic_v<TScalar> && std::is_arithmetic_v<TVec>)
constexpr auto operator+(TScalar lhs, const Vec<TVec, 3>& rhs) -> Vec<std::common_type_t<TScalar, TVec>, 3>
{
	using CommonType = std::common_type_t<TScalar, TVec>;
	return Vec<CommonType, 3>(lhs + rhs.m_X, lhs + rhs.m_Y, lhs + rhs.m_Z);
}

template<typename TScalar, typename TVec>
requires (std::is_arithmetic_v<TScalar> && std::is_arithmetic_v<TVec>)
constexpr auto operator-(TScalar lhs, const Vec<TVec, 3>& rhs) -> Vec<std::common_type_t<TScalar, TVec>, 3>
{
	//NOTE: using common type trait allows us to get correct type:
	//float - vec<int> -> vec<float>, int - vec<float> -> vec<float>, double - vec<float> -> vec<double>
	using CommonType = std::common_type_t<TScalar, TVec>;
	return Vec<CommonType, 3>(lhs - rhs.m_X, lhs - rhs.m_Y, lhs - rhs.m_Z);
}

template<typename TScalar, typename TVec>
requires (std::is_arithmetic_v<TScalar> && std::is_arithmetic_v<TVec>)
constexpr auto operator*(TScalar lhs, const Vec<TVec, 3>& rhs) -> Vec<std::common_type_t<TScalar, TVec>, 3>
{
	using CommonType = std::common_type_t<TScalar, TVec>;
	return Vec<CommonType, 3>(lhs * rhs.m_X, lhs * rhs.m_Y, lhs * rhs.m_Z);
}

template<typename TScalar, typename TVec>
requires (std::is_arithmetic_v<TScalar> && std::is_arithmetic_v<TVec>)
constexpr auto operator/(TScalar lhs, const Vec<TVec, 3>& rhs) -> Vec<std::common_type_t<TScalar, TVec>, 3>
{
	if constexpr (std::is_floating_point_v<TVec>)
	{
		ENGINE_ASSERT(!Utils::ApproximateEqualsF(rhs.m_X, 0) && !Utils::ApproximateEqualsF(rhs.m_Y, 0)
			&& !Utils::ApproximateEqualsF(rhs.m_Z, 0),
			"Tried to divide a scalar:{} by a 0-value vec3: {}", lhs, rhs.ToString());
	}
	else
	{
		ENGINE_ASSERT(rhs.m_X != 0 && rhs.m_Y != 0 && rhs.m_Z != 0,
			"Tried to divide a scalar:{} by a 0-value vec3: {}", lhs, rhs.ToString());
	}

	//NOTE: using common type trait allows us to get correct type:
	//float - vec<int> -> vec<float>, int - vec<float> -> vec<float>, double - vec<float> -> vec<double>
	using CommonType = std::common_type_t<TScalar, TVec>;
	return Vec<CommonType, 3>(lhs / rhs.m_X, lhs / rhs.m_Y, lhs / rhs.m_Z);
}

using Vec3 = Vec<float, 3>;
using Vec3Int = Vec<int, 3>;
using Vec3Uint = Vec<std::uint32_t, 3>;

extern template class Vec<float, 3>;
extern template class Vec<int, 3>;

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

template<typename T>
Vec<T, 3> Lerp(const Vec<T, 3>& vec0, const Vec<T, 3>& vec1, const float a)
{
	return Vec<T, 3>(std::lerp(vec0.m_X, vec1.m_X, a), std::lerp(vec0.m_Y, vec1.m_Y, a), 
		std::lerp(vec0.m_Z, vec1.m_Z, a));
}

/// <summary>
/// Return true if the point is wtihin the bounding volume formed by the min (forwardmost bottom left pos)
/// and the max (backmost top right pos)
/// </summary>
/// <param name="point"></param>
/// <param name="min"></param>
/// <param name="max"></param>
/// <returns></returns>
template<typename T>
bool IsPointWithinBounds(const Vec<T, 3>& point, const Vec<T, 3>& min, const Vec<T, 3>& max) {
	return (point.m_X >= min.m_X && point.m_X <= max.m_X) &&
		   (point.m_Y >= min.m_Y && point.m_Y <= max.m_Y) &&
		   (point.m_Z >= min.m_Z && point.m_Z <= max.m_Z);
}