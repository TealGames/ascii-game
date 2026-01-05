#include "Core/Rendering/Model3d.hpp"
#include "Utils/ToStringFunctions.hpp"

namespace Rendering 
{
	const std::vector<BVHFlatNode>& ConstructBVHFromTriangles(StaticBVHTree<Triangle>& tree,
		Triangle* triangleArray, const size_t triangleSize, const Vertex* vertexArray)
	{
		const std::vector<BVHFlatNode>& nodes = tree.Construct(triangleArray, triangleSize, true, 
			Rendering::BLAS_TREE_LEAF_COUNT, BVHSplitAlgorithm::Midpoint,
			[vertexArray](const Triangle& triangle) -> AABB3D
			{
				return CalculateTriangleAABB(triangle, vertexArray);
			},
			[vertexArray](const Triangle& triangle) -> WorldPosition3D
			{
				return CalculateTriangleCenter(triangle, vertexArray);
			}, nullptr); 
		ENGINE_ASSERT(tree.IsValid(nullptr, true), "After constructing BLAS BVH from triangles tree was invalid");
		return nodes;
	}

	const std::vector<BVHFlatNode>& ConstructBVHFromIndices(StaticBVHTree<Triangle>& tree,
		IndexType* indices, const size_t indexSize, const Vertex* vertexArray)
	{
		ENGINE_ASSERT(indexSize % 3 == 0, "Attempted to construct BVH from indices by "
				"reinterpreting as triangles but size is not divisible by 3: {}", indexSize);
		//NOTE: we CAN re-order the indices since they are just the redferences for the vertex buffer
		//therefore it does not matter as long as we move all 3 indices
		Triangle* triangles = reinterpret_cast<Triangle*>(&indices[0]);
		return ConstructBVHFromTriangles(tree, triangles, indexSize / 3, vertexArray);
	}
	
	const std::vector<BVHFlatNode>& ModelMesh::ConstructBLASTree(const size_t leafCount)
	{
		return ConstructBVHFromIndices(m_BLASTree, &m_Indices[0], m_Indices.size(), &m_Vertices[0]);
	}
	AABB3D ModelMesh::CalculateTightBounds() const
	{
		WorldPosition3D minVertex = Vec3::Max();
		WorldPosition3D maxVertex = Vec3::Min();
		for (const auto& vertex : m_Vertices)
		{
			minVertex = Min(minVertex, vertex.m_LocalPos);
			maxVertex = Max(maxVertex, vertex.m_LocalPos);
		}
		return AABB3D(maxVertex - minVertex);
	}

	std::string ModelMesh::ToString() const
	{
		return std::format("[MESH Vertices:{} Indices:{}]",
			Utils::ToStringIterable(m_Vertices),
			Utils::ToStringIterable(m_Indices));
	}

	std::string ModelObject::ToString() const
	{
		return std::format("[MOdelObject Mesh:{} Material:{}]", m_Mesh.ToString(), m_Material.ToString());
	}

	std::string Model3d::ToString() const
	{
		std::string result = "[MODEL3D ";
		for (const auto& group : m_ObjectGroups)
		{
			result += "MeshGroup\nTransform:" + group.m_GlobalTransform.ToString() + "\nMeshes: ";
			for (const auto& meshIndex : group.m_ObjectIndices)
			{
				result += m_Objects[meshIndex].ToString() + "\n";
			}
		}
		result += "]";
		return result;
	}
}