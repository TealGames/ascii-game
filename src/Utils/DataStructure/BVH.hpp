#pragma once
#include <cstdint>
#include <algorithm>
#include <concepts>
#include <functional>
#include <stack>
#include "Utils/AnsiCodes.hpp"
#include "Utils/Math/WorldPosition.hpp"
#include "Utils/HelperMacros.hpp"
#include "Utils/Math/MathAdvanced.hpp"
#include "Utils/Math/AABB.hpp"
#include "Utils/ToStringFunctions.hpp"

//TODO: implement a linear bvh with morton codes which 
//is better for gpu for having faster reload/reconstruction times when movmeent 
template<size_t ALIGN>
struct alignas(ALIGN) BVHNodeBase
{
	//AABB3D_Align16 m_Bounds = {};
	AABB3DBase<ALIGN> m_Bounds = {};

	int m_ObjectStartIndex = -1;
	std::uint32_t m_ObjectCount = 0;

	int m_IndexChild0 = -1;
	int m_IndexChild1 = -1;

	static constexpr size_t GetAlignment() { return ALIGN; }

	bool IsLeaf() const { return m_ObjectCount > 0 && m_ObjectStartIndex >= 0 && m_IndexChild0 == -1 && m_IndexChild1 == -1; }
	bool IsInvalid() const { return m_ObjectCount == 0 && m_ObjectStartIndex == -1 && m_IndexChild0 == -1 && m_IndexChild1 == -1; }
	bool IsIntersectedByRay(const Ray3D& ray, float* outMargin) const
	{
		return Utils::RayIntersectsBounds<ALIGN>(m_Bounds, ray, outMargin);
	}
	bool IsIntersectedByRay(const Ray3D& ray, float* outTEnter, float* outTExit) const
	{
		return Utils::RayIntersectsBounds<ALIGN>(m_Bounds, ray, outTEnter, outTEnter);
	}

	AABB3D GetAABB() const
	{
		return ConvertAlignment<ALIGN, AABB3D::GetAlignment()>(m_Bounds);
	}
	WorldPosition3D GetCenter() const
	{
		return m_Bounds.GetCenter();
	}

	std::string ToString(const bool fullData) const
	{
		return std::format("[Bounds:{}{}]", m_Bounds.ToString(),
			(fullData) ? std::format("C0:{} C1:{} L:{}{}", m_IndexChild0, m_IndexChild1, IsLeaf(),
				(IsLeaf() ? std::format("ObjStart:{} ObjCount:{}", m_ObjectStartIndex, m_ObjectCount) : "")) : "");
	}
	std::string ToString() const { return ToString(true); }
};
template<size_t START_ALIGN, size_t NEW_ALIGN>
BVHNodeBase<NEW_ALIGN> ConvertAlignment(const BVHNodeBase<START_ALIGN>& node)
{
	return BVHNodeBase<NEW_ALIGN>(::ConvertAlignment<START_ALIGN, NEW_ALIGN>(node.m_Bounds), 
		node.m_ObjectStartIndex, node.m_ObjectCount, node.m_IndexChild0, node.m_IndexChild1);
}

using BVHNode = BVHNodeBase<0>;

DEFINE_TEMPLATE_HAS_FUNCTION(GetObjectStartIndex, std::uint32_t);
DEFINE_TEMPLATE_HAS_NAMED_FUNCTION(IsIntersectedByRay);

enum class BVHToStringType : std::uint8_t
{
	/// <summary>
	/// Will convert object data stored in a node to string
	/// </summary>
	Object = 0,
	/// <summary>
	/// Will convert the bounds represented by the node to string
	/// </summary>
	NodeBounds = 1,
	/// <summary>
	/// Will convert all data of a node to string
	/// </summary>
	NodeFull = 2
};

