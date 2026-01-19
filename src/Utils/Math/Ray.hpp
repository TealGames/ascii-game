#pragma once
#include "Utils/Math/WorldPosition.hpp"

class Ray2D
{
private:
public:
	Vec2 m_Origin;
	Vec2 m_Dir;

private:
public:
	Ray2D();
	Ray2D(const Vec2& pos, const Vec2& length);

	Vec2 GetEndpoint() const;
	Vec2 GetDir() const;
};

class Ray3D
{
private:
public:
	Vec3 m_Origin;
	Vec3 m_Dir;

private:
public:
	Ray3D();
	Ray3D(const Vec3& pos, const Vec3& length);

	Vec3 GetEndpoint() const;
	Vec3 GetDir() const;
};

