#include "Math/Ray.hpp"

Ray2D::Ray2D() : Ray2D(WorldPosition2D(), Vec2::Zero()) {}
Ray2D::Ray2D(const WorldPosition2D& pos, const Vec2& length) 
	: m_Origin(pos), m_Length(length) {}

WorldPosition2D Ray2D::GetEndpoint() const
{
	return m_Origin + m_Length;
}
Vec2 Ray2D::GetDir() const
{
	return m_Length.GetNormalized();
}

Ray3D::Ray3D() : Ray3D(WorldPosition3D(), Vec3::Zero()) {}
Ray3D::Ray3D(const WorldPosition3D& pos, const Vec3& length)
	: m_Origin(pos), m_Length(length) {}

WorldPosition3D Ray3D::GetEndpoint() const
{
	return m_Origin + m_Length;
}
Vec3 Ray3D::GetDir() const
{
	return m_Length.GetNormalized();
}