template<typename T, size_t NODE_ALIGN = 0>
requires (Utils::HasFunctionToString<T>)
std::string ToStringBVHNodes(const BVHNodeBase<NODE_ALIGN>* nodeArray, const size_t arrayNodeOffset, const size_t nodeCount,
	const T* objectArray, const std::uint32_t* objectIndicesArray, BVHToStringType toStringType,
	std::function<std::string(const BVHNodeBase<NODE_ALIGN>& node, const BVHNodeBase<NODE_ALIGN>* parentNode)> overrideNodeToStringFunc = nullptr,
	const std::function<std::string(const BVHNodeBase<NODE_ALIGN>&)>& leafSuccessorToStringFunc = nullptr,
	const bool markInvalidBounds = false)
{
	const BVHNodeBase<NODE_ALIGN>* rootNode = &nodeArray[arrayNodeOffset];
	if (overrideNodeToStringFunc == nullptr)
	{
		overrideNodeToStringFunc = 
			[&leafSuccessorToStringFunc, toStringType, objectArray, objectIndicesArray, markInvalidBounds]
			(const BVHNodeBase<NODE_ALIGN>& node, const BVHNodeBase<NODE_ALIGN>* parentNode) -> std::string
			{
				std::string stringPrefix = "";
				//If the parent node's size or any of the parent's aabb min and/or max components
				//are contained within this bound, it means the parent does not fully encompass the child
				//and therefore we mark is red
				if (markInvalidBounds && parentNode != nullptr)
				{
					const AABB3D parentBounds = parentNode->GetAABB();
					const AABB3D thisBounds = node.GetAABB();
					if (parentBounds.GetSize().AnyAxisLessThan(thisBounds.GetSize()) || parentBounds.m_MinPos.AnyAxisGreaterThan(thisBounds.m_MinPos) ||
						parentBounds.m_MaxPos.AnyAxisLessThan(thisBounds.m_MaxPos))
					{
						stringPrefix = ANSI_COLOR_RED;
					}
				}

				if (toStringType == BVHToStringType::NodeBounds)
					return stringPrefix + node.ToString(false);
				else if (toStringType == BVHToStringType::NodeFull)
					return stringPrefix + node.ToString(true);
				else if (toStringType == BVHToStringType::Object)
				{
					std::string result = stringPrefix;
					for (size_t i = 0; i < node.m_ObjectCount; i++)
					{
						if (objectIndicesArray == nullptr)
							result += objectArray[node.m_ObjectStartIndex + i].ToString();
						else
							result += objectArray[objectIndicesArray[node.m_ObjectStartIndex + i]].ToString();
					}
					return result;
				}
				return "[STRINGIFY FAILED]";
			};
	}

	const auto oldOverrideToStringFunc = overrideNodeToStringFunc;
	overrideNodeToStringFunc =
		[&leafSuccessorToStringFunc, &oldOverrideToStringFunc, rootNode]
		(const BVHNodeBase<NODE_ALIGN>& node, const BVHNodeBase<NODE_ALIGN>* parentNode)->std::string
		{
			std::string leafStringSuffix = "";
			if (node.IsLeaf() && leafSuccessorToStringFunc != nullptr) 
				leafStringSuffix = "\n" + leafSuccessorToStringFunc(node);

			//NOTE: since we want indices printed with each node, we can use a clever trick since 
			//all nodes should be contiguous to just subtract the current addr from root address
			return std::format("({})", size_t(&node - rootNode)) + 
				oldOverrideToStringFunc(node, parentNode) + leafStringSuffix;
		};

	return Utils::ToStringTree<BVHNodeBase<NODE_ALIGN>>(*rootNode,
		[nodeArray, arrayNodeOffset, nodeCount](const BVHNodeBase<NODE_ALIGN>& node, const size_t childIndex) -> const BVHNodeBase<NODE_ALIGN>*
		{
			if (childIndex >= 2 || node.IsLeaf())
			{
				return nullptr;
			}
			const int minValidIndex = static_cast<int>(arrayNodeOffset);
			const int maxValidIndex = minValidIndex + nodeCount - 1;
			if (childIndex == 0)
			{
				if (node.m_IndexChild0 < minValidIndex || node.m_IndexChild0 > maxValidIndex)
				{
					LogError(std::format("Attempted to convert BVH to string "
						"but found invalid child index: {} (BVH Tree Valid Indices: [{}, {}])", node.m_IndexChild0, minValidIndex, maxValidIndex));
					return nullptr;
				}
				return &nodeArray[node.m_IndexChild0];
			}

			if (node.m_IndexChild1 < minValidIndex || node.m_IndexChild1 > maxValidIndex)
			{
				LogError(std::format("Attempted to convert BVH to string "
					"but found invalid child index: {} (BVH Tree Valid Indices: [{}, {}])", node.m_IndexChild0, minValidIndex, maxValidIndex));
				return nullptr;
			}
			return &nodeArray[node.m_IndexChild1];
		}, overrideNodeToStringFunc);
}

