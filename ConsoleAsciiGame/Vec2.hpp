#pragma once
#include <string>
#include "VectorEnums.hpp"

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

	static const Vec2 NORTH;
	static const Vec2 NORTHEAST;
	static const Vec2 EAST;
	static const Vec2 SOUTHEAST;
	static const Vec2 SOUTH;
	static const Vec2 SOUTHWEST;
	static const Vec2 WEST;
	static const Vec2 NORTHWEST;

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

	int XAsInt() const;
	int YAsInt() const;

	Vec2 GetFlipped() const;

	/// <summary>
	/// Returns the angle of the vector by default in radians (or degrees)
	/// Examples: <0, -1> returns 
	/// </summary>
	/// <param name="mode"></param>
	/// <returns></returns>
	float GetAngle(const AngleMode& mode = AngleMode::Radians) const;
	float GetMagnitude() const;
	/// <summary>
	/// Returns only the direction of the vector
	/// </summary>
	/// <returns></returns>
	Vec2 GetNormalized() const;
	Vec2 GetNormal() const;
	Vec2 GetOppositeDirection() const;

	bool IsUnitVector() const;

	std::string ToString(const std::uint8_t& decimalPlaces = 5, 
		const VectorForm form = VectorForm::Component) const;

	Vec2 operator+(const Vec2& other) const;
	Vec2& operator+=(const Vec2& other);
	Vec2 operator-(const Vec2& other) const;
	Vec2& operator-=(const Vec2& other);
	Vec2 operator*(const Vec2& other) const;
	Vec2 operator*(const float& scalar) const;
	Vec2 operator/(const Vec2& other) const;
	Vec2 operator/(const float& scalar) const;

	bool operator==(const Vec2& other) const;
	bool operator!=(const Vec2& other) const;
	bool operator>(const Vec2& other) const;
	bool operator>=(const Vec2& other) const;
	bool operator<(const Vec2& other) const;
	bool operator<=(const Vec2& other) const;

	Vec2& operator=(const Vec2& other);
	Vec2& operator=(Vec2&& other) noexcept;
};

float GetDistance(const Vec2& vec1, const Vec2& vec2);
float GetXComponent(const float& speed, const float& angle, const AngleMode& mode = AngleMode::Radians);
float GetYComponent(const float& speed, const float& angle, const AngleMode& mode = AngleMode::Radians);

/// <summary>
/// Will get the vector formed by the 2 points
/// </summary>
/// <param name="startPos"></param>
/// <param name="endPos"></param>
/// <returns></returns>
Vec2 GetVector(const Vec2& startPos, const Vec2& endPos);
Vec2 GetVector(const Vec2& unitVector, const float& magnitude);
Vec2 GetVector(const float& speed, const float& angle, const AngleMode& mode = AngleMode::Radians);

Vec2 GetVectorEndPoint(const Vec2& startPos, const Vec2& vector);

//	Returns the scalar dot product. 
// -> This essentially finds how much one vector is aligned in terms of the other (where order does NOT matter and produces the same result)
// -> If one vector is a unit vector, it essnetially then can find how much of a vector is in the same direction as the unit vector
// -> THIS IS MOST USEFUL FOR GETTING THE SCALAR FOR VECTORS IN PARALLEL DIRECTIONS (and extracting the parts in that direction)
// -> Note: <0 result means they are in opposite directions, >0 means they are <90 degrees from each other and =0 means they are at 90 degrees
float DotProduct(const Vec2& vecA, const Vec2& vecB);

Vec2 Abs(const Vec2& vec);

Vec2 GenerateRandomVec2(const Vec2& minVec, const Vec2 maxVec);
Vec2 GenerateRandomDir();

