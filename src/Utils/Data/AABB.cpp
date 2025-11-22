#include "pch.hpp"
#include "Utils/Data/AABB.hpp"

AABB2D::AABB2D() : AABB2D(WorldPosition2D{}, WorldPosition2D{}) {}
AABB2D::AABB2D(const WorldPosition2D& minPos, const WorldPosition2D& maxPos)
	: m_MinPos(minPos), m_MaxPos(maxPos) {}

AABB2D::AABB2D(const Vec2& size) : m_MinPos(size / 2 * -1), m_MaxPos(size / 2) {}

WorldPosition2D AABB2D::GetGlobalMin(const WorldPosition2D& centerWorldPos) const
{
	return centerWorldPos + m_MinPos;
}

WorldPosition2D AABB2D::GetGlobalMax(const WorldPosition2D& centerWorldPos) const
{
	return centerWorldPos + m_MaxPos;
}

Vec2 AABB2D::GetSize() const
{
	return m_MaxPos - m_MinPos;
}

Vec2 AABB2D::GetHalfExtent() const
{
	return GetSize() / 2;
}

WorldPosition2D AABB2D::GetWorldPos(const WorldPosition2D& centerPos, const NormalizedPosition& relativePos) const
{
	Vec2 boundSize = GetSize();
	WorldPosition2D bottomLeftPos = centerPos - WorldPosition2D(boundSize.m_X / 2, boundSize.m_Y / 2);
	if (relativePos.GetPos() == Vec2{ 0, 0 }) return bottomLeftPos;

	return bottomLeftPos + WorldPosition2D(relativePos.GetPos().m_X * boundSize.m_X, relativePos.GetPos().m_Y * boundSize.m_Y);
}

std::string AABB2D::ToString(const WorldPosition2D& transformPos) const
{
	return std::format("[GMin:{} GMax:{} Size: {}]", GetGlobalMin(transformPos).ToString(),
		GetGlobalMax(transformPos).ToString(), GetSize().ToString());
}

std::string AABB2D::ToString() const
{
	return std::format("[Min:{} Max:{} Size: {}]", m_MinPos.ToString(),
		m_MaxPos.ToString(), GetSize().ToString());
}

AABB3D::AABB3D() : AABB3D(WorldPosition3D{}, WorldPosition3D{}) {}
AABB3D::AABB3D(const WorldPosition3D& minPos, const WorldPosition3D& maxPos)
	: m_MinPos(minPos), m_MaxPos(maxPos) {}

AABB3D::AABB3D(const Vec3& size) : m_MinPos(size / 2 * -1), m_MaxPos(size / 2) {}

WorldPosition3D AABB3D::GetGlobalMin(const WorldPosition3D& centerWorldPos) const
{
	return centerWorldPos + m_MinPos;
}

WorldPosition3D AABB3D::GetGlobalMax(const WorldPosition3D& centerWorldPos) const
{
	return centerWorldPos + m_MaxPos;
}

Vec3 AABB3D::GetSize() const
{
	return m_MaxPos - m_MinPos;
}

Vec3 AABB3D::GetHalfExtent() const
{
	return GetSize() / 2;
}

std::string AABB3D::ToString(const WorldPosition3D& transformPos) const
{
	return std::format("[GMin:{} GMax:{} Size: {}]", GetGlobalMin(transformPos).ToString(),
		GetGlobalMax(transformPos).ToString(), GetSize().ToString());
}

std::string AABB3D::ToString() const
{
	return std::format("[Min:{} Max:{} Size: {}]", m_MinPos.ToString(),
		m_MaxPos.ToString(), GetSize().ToString());
}