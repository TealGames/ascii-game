#pragma once
#include "Utils/DataStructure/BVH.hpp"

bool BVHFlatNode::IsLeaf() const { return m_ObjectCount > 0 && m_ObjectStartIndex >= 0 && m_IndexChild0 == -1 && m_IndexChild1 == -1; }
bool BVHFlatNode::IsInvalid() const { return m_ObjectCount == 0 && m_ObjectStartIndex == -1 && m_IndexChild0 == -1 && m_IndexChild1 == -1; }
bool BVHFlatNode::IsIntersectedByRay(const WorldPosition3D& rayOrigin, const WorldPosition3D& inverseDir, float* outMargin) const
{
	return Utils::RayIntersectsBoundsInverseDir(
		ConvertAlignment<AABB3D_Align16::GetAlignment(), AABB3D::GetAlignment()>(m_Bounds), rayOrigin, inverseDir, outMargin);
}
bool BVHFlatNode::IsIntersectedByRay(const WorldPosition3D& rayOrigin, const WorldPosition3D& inverseDir,
	float* outTEnter, float* outTExit) const
{
	return Utils::RayIntersectsBoundsInverseDir(
		ConvertAlignment<AABB3D_Align16::GetAlignment(), AABB3D::GetAlignment()>(m_Bounds), rayOrigin, inverseDir, outTEnter, outTEnter);
}

AABB3D BVHFlatNode::GetAABB() const
{
	return ConvertAlignment<AABB3D_Align16::GetAlignment(), 
		AABB3D::GetAlignment()>(m_Bounds); 
}
WorldPosition3D BVHFlatNode::GetCenter() const
{
	return m_Bounds.GetCenter();
}

std::string BVHFlatNode::ToString(const bool fullData) const
{
	return std::format("[Bounds:{}{}]", m_Bounds.ToString(),
		(fullData) ? std::format("C0:{} C1:{} L:{}{}", m_IndexChild0, m_IndexChild1, IsLeaf(),
			(IsLeaf()? std::format("ObjStart:{} ObjCount:{}", m_ObjectStartIndex, m_ObjectCount) : "")) : "");
}

