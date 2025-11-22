#pragma once
#include <cstdint>
#include <algorithm>
#include <concepts>
#include "Utils/Data/WorldPosition.hpp"
#include "Utils/HelperMacros.hpp"
#include "Utils/MathAdvanced.hpp"
#include "Utils/Data/AABB.hpp"

//TODO: implement a linear bvh with morton codes which 
//is better for gpu for having faster reload/reconstruction times when movmeent occurs
struct BVHFlatNode
{
	int m_ObjectStartIndex = - 1;
	std::uint8_t m_ObjectCount = 0;
	AABB3D m_Bounds;

	std::uint32_t m_IndexChild0 = 0;
	std::uint32_t m_IndexChild1 = 0;

	bool IsLeaf() const;
	bool IsIntersectedByRay(const WorldPosition3D& origin, const WorldPosition3D& inverseDir) const;
};

DEFINE_TEMPLATE_HAS_FUNCTION(GetCenter, WorldPosition3D);
DEFINE_TEMPLATE_HAS_FUNCTION(GetAABB, AABB3D);
DEFINE_TEMPLATE_HAS_FUNCTION_NAMED(IsIntersectedByRay);

enum class BVHSplitAlgorithm : std::uint8_t
{
	Median			= 0,
	SurfaceArea		= 1
};

/// <summary>
/// A bounding volume hierarchy which constructs tree by splitting an area's dimensions
/// by the longest axis in half and dividing into two partitions which then continue to
/// get split until reaching leaf node count and storing indices of the primitives
/// </summary>
template<typename T>
requires (HasFunctionGetCenter<T> && HasFunctionGetAABB<T> && std::is_default_constructible_v<T> && 
		  HasFunctionIsIntersectedByRay<T, bool, WorldPosition3D, WorldPosition3D>)
class StaticBVHTree
{
private:
	T* m_objectPtr;
	size_t m_objectCount;
	std::vector<BVHFlatNode> m_flatNodes;
public:

private:
	void CreateSubtree(T* objectPtr, const size_t objectStartIndex, const size_t objectCount, 
		const size_t leafObjectCount, const BVHSplitAlgorithm splitAlgorithm, BVHFlatNode& node, size_t& nextNodeIndex)
	{
		//We know the previous index must have been this node index
		const size_t thisNodeIndex = nextNodeIndex - 1;

		WorldPosition3D maxPos = objectPtr[objectStartIndex].GetCenter();
		WorldPosition3D minPos = maxPos;
		WorldPosition3D objectPos = {};
		std::size_t i;
		for (i = 1; i < objectCount; i++)
		{
			objectPos = objectPtr[objectStartIndex + i].GetCenter();
			maxPos = Max(maxPos, objectPos);
			minPos = Min(minPos, objectPos);
		}

		node.m_Bounds = AABB3D(minPos, maxPos);
		node.m_IndexChild0 = 0;
		node.m_IndexChild1 = 0;
		//If we reach the target (or less) object count per leaf we we
		//set the primitive values for this node and exit recursion
		if (objectCount <= leafObjectCount)
		{
			node.m_ObjectStartIndex = objectStartIndex;
			node.m_ObjectCount = objectCount;
			return;
		}

		node.m_ObjectStartIndex = -1;
		node.m_ObjectCount = 0;

		WorldPosition3D extents = maxPos - minPos;
		std::uint8_t maxAxisIndex = extents.GetMaxComponentIndex();
		
		//We partition the data into smaller and greater partition compared to 
		//the pivot as midpoint (NOTE: this is the same as partition algorithm in quicksort)
		if (splitAlgorithm == BVHSplitAlgorithm::Median)
		{
			float midpoint = (minPos[maxAxisIndex] + maxPos[maxAxisIndex]) * 0.5f;
			T temp = {};

			i = 0;
			std::size_t j = objectCount - 1;
			while (i <= j)
			{
				objectPos = objectPtr[objectStartIndex + i].GetCenter();
				if (objectPos[maxAxisIndex] < midpoint)
					i++;
				else
				{
					temp = objectPtr[objectStartIndex + i];
					objectPtr[objectStartIndex + i] = objectPtr[objectStartIndex + j];
					objectPtr[objectStartIndex + j] = temp;
					j--;
				}
			}
		}
		else if (splitAlgorithm == BVHSplitAlgorithm::SurfaceArea)
		{
			auto compareFunction = [maxAxisIndex](const T& object1, const T& object2) 
				{
					return object1.GetCenter()[maxAxisIndex] < object2.GetCenter()[maxAxisIndex];
				};
			std::sort(objectPtr + objectStartIndex, objectPtr + objectStartIndex + objectCount, compareFunction);

			AABB3D currentAABB = {};
			AABB3D* prefixBounds = (AABB3D*)alloca(sizeof(AABB3D) * (objectCount - 1));
			prefixBounds[0] = objectPtr[objectStartIndex].GetAABB();
			for (i = 1; i < objectCount - 1; i++)
			{
				currentAABB= objectPtr[objectStartIndex + i].GetAABB();
				prefixBounds[i] = AABB3D(Min(prefixBounds[i-1].m_MinPos, currentAABB.m_MinPos), 
										 Max(prefixBounds[i-1].m_MaxPos, currentAABB.m_MaxPos));
			}
			AABB3D* suffixBounds = (AABB3D*)alloca(sizeof(AABB3D) * (objectCount - 1));
			suffixBounds[objectCount -2] = objectPtr[objectStartIndex + objectCount -1].GetAABB();
			for (i = objectCount-3; i >= 0; i--)
			{
				currentAABB = objectPtr[objectStartIndex + i + 1].GetAABB();
				suffixBounds[i] = AABB3D(Min(suffixBounds[i + 1].m_MinPos, currentAABB.m_MinPos),
					Max(suffixBounds[i + 1].m_MaxPos, currentAABB.m_MaxPos));
			}

			float surfaceAreaMinCost = std::numeric_limits<float>::max();
			size_t minSurfaceAreaIndex = 1;
			WorldPosition3D leftMaxPos = minPos;
			WorldPosition3D rightMinPos = minPos;
			for (i = 1; i<objectCount -1; i++)
			{
				const float currentCost = Utils::CalculateSurfaceArea(prefixBounds[i -1]) * i + 
									 Utils::CalculateSurfaceArea(suffixBounds[i]) * (objectCount - i);
				if (currentCost < surfaceAreaMinCost)
				{
					minSurfaceAreaIndex = i;
					surfaceAreaMinCost = currentCost;
				}
			}
		}

		
		//We then setup the left and right children nodes and invoke next subtree
		//with the left and right partioned segments
		node.m_IndexChild0 = nextNodeIndex;
		nextNodeIndex++;
		CreateSubtree<T>(objectPtr, objectStartIndex, i, leafObjectCount,
			m_flatNodes[node.m_IndexChild0], nextNodeIndex);

		node.m_IndexChild0 = nextNodeIndex;
		nextNodeIndex++;
		CreateSubtree<T>(objectPtr, objectStartIndex + i, objectCount - i, leafObjectCount,
			m_flatNodes[node.m_IndexChild1], nextNodeIndex);
	}

public:
	StaticBVHTree() : m_objectPtr(nullptr), m_objectCount(0), m_flatNodes() {}

