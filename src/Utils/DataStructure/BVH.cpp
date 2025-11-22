#pragma once
#include "Utils/DataStructure/BVH.hpp"

bool BVHFlatNode::IsLeaf() const { return m_ObjectCount == 0 && m_ObjectStartIndex == -1; }
bool BVHFlatNode::IsIntersectedByRay(const WorldPosition3D& rayOrigin, const WorldPosition3D& inverseDir) const
{
	return Utils::RayIntersectsBoundsInverseDir(m_Bounds, rayOrigin, inverseDir);
}
