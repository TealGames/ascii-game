#include "Plane.hpp"
#include "Utils/Math/Math.hpp"

Vec2 Plane2D::GetSize() const
{
	return m_MaxPos - m_MinPos;
}

Vec3 Plane3D::GetSize() const
{
	return m_MaxPos - m_MinPos;
}

Vec3 InfinitePlane3D::GetNormal() const
{
	return m_Equation.GetXYZ();
}
Vec3 InfinitePlane3D::GetShortestVectorToOrigin() const
{
	//Note: we do -d (w of vec4) because if d>0: normal points away from origin
	//but if d<0: we need to offset the -d
	return GetNormal() * -m_Equation.m_W;
}
float InfinitePlane3D::GetPointDistanceFromPlane(const Vec3& pos) const
{
	return Sum(GetNormal() * pos) + m_Equation.m_W;
}
bool InfinitePlane3D::IsPointOnPlane(const Vec3& pos) const
{
	return Utils::ApproximateEqualsF(GetPointDistanceFromPlane(pos), 0);
}
bool InfinitePlane3D::IsPointInFrontOfPlane(const Vec3& pos) const
{
	return GetPointDistanceFromPlane(pos) > 0;
}
bool InfinitePlane3D::IsPointBackOfPlane(const Vec3& pos) const
{
	return GetPointDistanceFromPlane(pos) < 0;
}

void InfinitePlane3D::NormalizeNormal()
{
	m_Equation /= m_Equation.GetXYZ().GetMagnitude();
}

std::string InfinitePlane3D::ToString() const
{
	return std::format("[InfPlane3D eq:{}]", m_Equation.ToString());
}

InfinitePlane3D NormalizePlaneNormal(const InfinitePlane3D& plane)
{
	return InfinitePlane3D(plane.m_Equation / plane.m_Equation.GetXYZ().GetMagnitude());
}