	const std::vector<BVHFlatNode>& ConstructFrom(const T* objectPtr, const size_t objectCount, const size_t leafObjectCount)
	{
		m_objectPtr = objectPtr;
		m_objectCount = objectCount;
		//NOTE: the best case is 2 * (object count / leaf object count) -1, but in case we make 
		//bad splits, we approximate upper bound with next power of 2 for the objectCount/leaf object count term
		const size_t worseCaseNodes = 2 * Utils::NextPowerOf2(objectCount / leafObjectCount) - 1;
		m_flatNodes.reserve(worseCaseNodes);
		if (m_flatNodes.size() < worseCaseNodes)
		{
			m_flatNodes.resize(worseCaseNodes);
		}

		size_t nextNodeIndex = 1;
		CreateSubtree<T>(objectPtr, 0, objectCount, leafObjectCount, m_flatNodes[0], nextNodeIndex, nextNodeIndex);
		return m_flatNodes;
	}

	bool Intersects(const WorldPosition3D& rayOrigin, const Vec3& rayDir, T* outHitObject) const
	{
		const Vec3 inverseRayDir = 1 / rayDir;
		if (!m_flatNodes[0].IsIntersectedByRay(rayOrigin, inverseRayDir))
			return false;

		int nodeIndex = -1;
		while (!m_flatNodes[nodeIndex].IsLeaf())
		{
			BVHFlatNode& node = m_flatNodes[nodeIndex];

			if (m_flatNodes[node.m_IndexChild0].IsIntersectedByRay(rayOrigin, inverseRayDir))
				nodeIndex = node.m_IndexChild0;
			else if (m_flatNodes[node.m_IndexChild1].IsIntersectedByRay(rayOrigin, inverseRayDir))
				nodeIndex = node.m_IndexChild1;
			else return false;
		}

		T* object = nullptr;
		for (size_t i = 0; i < m_flatNodes[nodeIndex].m_ObjectCount; i++)
		{
			object = m_objectPtr[m_flatNodes[nodeIndex].m_ObjectStartIndex + i];
			if (object->IsIntersectedByRay(rayOrigin, rayDir))
			{
				outHitObject = object;
				return true;
			}
		}
		return false;
	}

	const std::vector<BVHFlatNode>& GetNodes() const { return m_flatNodes; }
};