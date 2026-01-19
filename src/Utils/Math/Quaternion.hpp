#pragma once
#include "Utils/Math/Matrix.hpp"
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
	Quat(const Vec3& radianEulerAngles);
	Quat(const Mat3& matrix);

	static inline constexpr Quat Identity()
	{
		return Quat(0, 0, 0, 1);
	}

	Vec4 AsVec4() const;
	const float* GetMemPointer() const;

	Quat GetNormalized() const;
	void Normalize();

	Vec3 ToRadians() const;
	Vec3 ToDegrees() const;

	void SetAsRadians(const Vec3& radianEulerAngle);
	void SetAsDegrees(const Vec3& degreeEulerAngle);

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

	/// <summary>
	/// Applies a rotation to this quaternion using euler angles in radians.
	/// Note: while you can use SetRad(ToRad() + radDelta) for updating rotation
	/// it is best to use the direct quaternion, which is used here.
	/// Note: Multiplication is what applies rotations to quaternions not addition
	/// </summary>
	/// <param name="radianEulerAngle"></param>
	/// <returns></returns>
	Quat operator*(const Vec3& radianEulerAngle) const;
	Quat& operator*=(const Vec3& radianEulerAngle);

	std::string ToString(const AngleType angleType = AngleType::Euler) const;
};

void FromRadianEulerAngles(Quat& quaternion, const Vec3& radianEulerAngles);
/// <summary>
/// Will set the quaternion from the matrix 
/// NOTE: each COL MUST be NORMALIZED
/// </summary>
/// <param name="quaternion"></param>
/// <param name="matrix"></param>
void FromRotationMatrix(Quat& quaternion, const Mat3& matrix);
/// <summary>
/// Will set the quaternion by applying a radian rotation around the given axis
/// </summary>
/// <param name="axis"></param>
/// <param name="radianAmount"></param>
/// <returns></returns>
void FromAxisAngle(Quat& quaternion, const Vec3& axis, const float radianRotation);

Quat ToQuaternion(const Vec3& radianEulerAngles);
Quat ToQuaternion(const Mat3& matrix);
Quat ToQuaternion(const Vec3& axis, const float radianRotation);