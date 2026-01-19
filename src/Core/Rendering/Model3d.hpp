#pragma once
#include "Core/Rendering/Vertex.hpp"
#include "Core/Rendering/Material.hpp"
#include "Utils/Math/Matrix.hpp"
#include "Utils/DataStructure/BVH.hpp"

namespace Rendering
{
	constexpr size_t BLAS_TREE_LEAF_COUNT = 4;

	struct ModelMesh
	{
		std::vector<Vertex> m_Vertices = {};
		std::vector<IndexType> m_Indices = {};
		StaticBVHTree<IndexTriangle> m_BLASTree = {};

		const std::vector<BVHNode>& ConstructBLASTree(const size_t leafCount);
		AABB3D CalculateTightBounds() const;

		std::string ToString() const;
	};

	struct ModelObject
	{
		ModelMesh m_Mesh = {};
		Material m_Material = {};

		std::string ToString() const;
	};

	const std::vector<BVHNode>& ConstructBVHFromTriangles(StaticBVHTree<IndexTriangle>& tree,
		IndexTriangle* triangleArray, const size_t triangleSize, const Vertex* vertexArray);
	const std::vector<BVHNode>& ConstructBVHFromIndices(StaticBVHTree<IndexTriangle>& tree,
		IndexType* indices, const size_t indexSize, const Vertex* vertexArray);

	struct ModelObjectGroup
	{
		Mat4 m_GlobalTransform = Mat4::GetIdentity();
		/// <summary>
		/// The indices into the model object array
		/// </summary>
		std::vector<std::uint32_t> m_ObjectIndices = {};
	};

	struct Model3d
	{
		/// <summary>
		/// A mesh is any type of separate geometric data that has its own vertex and material data
		/// </summary>
		std::vector<ModelObject> m_Objects = {};
		/// <summary>
		/// A mesh group is a singular full object within a model
		/// </summary>
		std::vector<ModelObjectGroup> m_ObjectGroups = {};

		std::string ToString() const;
	};
}