template<typename T, size_t NODE_ALIGN = 0>
bool IntersectsBVH(const Ray3D& ray, const BVHNodeBase<NODE_ALIGN>* rootNode, const size_t nodeCount,
	const T* objectArray, const std::uint32_t* objectIndicesArray, const T** outHitObject, float* outMinHitDistance,
	const std::function<bool(const BVHNodeBase<NODE_ALIGN>& node, const T& obj,
		const Ray3D& ray, float* outHitDistance)>& objectIntersectedByRayFunc)
{
	ENGINE_ASSERT(ray.m_Dir.IsUnitVector(), "Attempted to invoke IntersectsBVH with non-unit vector dir: {}", ray.m_Dir.ToString());

	//LogWarning(std::format("INVERSE RAY DIR for {} is {}", rayDir.ToString(), inverseRayDir.ToString()));
	std::stack<int> stack = {};
	stack.push(0);

	const BVHNodeBase<NODE_ALIGN>* node = nullptr;
	const T* object = nullptr;
	float tEnter = 0;
	float tExit = 0;
	float minHitDistance = std::numeric_limits<float>::max();
	bool foundIntersection = false;

	bool hitLeaf = false;
	while (!stack.empty())
	{
		node = &rootNode[stack.top()];
		stack.pop();

		if (!node->IsIntersectedByRay(ray, &tEnter, &tExit))
			continue;

		//NOTE: as long as raydir is normalized, we can use tEnter as displacement
		if (tEnter > minHitDistance)
			continue;

		if (node->IsLeaf())
		{
			if (!hitLeaf)
			{
				//LogWarning("HIT LEAF");
				hitLeaf = true;
			}

			for (size_t i = 0; i < node->m_ObjectCount; i++)
			{
				if (objectIndicesArray == nullptr) object = &objectArray[node->m_ObjectStartIndex + i];
				else object = &objectArray[objectIndicesArray[node->m_ObjectStartIndex + i]];

				float hitDistance = 0;
				if (objectIntersectedByRayFunc(*node, *object, ray, &hitDistance))
				{
					foundIntersection = true;
					if (hitDistance < minHitDistance)
					{
						if (outHitObject != nullptr) *outHitObject = object;
						minHitDistance = hitDistance;
					}
				}
			}
		}
		else
		{
			float tEnterChild0 = 0;
			float tEnterChild1 = 0;

			//NOTE: as long as raydir is normalized, we can use tEnter as displacement
			bool minHitChild0 = rootNode[node->m_IndexChild0].IsIntersectedByRay(ray, &tEnterChild0) && tEnterChild0 <= minHitDistance;
			bool minHitChild1 = rootNode[node->m_IndexChild1].IsIntersectedByRay(ray, &tEnterChild1) && tEnterChild1 <= minHitDistance;

			if (minHitChild0 && minHitChild1)
			{
				if (tEnterChild0 < tEnterChild1)
				{
					//LogWarning("ADDED SHIT");
					stack.push(node->m_IndexChild1);
					stack.push(node->m_IndexChild0);
				}
				else
				{
					//LogWarning("ADDED SHIT");
					stack.push(node->m_IndexChild0);
					stack.push(node->m_IndexChild1);
				}
			}
			else if (minHitChild0)
			{
				//LogWarning("ADDED SHIT");
				stack.push(node->m_IndexChild0);
			}
			else if (minHitChild1)
			{
				//LogWarning("ADDED SHIT");
				stack.push(node->m_IndexChild1);
			}
		}
	}
	if (outMinHitDistance != nullptr)
		*outMinHitDistance = minHitDistance;
	return foundIntersection;
}

