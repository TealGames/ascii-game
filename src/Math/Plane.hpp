#pragma once
#include "Utils/Data/Vec4Type.hpp"

/// <summary>
/// Represents a 2d plane comprised of a min and max pos
/// </summary>
struct Plane2D
{
	/// <summary>
	/// The bottom left point
	/// </summary>
	Vec2 m_MinPos = {};
	/// <summary>
	/// The top right point
	/// </summary>
	Vec2 m_MaxPos = {};

	Vec2 GetSize() const;
};

/// <summary>
/// Represents a 3d plane comprised of a min and max pos
/// </summary>
struct Plane3D
{
	/// <summary>
	/// The forwardmost bottom left pos
	/// </summary>
	Vec3 m_MinPos;
	/// <summary>
	/// The backmost top right pos
	/// </summary>
	Vec3 m_MaxPos;

	Vec3 GetSize() const;
};

/// <summary>
/// Represents an infinite 3d plane meaning it has no
/// definitive location, and only exists in relation/direction to another
/// object or reference point (usually the world origin)
/// </summary>
struct InfinitePlane3D
{
	/// <summary>
	/// The equation for the plane where (x, y, z) is the unit vector normal of the plane
	/// and w is the signed offset from the origin to the plan in the direction of the normal
	/// (basically the shortest distance from the origin to the plane)
	/// 
	/// In math terms, we use the equation ax + by + cz + d=0, where d is the distance to origin
	/// and (a, b, c) is the normal.
	/// We can also use the equation n-> * p-> + d =0 (where n-> is the normal vector, p-> is a point on plane)
	/// to represent the plane
	/// d > 0: the plane is in direction of normal (normal points AWAY from origin)
	/// d < 0: the plane is opposite the normal (normal points TOWARD the origin)
	/// d =0 : plane passes through origin
	/// </summary>
	Vec4 m_Equation;

	Vec3 GetNormal() const;
	Vec3 GetShortestVectorToOrigin() const;
	/// <summary>
	/// Gets the signed distance of the point from the plane.
	/// Note: front is considered the side that the normal points
	/// d < 0: behind plane
	/// d = 0: on the plane
	/// d > 0: in front of plane
	/// </summary>
	/// <param name="pos"></param>
	/// <returns></returns>
	float GetPointDistanceFromPlane(const Vec3& pos) const;
	/// <summary>
	/// Returns true if the distance from plane is 0
	/// </summary>
	/// <param name="pos"></param>
	/// <returns></returns>
	bool IsPointOnPlane(const Vec3& pos) const;
	bool IsPointInFrontOfPlane(const Vec3& pos) const;
	bool IsPointBackOfPlane(const Vec3& pos) const;

	void NormalizeNormal();

	std::string ToString() const;
};

/// <summary>
/// Normalizes the normal stored in the equation
/// </summary>
InfinitePlane3D NormalizePlaneNormal(const InfinitePlane3D& plane);