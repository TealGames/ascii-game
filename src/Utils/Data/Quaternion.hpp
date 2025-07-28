#pragma once
#include "Utils/Data/Vec3Type.hpp"
#include "Utils/Data/Matrix.hpp"
#include <string>
#include <cstdint>

enum class AngleType : std::uint8_t
{
	Euler		= 0,
	Quaternion	= 1
};

/// <summary>
/// Represents rotations in 4d space -> better than vec3 because rotations are consistent and no gimbal lock
/// This implementation uses the Tait-Bryan angle sequence (ZYX)
/// Note: quaternions do NOT rotate angles they only represent angles and can apply them to vectors.
/// If you want to go from euler angle to quaternion, you have to go through complex conversions.
/// </summary>
class Quat
{
private:
public:
	float m_X;
	float m_Y;
	float m_Z;
	float m_W;

private:
public:
	constexpr Quat() : Quat(Identity()) {}
	constexpr Quat(const float x, const float y, const float z, const float w) :
		m_X(x), m_Y(y), m_Z(z), m_W(w) {}
	constexpr Quat(const Vec3& radianEulerAngles) : Quat(ToQuaternion(radianEulerAngles)) {}

	static inline constexpr Quat Identity()
	{
		return Quat(0, 0, 0, 1);
	}

	Vec3 ToEulerAngles() const;
	static constexpr Quat ToQuaternion(const Vec3& radianEulerAngles);

	/// <summary>
	/// Applies this rotation to the vec3 direction. Note: this does NOT convert 
	/// to euler angles and does NOT apply this rotation to existing rotation.
	/// What it does is rotate the direction vector by the rotation stored in this quaternion
	/// 
	/// Note: this only works with UNIT VECTORS and POINTS in space
	/// </summary>
	/// <param name="direction"></param>
	/// <returns></returns>
	Vec3 ApplyRotationToDir(const Vec3& direction) const;

	Quat operator*(const Quat& other) const;
	Quat& operator*=(const Quat& other);

	std::string ToString(const AngleType angleType = AngleType::Euler) const;
};