#pragma once
#include "Utils/Data/WorldPosition.hpp"
#include <string>
#include "Utils/Data/NormalizedPosition.hpp"

struct AABB2D
{
	WorldPosition2D m_MinPos;
	WorldPosition2D m_MaxPos;

	AABB2D();
	AABB2D(const Vec2& size);
	AABB2D(const WorldPosition2D& minPos, const WorldPosition2D& maxPos);

	WorldPosition2D GetGlobalMin(const WorldPosition2D& centerWorldPos) const;
	WorldPosition2D GetGlobalMax(const WorldPosition2D& centerWorldPos) const;

	/// <summary>
	/// Gets the size in [WIDTH, HEIGHT]
	/// </summary>
	/// <returns></returns>
	Vec2 GetSize() const;

	/// <summary>
	/// Gets the half size in [WIDTH/2, HEIGHT/2]
	/// </summary>
	/// <returns></returns>
	Vec2 GetHalfExtent() const;

	/// <summary>
	/// Will get the AABB pos based on the relative pos of the AABB
	/// where (0,0) is bottom left and (1, 1) is top right
	/// </summary>
	/// <param name="relativePos"></param>
	/// <returns></returns>
	WorldPosition2D GetWorldPos(const WorldPosition2D& centerPos, const NormalizedPosition& relativePos) const;

	std::string ToString() const;
	std::string ToString(const WorldPosition2D& transformPos) const;
};

struct AABB3D
{
	WorldPosition3D m_MinPos;
	WorldPosition3D m_MaxPos;

	AABB3D();
	AABB3D(const Vec3& size);
	AABB3D(const WorldPosition3D& minPos, const WorldPosition3D& maxPos);

	WorldPosition3D GetGlobalMin(const WorldPosition3D& centerWorldPos) const;
	WorldPosition3D GetGlobalMax(const WorldPosition3D& centerWorldPos) const;

	/// <summary>
	/// Gets the size in [WIDTH, HEIGHT]
	/// </summary>
	/// <returns></returns>
	Vec3 GetSize() const;

	/// <summary>
	/// Gets the half size in [WIDTH/2, HEIGHT/2]
	/// </summary>
	/// <returns></returns>
	Vec3 GetHalfExtent() const;

	std::string ToString() const;
	std::string ToString(const WorldPosition3D& transformPos) const;
};


