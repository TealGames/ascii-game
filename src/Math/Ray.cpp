#include "Math/Ray.hpp"

Ray2::Ray2() : Ray2(WorldPosition2D(), Vec2::Zero()) {}
Ray2::Ray2(const WorldPosition2D& pos, const Vec2& length) 
	: m_Origin(pos), m_Length(length) {}

WorldPosition2D Ray2::GetEndpoint() const
{
	return m_Origin + m_Length;
}
Vec2 Ray2::GetDir() const
{
	return m_Length.GetNormalized();
}

Ray3::Ray3() : Ray3(WorldPosition3D(), Vec3::Zero()) {}
Ray3::Ray3(const WorldPosition3D& pos, const Vec3& length)
	: m_Origin(pos), m_Length(length) {}

WorldPosition3D Ray3::GetEndpoint() const
{
	return m_Origin + m_Length;
}
Vec3 Ray3::GetDir() const
{
	return m_Length.GetNormalized();
}