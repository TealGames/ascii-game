#pragma once
#include <string>
#include "VectorEnums.hpp"

class Vec2Int
{
private:
public:
	int m_X;
	int m_Y;

	static const Vec2Int UP;
	static const Vec2Int DOWN;
	static const Vec2Int LEFT;
	static const Vec2Int RIGHT;

	static const Vec2Int NORTH;
	static const Vec2Int EAST;
	static const Vec2Int SOUTH;
	static const Vec2Int WEST;

	static const Vec2Int ZERO;
	static const Vec2Int ONE;

public:
	Vec2Int();
	Vec2Int(const int&, const int&);
	Vec2Int(const Vec2Int&) = default;
	Vec2Int(Vec2Int&&) noexcept = default;

	/// <summary>
	/// Will get the x value as its own vector
	/// </summary>
	/// <returns></returns>
	Vec2Int GetXAsVector() const;

	/// <summary>
	/// Will get the x value as its own vector
	/// </summary>
	/// <returns></returns>
	Vec2Int GetYAsVector() const;

	Vec2Int GetFlipped() const;

	/// <summary>
	/// Returns the angle of the vector by default in radians (or degrees)
	/// Examples: <0, -1> returns 
	/// </summary>
	/// <param name="mode"></param>
	/// <returns></returns>
	float GetAngle(const AngleMode& mode = AngleMode::Radians) const;
	float GetMagnitude() const;
	Vec2Int GetNormalized() const;
	Vec2Int GetNormal() const;
	Vec2Int GetOppositeDirection() const;

	bool IsUnitVector() const;

	std::string ToString(const VectorForm form = VectorForm::Component) const;

	Vec2Int operator+(const Vec2Int& other) const;
	Vec2Int operator-(const Vec2Int& other) const;
	Vec2Int operator*(const Vec2Int& other) const;
	Vec2Int operator*(const float& scalar) const;
	Vec2Int operator/(const float& scalar) const;

	bool operator==(const Vec2Int& other) const;

	Vec2Int& operator=(const Vec2Int& other);
	Vec2Int& operator=(Vec2Int&& other) noexcept;
};

float GetDistance(const Vec2Int& vec1, const Vec2Int& vec2);

/// <summary>
/// Will get the vector formed by the 2 points
/// </summary>
/// <param name="startPos"></param>
/// <param name="endPos"></param>
/// <returns></returns>
Vec2Int GetVector(const Vec2Int& startPos, const Vec2Int& endPos);
Vec2Int GetVector(const Vec2Int& unitVector, const float& magnitude);

Vec2Int GetVectorEndPoint(const Vec2Int& startPos, const Vec2Int& vector);

/// <summary>
/// Returns the scalar dot product. 
/// -> This essentially finds how much one vector is aligned in terms of the other (where order does NOT matter and produces the same result)
/// -> If one vector is a unit vector, it essnetially then can find how much of a vector is in the same direction as the unit vector
/// -> THIS IS MOST USEFUL FOR GETTING THE SCALAR FOR VECTORS IN PARALLEL DIRECTIONS (and extracting the parts in that direction)
/// -> Note: <0 result means they are in opposite directions, >0 means they are <90 degrees from each other and =0 means they are at 90 degrees
/// </summary>
/// <param name="vecA"></param>
/// <param name="vecB"></param>
/// <returns></returns>
float DotProduct(const Vec2Int& vecA, const Vec2Int& vecB);

Vec2Int GenerateRandomVec2Int(const Vec2Int& minVec, const Vec2Int maxVec);