template<typename T, size_t NODE_ALIGN = 0>
bool IsValidBVHHelper(const BVHNodeBase<NODE_ALIGN>* rootNode, const size_t currIndex, const int parentIndex, const size_t nodeCount,
	const T* objectArray, const std::uint32_t* objectIndicesArray, const std::function<AABB3D(const T&)>& getBoundsFunc, 
	const std::function<bool(const BVHNodeBase<NODE_ALIGN>&)>& leafSuccessorIsValidFunc, bool outputMessages)
{
	const BVHNodeBase<NODE_ALIGN>& currentNode = rootNode[currIndex];
	const auto& thisBounds = currentNode.GetAABB();
	const BVHNodeBase<NODE_ALIGN>* parentNode = parentIndex >= 0 ? &rootNode[parentIndex] : nullptr;

	if (parentNode != nullptr)
	{
		const auto& parentBounds = parentNode->GetAABB();
		if (!Utils::IsWithinBounds(parentBounds, thisBounds.m_MaxPos) ||
			!Utils::IsWithinBounds(parentBounds, thisBounds.m_MinPos))
		{
			if (outputMessages)
			{
				LogWarning(std::format("[ValidateBVH]: node index:{} has bounds:{} outside parent bounds:{}", 
					currIndex, thisBounds.ToString(), parentBounds.ToString()));
			}
			return false;
		}
	}
	if (currentNode.IsLeaf())
	{
		for (size_t i = 0; i < currentNode.m_ObjectCount; i++)
		{
			const size_t objectIndex = currentNode.m_ObjectStartIndex + i;
			const auto& objectBounds = getBoundsFunc(objectIndicesArray!= nullptr? 
				objectArray[objectIndicesArray[objectIndex]] : objectArray[objectIndex]);
			if (!Utils::IsWithinBounds(thisBounds, objectBounds.m_MaxPos) ||
				!Utils::IsWithinBounds(thisBounds, objectBounds.m_MinPos))
			{
				if (outputMessages)
				{
					LogWarning(std::format("[ValidateBVH]: object {} bound at node index:{} has bounds:{} outside leaf node bounds:{}",
						i, currIndex, objectBounds.ToString(), thisBounds.ToString()));
				}
				return false;
			}
		}
		if (leafSuccessorIsValidFunc != nullptr && !leafSuccessorIsValidFunc(currentNode))
			return false;
	}
	else
	{
		if (!IsValidBVHHelper(rootNode, currentNode.m_IndexChild0, currIndex, nodeCount,
			objectArray, objectIndicesArray, getBoundsFunc, leafSuccessorIsValidFunc, outputMessages))
			return false;

		if (!IsValidBVHHelper(rootNode, currentNode.m_IndexChild1, currIndex, nodeCount,
			objectArray, objectIndicesArray, getBoundsFunc, leafSuccessorIsValidFunc, outputMessages))
			return false;
	}

	return true;
}

template<typename T, size_t NODE_ALIGN = 0>
bool IsValidBVH(const BVHNodeBase<NODE_ALIGN>* rootNode, const size_t nodeCount,
	const T* objectArray, const std::uint32_t* objectIndicesArray, const std::function<AABB3D(const T&)>& getBoundsFunc,
	const std::function<bool(const BVHNodeBase<NODE_ALIGN>&)>& leafSuccessorIsValidFunc, bool outputMessages)
{
	return IsValidBVHHelper(rootNode, 0, -1, nodeCount, objectArray, objectIndicesArray, 
		getBoundsFunc, leafSuccessorIsValidFunc, outputMessages);
}

enum class BVHSplitAlgorithm : std::uint8_t
{
	/// <summary>
	/// Splits triangles based on the midpoint of the largest extent of the bounds for a given
	/// interval of vertices. 
	/// PROS: very fast to compute -> quick to create trees
	/// CONS: can result in highly unbalanced trees, 
	/// many nodes with less than optimal object count in leaves
	/// </summary>
	Midpoint			= 0,
	/// <summary>
	/// Splits triangles in half after sorting in the largest extent of the bounds for a given 
	/// interval of vertices using the primitive center position
	/// PROS: faster than Surface Area, guaranteed to have TARGET_LEAF_COUNT or TARGET_LEAF_COUNT -1 leaves
	/// due to splitting in half every time (NOTE: odd amounts may result in the TARGET_LEAF_COUNT-1 leaves)
	/// and less nodes
	/// CONS: requires sorting on every split which may increase performance cost when creating tree
	/// </summary>
	Median				= 1,
	/// <summary>
	/// Splits triangles based the most optimal surface area for given interval of vertices
	/// to create best split possible with even vertices on both sides
	/// PROS: can result in lesser nodes by creating more optimal splits, but not guaranteed balanced
	/// but possibly better traversal when using tree. Tree is more balanced when there is clustering of 
	/// primitives
	/// CONS: takes a lot more time to compute due to requiring sorting every split and may result
	/// in unbalanced/shallower tree in less dense areas
	/// </summary>
	SurfaceArea		= 2
};

