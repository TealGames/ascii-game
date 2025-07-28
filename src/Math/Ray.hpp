#pragma once
#include "Utils/Data/WorldPosition.hpp"

class Ray2
{
private:
public:
	WorldPosition2D m_Origin;
	Vec2 m_Length;

private:
public:
	Ray2();
	Ray2(const WorldPosition2D& pos, const Vec2& length);

	WorldPosition2D GetEndpoint() const;
	Vec2 GetDir() const;
};

class Ray3
{
private:
public:
	WorldPosition3D m_Origin;
	Vec3 m_Length;

private:
public:
	Ray3();
	Ray3(const WorldPosition3D& pos, const Vec3& length);

	WorldPosition3D GetEndpoint() const;
	Vec3 GetDir() const;
};

