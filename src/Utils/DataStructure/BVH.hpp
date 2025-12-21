#pragma once
#include <cstdint>
#include <algorithm>
#include <concepts>
#include <functional>
#include "Utils/Data/WorldPosition.hpp"
#include "Utils/HelperMacros.hpp"
#include "Utils/MathAdvanced.hpp"
#include "Utils/Data/AABB.hpp"
#include "Utils/ToStringFunctions.hpp"

//TODO: implement a linear bvh with morton codes which 
//is better for gpu for having faster reload/reconstruction times when movmeent occurs
struct BVHFlatNode
{
	AABB3D_Align16 m_Bounds = {};

	int m_ObjectStartIndex = -1;
	std::uint32_t m_ObjectCount = 0;

	int m_IndexChild0 = -1;
	int m_IndexChild1 = -1;

	bool IsLeaf() const;
	bool IsInvalid() const;
	bool IsIntersectedByRay(const WorldPosition3D& origin, const WorldPosition3D& inverseDir) const;

	AABB3D GetAABB() const;
	WorldPosition3D GetCenter() const;

	inline std::string ToString() const { return ToString(true); }
	std::string ToString(const bool fullData) const;
};

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

template<typename T>
requires (Utils::HasFunctionToString<T>)
std::string ToStringBVHNodes(const BVHFlatNode* rootNode, const size_t nodeCount, 
	const T* objectArray, const std::uint32_t* objectIndicesArray, BVHToStringType toStringType,
	const std::function<std::string(const BVHFlatNode&)>& leafSuccessorToStringFunc = nullptr, 
	std::function<std::string(const BVHFlatNode&)> overrideNodeToStringFunc = nullptr)
{
	if (overrideNodeToStringFunc == nullptr)
	{
		overrideNodeToStringFunc = 
			[&leafSuccessorToStringFunc, toStringType, objectArray, objectIndicesArray](const BVHFlatNode& node) -> std::string
			{
				std::string leafStringSuffix = "";
				if (node.IsLeaf() && leafSuccessorToStringFunc != nullptr)
					leafStringSuffix = "\n" + leafSuccessorToStringFunc(node);

				if (toStringType == BVHToStringType::NodeBounds)
					return node.ToString(false) + leafStringSuffix;
				else if (toStringType == BVHToStringType::NodeFull)
					return node.ToString(true) + leafStringSuffix;
				else if (toStringType == BVHToStringType::Object)
				{
					std::string result = "";
					for (size_t i = 0; i < node.m_ObjectCount; i++)
					{
						if (objectIndicesArray == nullptr)
							result += objectArray[node.m_ObjectStartIndex + i].ToString();
						else
							result += objectArray[objectIndicesArray[node.m_ObjectStartIndex + i]].ToString();
					}
					result += leafStringSuffix;
					return result;
				}
				return "[STRINGIFY FAILED]";
			};
	}

	return Utils::ToStringTree<BVHFlatNode>(*rootNode,
		[rootNode, nodeCount](const BVHFlatNode& node, const size_t childIndex) -> const BVHFlatNode*
		{
			if (childIndex >= 2 || node.IsLeaf())
			{
				return nullptr;
			}
			if (childIndex == 0)
			{
				if (node.m_IndexChild0 >= (int)nodeCount)
					LogError(std::format("Attempted to convert BVH to string but found invalid child index: {}", node.m_IndexChild0));
				return &rootNode[node.m_IndexChild0];
			}

			if (node.m_IndexChild1 >= (int)nodeCount)
				LogError(std::format("Attempted to convert BVH to string but found invalid child index: {}", node.m_IndexChild1));
			return &rootNode[node.m_IndexChild1];
		}, overrideNodeToStringFunc);
}