/// <summary>
/// A bounding volume hierarchy which constructs tree by splitting an area's dimensions
/// by the longest axis in half and dividing into two partitions which then continue to
/// get split until reaching leaf node count and storing indices of the primitives
/// </summary>
template<typename T, size_t NODE_ALIGN = 0>
requires (std::is_default_constructible_v<T>)
class StaticBVHTree
{
public:
	using NodeType = BVHNodeBase<NODE_ALIGN>;
private:
	T* m_objectArray;
	size_t m_objectCount;
	/// <summary>
	/// The true amount of nodes in the tree. m_flatNodes.size() is NOT reliable
	/// because some split algorithms have approximate size calculated on construction
	/// so it may be greater than true amount.
	/// </summary>
	size_t m_nodeCount;
	std::uint8_t m_targetLeafCount;
	std::vector<NodeType> m_flatNodes;
	std::vector<std::uint32_t> m_objectIndices;

	std::function<AABB3D(const T&)> m_getBoundsFunc;
	std::function<WorldPosition3D(const T&)> m_getCenterFunc;
public:
	

private:
	AABB3D GetObjectBounds(const bool mutateObjectsInPlace, const std::uint32_t index) const
	{
		if (mutateObjectsInPlace)
			return m_getBoundsFunc(m_objectArray[index]);
		return m_getBoundsFunc(m_objectArray[m_objectIndices[index]]);
	}
	WorldPosition3D GetObjectCenter(const bool mutateObjectsInPlace, const std::uint32_t index) const
	{
		if (mutateObjectsInPlace)
			return m_getCenterFunc(m_objectArray[index]);
		return m_getCenterFunc(m_objectArray[m_objectIndices[index]]);
	}

	void SortObjects(const bool mutateObjectsInPlace, const size_t objectStartIndex, const size_t objectCount, 
		std::uint8_t maxAxisIndex)
	{
		if (mutateObjectsInPlace)
		{
			std::sort(m_objectArray + objectStartIndex, m_objectArray + objectStartIndex + objectCount, 
				[this, maxAxisIndex](const T& object1, const T& object2) -> bool
				{
					return m_getCenterFunc(object1)[maxAxisIndex] < m_getCenterFunc(object2)[maxAxisIndex];
				});
		}
		else
		{
			std::sort(m_objectIndices.begin() + objectStartIndex, m_objectIndices.begin() + objectStartIndex + objectCount,
				[this, maxAxisIndex](const std::uint32_t& objectIndex1, const std::uint32_t& objectIndex2) -> bool
				{
					return m_getCenterFunc(m_objectArray[objectIndex1])[maxAxisIndex] <
						m_getCenterFunc(m_objectArray[objectIndex2])[maxAxisIndex];
				});
		}
	}

