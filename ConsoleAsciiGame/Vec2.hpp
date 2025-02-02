#pragma once
#include <string>
#include "Point2D.hpp"
#include "Point2DInt.hpp"

enum class AngleMode
{
	Degrees,
	Radians,
};
std::string ToString(const AngleMode& mode);

enum class VectorForm
{
	Component,
	Unit,
	MagnitudeDirection,
};

std::string ToString(const VectorForm& mode);

class Vec2
{
private:
public:
	float m_X;
	float m_Y;

	static const Vec2 UP;
	static const Vec2 DOWN;
	static const Vec2 LEFT;
	static const Vec2 RIGHT;

	static const Vec2 ZERO;
	static const Vec2 ONE;

public:
	Vec2();
	Vec2(const float&, const float&);
	Vec2(const Vec2&) = default;
	Vec2(Vec2&&) noexcept = default;

	/// <summary>
	/// Will get the x value as its own vector
	/// </summary>
	/// <returns></returns>
	Vec2 GetXAsVector() const;

	/// <summary>
	/// Will get the x value as its own vector
	/// </summary>
	/// <returns></returns>
	Vec2 GetYAsVector() const;

	/// <summary>
	/// Returns the angle of the vector by default in radians (or degrees)
	/// Examples: <0, -1> returns 
	/// </summary>
	/// <param name="mode"></param>
	/// <returns></returns>
	float GetAngle(const AngleMode& mode = AngleMode::Radians) const;
	float GetMagnitude() const;
	Vec2 GetNormalized() const;
	Vec2 GetNormal() const;
	Vec2 GetOppositeDirection() const;

	bool IsUnitVector() const;

	std::string ToString(const VectorForm form = VectorForm::MagnitudeDirection) const;

	Vec2 operator+(const Vec2& other) const;
	Vec2 operator-(const Vec2& other) const;
	Vec2 operator*(const Vec2& other) const;
	Vec2 operator*(const float& scalar) const;

	bool operator==(const Vec2& other) const;

	Vec2& operator=(const Vec2& other);
	Vec2& operator=(Vec2&& other) noexcept;
};

/// <summary>
/// Will get the vector formed by the 2 points
/// </summary>
/// <param name="startPos"></param>
/// <param name="endPos"></param>
/// <returns></returns>
Vec2 GetVector(const Utils::Point2D& startPos, const Utils::Point2D& endPos);
Vec2 GetVector(const Utils::Point2DInt& startPos, const Utils::Point2DInt& endPos);
Vec2 GetVector(const Utils::Point2DInt& intVec);
Vec2 GetVector(const Vec2& unitVector, const float& magnitude);

Utils::Point2D GetVectorEndPoint(const Utils::Point2D& startPos, const Vec2& vector);
Utils::Point2DInt GetVectorEndPoint(const Utils::Point2DInt& startPos, const Vec2& vector);

