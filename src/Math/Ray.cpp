#include "Math/Ray.hpp"

namespace Engine::Math
{
	Ray2D::Ray2D() : Ray2D(WorldPosition2D(), Vec2::Zero()) {}
	Ray2D::Ray2D(const Vec2& pos, const Vec2& length)
		: m_Origin(pos), m_Dir(length) {}

	Vec2 Ray2D::GetEndpoint() const
	{
		return m_Origin + m_Dir;
	}
	Vec2 Ray2D::GetDir() const
	{
		return m_Dir.GetNormalized();
	}

	Ray3D::Ray3D() : Ray3D(Vec3(), Vec3::Zero()) {}
	Ray3D::Ray3D(const Vec3& pos, const Vec3& length)
		: m_Origin(pos), m_Dir(length) {}

	Vec3 Ray3D::GetEndpoint() const
	{
		return m_Origin + m_Dir;
	}
	Vec3 Ray3D::GetDir() const
	{
		return m_Dir.GetNormalized();
	}
}
