#pragma once
#include "Utils/Data/WorldPosition.hpp"

class Ray2D
{
private:
public:
	WorldPosition2D m_Origin;
	Vec2 m_Length;

private:
public:
	Ray2D();
	Ray2D(const WorldPosition2D& pos, const Vec2& length);

	WorldPosition2D GetEndpoint() const;
	Vec2 GetDir() const;
};

class Ray3D
{
private:
public:
	WorldPosition3D m_Origin;
	Vec3 m_Length;

private:
public:
	Ray3D();
	Ray3D(const WorldPosition3D& pos, const Vec3& length);

	WorldPosition3D GetEndpoint() const;
	Vec3 GetDir() const;
};