	void CreateSubtrees(const size_t objectStartIndex, const size_t objectCount, const bool mutateObjectsInPlace,
		const BVHSplitAlgorithm splitAlgorithm, NodeType& node, size_t& nextNodeIndex,
		const std::function<void(const T* objectArr, const std::uint32_t* objectIndicesArr, const size_t objectSize, int intendedStartIndex,
			int& outStartIndex, std::uint32_t& outObjectCount)>& objectIntervalOverrideFunc)
	{
		//NOTE: if mutate objects in place is TRUE, the objectStartIndex and objectCount are indexing into ObjectArr,
		//but if mutate is FALSE then they are indices into the objectIndices
		
		//We know the previous index must have been this node index
		const size_t thisNodeIndex = nextNodeIndex - 1;

		AABB3D nodeTightBounds = GetObjectBounds(mutateObjectsInPlace, objectStartIndex);
		WorldPosition3D centerBoundsMin = GetObjectCenter(mutateObjectsInPlace, objectStartIndex);
		WorldPosition3D centerBoundsMax = centerBoundsMin;

		int i;
		for (i = 1; i < objectCount; i ++)
		{
			//TODO: a potential speedup could be that instead of unifying bounds at every new object, we intead try to get the object min
			//and max and compare that and create the bound at the end
			nodeTightBounds = UnifyBounds(GetObjectBounds(mutateObjectsInPlace, objectStartIndex + i), nodeTightBounds);
			WorldPosition3D triangleCenter = GetObjectCenter(mutateObjectsInPlace, objectStartIndex + i);
			centerBoundsMin = Min(centerBoundsMin, triangleCenter);
			centerBoundsMax = Max(centerBoundsMax, triangleCenter);
		}

		node.m_Bounds = ConvertAlignment<AABB3D::GetAlignment(), NODE_ALIGN>(nodeTightBounds);
		m_nodeCount++;

		node.m_IndexChild0 = -1;
		node.m_IndexChild1 = -1;
		//If we reach the target (or less) object count per leaf we we
		//set the primitive values for this node and exit recursion
		if (objectCount <= m_targetLeafCount)
		{
			if (objectIntervalOverrideFunc != nullptr)
			{
				objectIntervalOverrideFunc(m_objectArray, m_objectIndices.empty()? nullptr : &m_objectIndices[0], 
					objectCount, objectStartIndex, node.m_ObjectStartIndex, node.m_ObjectCount);
			}
			else
			{
				node.m_ObjectStartIndex = objectStartIndex;
				node.m_ObjectCount = objectCount;
			}
			return;
		}

		node.m_ObjectStartIndex = -1;
		node.m_ObjectCount = 0;

		//NOTE: since we use the center for comparing triangles, we must also use the center when calculatin the max component index
		//otherwise there may be some inconsistencies
		const Vec3 centerSpan = centerBoundsMax - centerBoundsMin;
		std::uint8_t maxAxisIndex = centerSpan.GetMaxComponentIndex();
		//If the centers of the objects are relatively coplanar, they may have a size that is close to 0
		//in one or more components, and thus the max axis may be ~0 and we get degenerate splits
		if (centerSpan[maxAxisIndex] < 1e-6f)
		{
			maxAxisIndex = nodeTightBounds.GetSize().GetMaxComponentIndex();
		}

		//We partition the data into smaller and greater partition compared to 
		//the pivot as midpoint (NOTE: this is the same as partition algorithm in quicksort)
		if (splitAlgorithm == BVHSplitAlgorithm::Midpoint)
		{
			float midpoint = (centerBoundsMin[maxAxisIndex] + centerBoundsMax[maxAxisIndex]) * 0.5f;

			i = 0;
			int j = objectCount - 1;
			WorldPosition3D objectCenter = {};
			while (i <= j)
			{
				objectCenter = GetObjectCenter(mutateObjectsInPlace, objectStartIndex + i);
				if (objectCenter[maxAxisIndex] < midpoint)
					i++;
				else
				{
					if (mutateObjectsInPlace) std::swap(m_objectArray[objectStartIndex + i], m_objectArray[objectStartIndex + j]);
					else std::swap(m_objectIndices[objectStartIndex + i], m_objectIndices[objectStartIndex + j]);
					j--;
				}
			}
		}
		else if (splitAlgorithm == BVHSplitAlgorithm::Median)
		{
			SortObjects(mutateObjectsInPlace, objectStartIndex, objectCount, maxAxisIndex);
			i = objectCount / 2;
		}
		else if (splitAlgorithm == BVHSplitAlgorithm::SurfaceArea)
		{
			SortObjects(mutateObjectsInPlace, objectStartIndex, objectCount, maxAxisIndex);

			//TODO: right now this implemenation is expensive because we use a lot of memory and time
			//for group of nodes. We should be caching some prefix/suffix bounds to not need to recalculate
			AABB3D currentAABB = {};
			AABB3D* prefixBounds = (AABB3D*)alloca(sizeof(AABB3D) * (objectCount - 1));
			prefixBounds[0] = GetObjectBounds(mutateObjectsInPlace, objectStartIndex);
			for (i = 1; i < objectCount - 1; i++)
			{
				currentAABB = GetObjectBounds(mutateObjectsInPlace, objectStartIndex + i);
				prefixBounds[i] = UnifyBounds(prefixBounds[i - 1], currentAABB);
			}
			AABB3D* suffixBounds = (AABB3D*)alloca(sizeof(AABB3D) * (objectCount - 1));
			suffixBounds[objectCount - 2] = GetObjectBounds(mutateObjectsInPlace, objectStartIndex + objectCount - 1);
			for (i = objectCount - 3; i >= 0; i--)
			{
				currentAABB = GetObjectBounds(mutateObjectsInPlace, objectStartIndex + i + 1);
				suffixBounds[i] = UnifyBounds(suffixBounds[i + 1], currentAABB);
			}

			float surfaceAreaMinCost = std::numeric_limits<float>::max();
			size_t minSurfaceAreaIndex = 1;
			for (i = 1; i < objectCount - 1; i++)
			{
				const float currentCost = Utils::CalculateSurfaceArea(prefixBounds[i - 1]) * i +
					Utils::CalculateSurfaceArea(suffixBounds[i]) * (objectCount - i);
				if (currentCost < surfaceAreaMinCost)
				{
					minSurfaceAreaIndex = i;
					surfaceAreaMinCost = currentCost;
				}
			}
			i = minSurfaceAreaIndex;
		}

		/*LogWarning(std::format("Made split: [{},{}] [{},{}] from [{}, {}]", objectStartIndex, objectStartIndex + i,
			objectStartIndex + i, objectStartIndex + i + objectCount - i, objectStartIndex, objectStartIndex+objectCount)); */

		//If we end up with a degenerate split no matter the algorithm to prevent
		//infinite recursion, we force split in half
		if (i == 0 || i == objectCount)
		{
			i = objectCount / 2;
		}

		//We then setup the left and right children nodes and invoke next subtree
		//with the left and right partioned segments
		node.m_IndexChild0 = nextNodeIndex;
		nextNodeIndex++;
		CreateSubtrees(objectStartIndex, i, mutateObjectsInPlace, splitAlgorithm, m_flatNodes[node.m_IndexChild0],
			nextNodeIndex, objectIntervalOverrideFunc);

		node.m_IndexChild1 = nextNodeIndex;
		nextNodeIndex++;
		CreateSubtrees(objectStartIndex + i, objectCount - i, mutateObjectsInPlace, splitAlgorithm, m_flatNodes[node.m_IndexChild1], 
			nextNodeIndex, objectIntervalOverrideFunc);
	}
private:
	void SetupConstruction(T* objectPtr, const size_t objectCount, const bool mutateObjectsInPlace, 
		const size_t leafObjectCount, const BVHSplitAlgorithm splitAlgorithm)
	{
		m_objectArray = objectPtr;
		m_objectCount = objectCount;
		m_targetLeafCount = leafObjectCount;
		m_nodeCount = 0;
		
		size_t worseCaseNodes = 0;
		//NOTE: the amount for midpoint is a worse case scenario where tree is essentially linked list
		if (splitAlgorithm == BVHSplitAlgorithm::Midpoint) worseCaseNodes = 2 * objectCount;
		//Median achieves best case scenario every time due to splitting in half
		else if (splitAlgorithm == BVHSplitAlgorithm::Median) worseCaseNodes = 2 * Utils::NextPowerOf2(objectCount / leafObjectCount) - 1;
		//SAH has same worse case but practically never achieves this amount and usually approches median count
		else if (splitAlgorithm == BVHSplitAlgorithm::SurfaceArea) worseCaseNodes = 2 * objectCount;

		m_flatNodes.reserve(worseCaseNodes);
		if (m_flatNodes.size() < worseCaseNodes)
		{
			m_flatNodes.resize(worseCaseNodes);
		}

		//If we do not mutate objects in place, it means we need to setup indices
		//into the object array to then move the indices rather than the objects in place
		if (!mutateObjectsInPlace)
		{
			m_objectIndices.reserve(objectCount);
			for (std::uint32_t i = 0; i < objectCount; i++)
				m_objectIndices.emplace_back(i);
		}
	}
public:
	StaticBVHTree() : m_objectArray(nullptr), m_objectCount(0), m_nodeCount(0), m_flatNodes(), m_objectIndices(), m_targetLeafCount(0) {}
	static constexpr size_t GetNodeByteSize() { return sizeof(NodeType); }