template<typename T>
bool IntersectsBVH(const WorldPosition3D& rayOrigin, const Vec3& rayDir, const BVHFlatNode* rootNode, const size_t nodeCount, 
	T* objectArray, const std::uint32_t* objectIndicesArray, T* outHitObject,
	const std::function<bool(const T& obj, const WorldPosition3D& rayOrigin, const Vec3& rayDir)>& objectIntersectedByRayFunc)
{
	const Vec3 inverseRayDir = 1 / rayDir;
	if (!rootNode[0].IsIntersectedByRay(rayOrigin, inverseRayDir))
		return false;

	int nodeIndex = 0;
	while (!rootNode[nodeIndex].IsLeaf())
	{
		const BVHFlatNode& node = rootNode[nodeIndex];

		if (rootNode[node.m_IndexChild0].IsIntersectedByRay(rayOrigin, inverseRayDir))
			nodeIndex = node.m_IndexChild0;
		else if (rootNode[node.m_IndexChild1].IsIntersectedByRay(rayOrigin, inverseRayDir))
			nodeIndex = node.m_IndexChild1;
		else return false;

		if (nodeIndex >= nodeCount)
		{
			LogError(std::format("Attempted to find BVH node intersection but surpassed tree bounds"));
			return false;
		}
	}
	T* object = nullptr;
	for (size_t i = 0; i < rootNode[nodeIndex].m_ObjectCount; i++)
	{
		if (objectIndicesArray == nullptr) object = &objectArray[rootNode[nodeIndex].m_ObjectStartIndex + i];
		else object = &objectArray[objectIndicesArray[rootNode[nodeIndex].m_ObjectStartIndex + i]];

		if (objectIntersectedByRayFunc(*object, rayOrigin, rayDir))
		{
			outHitObject = object;
			return true;
		}
	}
	return false;
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
template<typename T>
requires (std::is_default_constructible_v<T>)
class StaticBVHTree
{
private:
	T* m_objectArray;
	size_t m_objectCount;
	std::uint8_t m_targetLeafCount;
	std::vector<BVHFlatNode> m_flatNodes;
	std::vector<std::uint32_t> m_objectIndices;
public:

private:
	AABB3D GetObjectBounds(const bool mutateObjectsInPlace, const std::uint32_t index, 
		const std::function<AABB3D(const T&)>& getBoundsFunc) const
	{
		if (mutateObjectsInPlace)
			return getBoundsFunc(m_objectArray[index]);
		return getBoundsFunc(m_objectArray[m_objectIndices[index]]);
	}
	WorldPosition3D GetObjectCenter(const bool mutateObjectsInPlace, const std::uint32_t index,
		const std::function<WorldPosition3D(const T&)>& getCenterFunc) const
	{
		if (mutateObjectsInPlace)
			return getCenterFunc(m_objectArray[index]);
		return getCenterFunc(m_objectArray[m_objectIndices[index]]);
	}

	void SortObjects(const bool mutateObjectsInPlace, const size_t objectStartIndex, const size_t objectCount, 
		std::uint8_t maxAxisIndex, const std::function<WorldPosition3D(const T&)>& getCenterFunc)
	{
		if (mutateObjectsInPlace)
		{
			std::sort(m_objectArray + objectStartIndex, m_objectArray + objectStartIndex + objectCount, 
				[this, maxAxisIndex, &getCenterFunc](const T& object1, const T& object2) -> bool
				{
					return getCenterFunc(object1)[maxAxisIndex] < getCenterFunc(object2)[maxAxisIndex];
				});
		}
		else
		{
			std::sort(m_objectIndices.begin() + objectStartIndex, m_objectIndices.begin() + objectStartIndex + objectCount,
				[this, maxAxisIndex, &getCenterFunc](const std::uint32_t& objectIndex1, const std::uint32_t& objectIndex2) -> bool
				{
					return getCenterFunc(m_objectArray[objectIndex1])[maxAxisIndex] < 
						getCenterFunc(m_objectArray[objectIndex2])[maxAxisIndex];
				});
		}
	}

	void CreateSubtrees(const size_t objectStartIndex, const size_t objectCount, const bool mutateObjectsInPlace,
		const BVHSplitAlgorithm splitAlgorithm, BVHFlatNode& node, size_t& nextNodeIndex,
		const std::function<AABB3D(const T&)>& getBoundsFunc,
		const std::function<WorldPosition3D(const T&)>& getCenterFunc,
		const std::function<void(const T* objectArr, const std::uint32_t* objectIndicesArr, const size_t objectSize, int intendedStartIndex,
			int& outStartIndex, std::uint32_t& outObjectCount)>& objectIntervalOverrideFunc)
	{
		//NOTE: if mutate objects in place is TRUE, the objectStartIndex and objectCount are indexing into ObjectArr,
		//but if mutate is FALSE then they are indices into the objectIndices
		
		//We know the previous index must have been this node index
		const size_t thisNodeIndex = nextNodeIndex - 1;

		AABB3D nodeTightBounds = GetObjectBounds(mutateObjectsInPlace, objectStartIndex, getBoundsFunc);
		WorldPosition3D centerBoundsMin = GetObjectCenter(mutateObjectsInPlace, objectStartIndex, getCenterFunc);
		WorldPosition3D centerBoundsMax = centerBoundsMin;

		int i;
		for (i = 1; i < objectCount; i ++)
		{
			nodeTightBounds = UnifyBounds(GetObjectBounds(mutateObjectsInPlace, objectStartIndex + i, getBoundsFunc), nodeTightBounds);
			WorldPosition3D triangleCenter = GetObjectCenter(mutateObjectsInPlace, objectStartIndex + i, getCenterFunc);
			centerBoundsMin = Min(centerBoundsMin, triangleCenter);
			centerBoundsMax = Max(centerBoundsMax, triangleCenter);
		}

		node.m_Bounds = ConvertAlignment<AABB3D::GetAlignment(), AABB3D_Align16::GetAlignment()>(nodeTightBounds);

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
		std::uint8_t maxAxisIndex = (centerBoundsMax - centerBoundsMin).GetMaxComponentIndex();

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
				objectCenter = GetObjectCenter(mutateObjectsInPlace, objectStartIndex + i, getCenterFunc);
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
			SortObjects(mutateObjectsInPlace, objectStartIndex, objectCount, maxAxisIndex, getCenterFunc);
			i = objectCount / 2;
		}
		else if (splitAlgorithm == BVHSplitAlgorithm::SurfaceArea)
		{
			SortObjects(mutateObjectsInPlace, objectStartIndex, objectCount, maxAxisIndex, getCenterFunc);

			AABB3D currentAABB = {};
			AABB3D* prefixBounds = (AABB3D*)alloca(sizeof(AABB3D) * (objectCount - 1));
			prefixBounds[0] = GetObjectBounds(mutateObjectsInPlace, objectStartIndex, getBoundsFunc);
			for (i = 1; i < objectCount - 1; i++)
			{
				currentAABB = GetObjectBounds(mutateObjectsInPlace, objectStartIndex + i, getBoundsFunc);
				prefixBounds[i] = UnifyBounds(prefixBounds[i - 1], currentAABB);
			}
			AABB3D* suffixBounds = (AABB3D*)alloca(sizeof(AABB3D) * (objectCount - 1));
			suffixBounds[objectCount - 2] = GetObjectBounds(mutateObjectsInPlace, objectStartIndex + objectCount - 1, getBoundsFunc);
			for (i = objectCount - 3; i >= 0; i--)
			{
				currentAABB = GetObjectBounds(mutateObjectsInPlace, objectStartIndex + i + 1, getBoundsFunc);
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
		}

		/*LogWarning(std::format("Made split: [{},{}] [{},{}] from [{}, {}]", objectStartIndex, objectStartIndex + i,
			objectStartIndex + i, objectStartIndex + i + objectCount - i, objectStartIndex, objectStartIndex+objectCount)); */

		//We then setup the left and right children nodes and invoke next subtree
		//with the left and right partioned segments
		node.m_IndexChild0 = nextNodeIndex;
		nextNodeIndex++;
		CreateSubtrees(objectStartIndex, i, mutateObjectsInPlace, splitAlgorithm, m_flatNodes[node.m_IndexChild0],
			nextNodeIndex, getBoundsFunc, getCenterFunc, objectIntervalOverrideFunc);

		node.m_IndexChild1 = nextNodeIndex;
		nextNodeIndex++;
		CreateSubtrees(objectStartIndex + i, objectCount - i, mutateObjectsInPlace, splitAlgorithm, m_flatNodes[node.m_IndexChild1], 
			nextNodeIndex, getBoundsFunc, getCenterFunc, objectIntervalOverrideFunc);
	}
private:
	void SetupConstruction(T* objectPtr, const size_t objectCount, const bool mutateObjectsInPlace, 
		const size_t leafObjectCount, const BVHSplitAlgorithm splitAlgorithm)
	{
		m_objectArray = objectPtr;
		m_objectCount = objectCount;
		m_targetLeafCount = leafObjectCount;
		
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
	StaticBVHTree() : m_objectArray(nullptr), m_objectCount(0), m_flatNodes(), m_objectIndices(), m_targetLeafCount(0) {}

	const std::vector<BVHFlatNode>& Construct(T* objectPtr, const size_t objectCount, const bool mutateObjectsInPlace,
		const size_t leafObjectCount, const BVHSplitAlgorithm splitAlgorithm, 
		const std::function<AABB3D(const T&)>& getBoundsFunc,
		const std::function<WorldPosition3D(const T&)>& getCenterFunc,
		const std::function<void(const T* objectArr, const std::uint32_t* objectIndicesArr, const size_t objectSize, int intendedStartIndex, 
			int& outStartIndex, std::uint32_t& outObjectCount)>& objectIntervalOverrideFunc = nullptr)
	{
		SetupConstruction(objectPtr, objectCount, mutateObjectsInPlace, leafObjectCount, splitAlgorithm);
		size_t nextNodeIndex = 1;
		CreateSubtrees(0, objectCount, mutateObjectsInPlace, splitAlgorithm, m_flatNodes[0],
			nextNodeIndex, getBoundsFunc, getCenterFunc, objectIntervalOverrideFunc);
		return m_flatNodes;
	}

	template<typename OutT>
	bool Intersects(const WorldPosition3D& rayOrigin, const Vec3& rayDir, OutT* objectArray, OutT* outHitObject, 
		const std::function<bool(const OutT& obj, const WorldPosition3D& rayOrigin, const Vec3& rayDir)>& objectIntersectedByRayFunc) const 
	{
		return ::IntersectsBVH<OutT>(rayOrigin, rayDir, &m_flatNodes[0], m_flatNodes.size(), 
			objectArray, m_objectIndices.empty()? nullptr : &m_objectIndices[0], outHitObject, objectIntersectedByRayFunc);
	}

	template<typename OutT>
	requires HasNamedFunctionIsIntersectedByRay<OutT, bool, WorldPosition3D, WorldPosition3D>
	bool Intersects(const WorldPosition3D& rayOrigin, const Vec3& rayDir, OutT* objectArray, OutT* outHitObject) const
	{
		return Intersects<OutT>(rayOrigin, rayDir, objectArray, outHitObject,
			[](const OutT& obj, const WorldPosition3D& rayOrigin, const Vec3& rayDir) -> bool
			{
				return obj.IsIntersectedByRay(rayOrigin, rayDir);
			});
	}

	bool Intersects(const WorldPosition3D& rayOrigin, const Vec3& rayDir, T* outHitObject) const
		requires HasNamedFunctionIsIntersectedByRay<T, bool, WorldPosition3D, WorldPosition3D> 
	{
		return Intersects<T>(rayOrigin, rayDir, m_objectArray, outHitObject);
	}

	const std::vector<BVHFlatNode>& GetNodes() const { return m_flatNodes; }
	const std::vector<std::uint32_t>& GetObjectIndices() const { return m_objectIndices; }
	const BVHFlatNode& GetRoot() const { return m_flatNodes[0]; }
	size_t Size() const { return m_flatNodes.size(); }

	
	std::string ToString(BVHToStringType toStringType, 
		const std::function<std::string(const BVHFlatNode&)>& leafSuccessorToStringFunc = nullptr,
		const std::function<std::string(const BVHFlatNode&)>& overrideNodeToStringFunc = nullptr) const
	{
		return ToStringBVHNodes<T>(&m_flatNodes[0], m_flatNodes.size(),
			m_objectArray, m_objectIndices.empty()? nullptr : &m_objectIndices[0], 
			toStringType, leafSuccessorToStringFunc, overrideNodeToStringFunc);
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