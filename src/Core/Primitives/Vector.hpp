#pragma once
#include <cstdint>
#include <string>
#include "Utils/Math.hpp"
#include "Core/Primitives/VecBase.hpp"
#include "Utils/Data/Enums.hpp"

namespace Engine
{
	using Vec2 = Vec<float, 2>;
	using Vec3 = Vec<float, 3>;
	using Vec4 = Vec<float, 4>;

	using Vec2Int = Vec<int, 2>;
	using Vec3Int = Vec<int, 3>;
	using Vec4Int = Vec<int, 4>;

	using Vec2UInt = Vec<std::uint32_t, 2>;
	using Vec3UInt = Vec<std::uint32_t, 3>;
	using Vec4UInt = Vec<std::uint32_t, 4>;
}
namespace Engine::Math
{
	enum class VectorForm : std::uint8_t
	{
		Component = 0,
		Unit = 1,
		MagnitudeDirection = 2,
	};

	std::string ToString(const VectorForm& mode);


	template<typename T, size_t N>
	bool IsUnitVector(const Vec<T, N>& vec) { return ::Math::ApproximateEqualsF(vec.GetMagnitude(), 1); }

	template<typename T>
	float GetAngle(const Vec<T, 2>& vec, const AngleMode angleMode)
	{
		//Just in case to prevent implementations returning undefined
		if (::Math::ApproximateEqualsF(vec.GetMagnitude(), 0))
			return 0;

		float rad = std::atan2(vec.m_Y, vec.m_X);
		//Since atan2 gives you result in [-pi/2, pi/2) we can flip negaative rads
		//to their positive by doing full rotation other way

		//TODO: i think this is wrong because we get negative angles in both the second and fourth quadrants
		//which would not make sense to add 2pi to second quadrant angle. Also, this means all are in terms of -90, 90
		//which would also make the third quadrant wrong as well
		if (rad < 0) rad += 2 * std::numbers::pi;
		if (angleMode == AngleMode::Degrees)
			return ::Math::ToDegrees(rad);

		return rad;
	}

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
	template<typename T>
	T DotProduct(const Vec<T, 4>& vecA, const Vec<T, 4>& vecB)
	{
		return (vecA.m_X * vecB.m_X) + (vecA.m_Y * vecB.m_Y) + (vecA.m_Z * vecB.m_Z) + (vecA.m_W * vecB.m_W);
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

	template<typename T, size_t N>
		requires std::is_arithmetic_v<T>
	T Sum(const Vec<T, N>& vec)
	{
		T result = 0;
		for (size_t i=0; i<N; i++)
		{
			result += vec[i];
		}
		return result;
	}

	template<typename AxisT, size_t N, size_t ALIGN, typename... OtherT>
		requires ::Utils::AllSameType<Vec<AxisT, N, ALIGN>, OtherT...>
	AxisT MaxAxis(const std::uint8_t axisIndex, const Vec<AxisT, N, ALIGN>& first, const OtherT& ...other)
	{
		return ::Math::Max(first[axisIndex], other[axisIndex]...);
	}
	/// <summary>
	/// Creates a vector with a MAX component value from all vectors in args for all axes
	/// </summary>
	/// <typeparam name="AxisT"></typeparam>
	/// <typeparam name="...OtherT"></typeparam>
	/// <typeparam name="N"></typeparam>
	/// <param name="first"></param>
	/// <param name="...other"></param>
	/// <returns></returns>
	template<typename AxisT, size_t N, size_t ALIGN, typename... OtherT>
		requires ::Utils::AllSameType<Vec<AxisT, N, ALIGN>, OtherT...>
	Vec<AxisT, N, ALIGN> Max(const Vec<AxisT, N, ALIGN>& first, const OtherT& ...other)
	{
		Vec<AxisT, N> result = {};
		for (std::uint8_t i = 0; i < N; i++)
		{
			result[i] = ::Math::Max(first[i], other[i]...);
		}
		return result;
	}

	template<typename T, size_t N, size_t ALIGN>
	T MaxVal(const Vec<T, N, ALIGN>& vec)
	{
		T max = vec[0];
		for (std::uint8_t i = 1; i < N; i++)
		{
			max = std::max(max, vec[i]);
		}
		return max;
	}

	template<typename AxisT, size_t N, size_t ALIGN, typename... OtherT>
		requires ::Utils::AllSameType<Vec<AxisT, N, ALIGN>, OtherT...>
	AxisT MinAxis(const std::uint8_t axisIndex, const Vec<AxisT, N, ALIGN>& first, const OtherT& ...other)
	{
		return ::Math::Min(first[axisIndex], other[axisIndex]...);
	}
	/// <summary>
	/// Creates a vector with a MIN component value from all vectors in args for all axes
	/// </summary>
	/// <typeparam name="AxisT"></typeparam>
	/// <typeparam name="...OtherT"></typeparam>
	/// <typeparam name="N"></typeparam>
	/// <param name="first"></param>
	/// <param name="...other"></param>
	/// <returns></returns>
	template<typename AxisT, size_t N, size_t ALIGN, typename... OtherT>
		requires ::Utils::AllSameType<Vec<AxisT, N, ALIGN>, OtherT...>
	Vec<AxisT, N, ALIGN> Min(const Vec<AxisT, N, ALIGN>& first, const OtherT& ...other)
	{
		Vec<AxisT, N, ALIGN> result = {};
		for (std::uint8_t i = 0; i < N; i++)
		{
			result[i] = ::Math::Min(first[i], other[i]...);
		}
		return result;
	}

	template<typename T, size_t N, size_t ALIGN>
	T MinVal(const Vec<T, N, ALIGN>& vec)
	{
		T min = vec[0];
		for (std::uint8_t i = 1; i < N; i++)
		{
			min = std::min(min, vec[i]);
		}
		return min;
	}

	template<typename T>
	Vec<T, 2> Abs(const Vec<T, 2>& vec)
	{
		return Vec<T, 2>(std::abs(vec.m_X), std::abs(vec.m_Y));
	}
	template<typename T>
	Vec<T, 3> Abs(const Vec<T, 3>& vec)
	{
		return Vec<T, 3>(std::abs(vec.m_X), std::abs(vec.m_Y), std::abs(vec.m_Z));
	}

	template<typename T>
	Vec2Int GetSign(const Vec<T, 2>& vec)
	{
		return Vec2(::Math::GetSign(vec.m_X), ::Math::GetSign(vec.m_Y));
	}
	template<typename T>
	Vec3Int GetSign(const Vec<T, 3>& vec)
	{
		return Vec3Int(::Math::GetSign(vec.m_X), ::Math::GetSign(vec.m_Y), ::Math::GetSign(vec.m_Z));
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

	inline Vec2 GetDirVector(const float radianAngle)
	{
		return Vec2(std::cosf(radianAngle), std::sinf(radianAngle));
	}
	
	template<typename T>
	Vec<T, 3> Round(const Vec<T, 3>& vec, const std::uint8_t& decimalPlaces = 5)
	{
		return Vec<T, 3>(::Math::Roundf(vec.m_X, decimalPlaces),
			::Math::Roundf(vec.m_Y, decimalPlaces), ::Math::Roundf(vec.m_Z, decimalPlaces));
	}

	template<typename T>
	Vec<T, 3> Lerp(const Vec<T, 3>& vec0, const Vec<T, 3>& vec1, const float a)
	{
		return Vec<T, 3>(std::lerp(vec0.m_X, vec1.m_X, a), std::lerp(vec0.m_Y, vec1.m_Y, a),
			std::lerp(vec0.m_Z, vec1.m_Z, a));
	}
}