	const std::vector<NodeType>& Construct(T* objectPtr, const size_t objectCount, const bool mutateObjectsInPlace,
		const size_t leafObjectCount, const BVHSplitAlgorithm splitAlgorithm,
		const std::function<AABB3D(const T&)>& getBoundsFunc,
		const std::function<WorldPosition3D(const T&)>& getCenterFunc,
		const std::function<void(const T* objectArr, const std::uint32_t* objectIndicesArr, const size_t objectSize, int intendedStartIndex, 
			int& outStartIndex, std::uint32_t& outObjectCount)>& objectIntervalOverrideFunc = nullptr, const bool checkIsValid = false)
	{
		m_getBoundsFunc = getBoundsFunc;
		m_getCenterFunc = getCenterFunc;
		SetupConstruction(objectPtr, objectCount, mutateObjectsInPlace, leafObjectCount, splitAlgorithm);
		size_t nextNodeIndex = 1;
		CreateSubtrees(0, objectCount, mutateObjectsInPlace, splitAlgorithm, m_flatNodes[0],
			nextNodeIndex, objectIntervalOverrideFunc);

		if (checkIsValid)
		{
			if (IsValid(nullptr, true)) Log(std::format("Constructed Valid BVH Tree"));
			else LogError(std::format("Attempted to construct BVH tree:{} but was invalid. Look above for specific messages", 
				ToString(BVHToStringType::NodeBounds)));
		}
		return m_flatNodes;
	}

	template<typename OutT>
	bool Intersects(const Ray3D& ray, OutT* objectArray, const OutT** outHitObject, float* outMinHitDistance,
		const std::function<bool(const NodeType& node, const OutT& obj,
			const Ray3D& ray, float* outHitDistance)>& objectIntersectedByRayFunc) const 
	{
		return ::IntersectsBVH<OutT>(ray, &m_flatNodes[0], m_flatNodes.size(), 
			objectArray, m_objectIndices.empty()? nullptr : &m_objectIndices[0], outHitObject, outMinHitDistance, objectIntersectedByRayFunc);
	}

	template<typename OutT>
	requires HasNamedFunctionIsIntersectedByRay<OutT, bool, Ray3D, float*>
	bool Intersects(const Ray3D& ray, OutT* objectArray, const OutT** outHitObject) const
	{
		return Intersects<OutT>(ray, objectArray, outHitObject,
			[](const OutT& obj, const Ray3D& ray, float* outHitDistance) -> bool
			{
				return obj.IsIntersectedByRay(ray, outHitDistance);
			});
	}

	bool Intersects(const Ray3D& ray, const T** outHitObject, float* outMinHitDistance) const
		requires HasNamedFunctionIsIntersectedByRay<T, bool, Ray3D, float*>
	{
		return Intersects<T>(ray, m_objectArray, outHitObject, outMinHitDistance);
	}

	template<typename OverrideT>
	bool IsValid(const OverrideT* overrideObjectArr, const std::function<AABB3D(const OverrideT&)>& overrideGetBoundsFunc,
		const std::function<bool(const NodeType&)>& leafSuccessorIsValidFunc, bool outputMessages) const
	{
		return IsValidBVH<OverrideT, NODE_ALIGN>(&m_flatNodes[0], m_flatNodes.size(), overrideObjectArr,
			m_objectIndices.empty()? nullptr : &m_objectIndices[0], 
			overrideGetBoundsFunc, leafSuccessorIsValidFunc, outputMessages);
	}

	bool IsValid(const std::function<bool(const NodeType&)>& leafSuccessorIsValidFunc, bool outputMessages) const
	{
		return IsValid<T>(&m_objectArray[0], m_getBoundsFunc, leafSuccessorIsValidFunc, outputMessages);
	}

	const std::vector<NodeType>& GetNodes() const { return m_flatNodes; }
	const std::vector<std::uint32_t>& GetObjectIndices() const { return m_objectIndices; }
	const NodeType& GetRoot() const { return m_flatNodes[0]; }
	size_t Size() const { return m_nodeCount; }

	
	std::string ToString(BVHToStringType toStringType, 
		const std::function<std::string(const NodeType& node, const NodeType* parentNode)>& overrideNodeToStringFunc = nullptr,
		const std::function<std::string(const NodeType&)>& leafSuccessorToStringFunc = nullptr,
		const bool markInvalidBounds = false) const
	{
		return ToStringBVHNodes<T>(&m_flatNodes[0], 0, m_flatNodes.size(),
			m_objectArray, m_objectIndices.empty()? nullptr : &m_objectIndices[0], 
			toStringType, overrideNodeToStringFunc, leafSuccessorToStringFunc, markInvalidBounds);
	}

	std::string ToStringRaw() const
	{
		return Utils::ToStringIterable(m_flatNodes, false);
	}

	std::string ToStringStats() const
	{
		size_t* leafCounts = (size_t*)alloca(sizeof(size_t) * (m_targetLeafCount));
		size_t totalLeaves = 0;
		size_t totalNodes = 0;
		for (size_t i = 0; i < m_targetLeafCount; i++)
			leafCounts[i] = 0;

		std::string resultStr = "[BVH Tree Stats \nOther:";
		for (const auto& node : m_flatNodes)
		{
			if (!node.IsInvalid()) totalNodes++;
			if (!node.IsLeaf()) continue;

			totalLeaves++;
			if (node.m_ObjectCount > m_targetLeafCount)
				resultStr += std::format("InvalidLeaf:{}", node.m_ObjectCount);
			leafCounts[node.m_ObjectCount - 1]++;
		}
		resultStr += std::format("\nTotalNodes:{}\nInvokedPrimitives:{}\nTotalLeaves:{}\n", totalNodes, m_objectCount, totalLeaves);
		for (size_t i = 0; i < m_targetLeafCount; i++)
			resultStr += std::format("\nLeafCounts:\n{} Count:{}", i+1, leafCounts[i]);
		return resultStr;
	}
};