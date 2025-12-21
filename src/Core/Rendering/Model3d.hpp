#pragma once
#include "Core/Rendering/Vertex.hpp"
#include "Core/Rendering/Material.hpp"
#include "Utils/Data/Matrix.hpp"
#include "Utils/DataStructure/BVH.hpp"

namespace Rendering
{
	constexpr size_t BLAS_TREE_LEAF_COUNT = 4;

	struct ModelMesh
	{
		std::vector<Vertex> m_Vertices = {};
		std::vector<IndexType> m_Indices = {};
		Rendering::Material m_Material = {};
		StaticBVHTree<Triangle> m_BLASTree;

		const std::vector<BVHFlatNode>& ConstructBLASTree(const size_t leafCount);

		std::string ToString() const;
	};

	const std::vector<BVHFlatNode>& ConstructBVHFromTriangles(StaticBVHTree<Triangle>& tree,
		Triangle* triangleArray, const size_t triangleSize, const Vertex* vertexArray);
	const std::vector<BVHFlatNode>& ConstructBVHFromIndices(StaticBVHTree<Triangle>& tree,
		IndexType* indices, const size_t indexSize, const Vertex* vertexArray);

	struct ModelMeshGroup
	{
		Mat4 m_GlobalTransform = Mat4::GetIdentity();
		/// <summary>
		/// The indices into the model mesh array
		/// </summary>
		std::vector<size_t> m_MeshIndices = {};
	};

	struct Model3d
	{
		/// <summary>
		/// A mesh is any type of separate geometric data that has its own vertex and material data
		/// </summary>
		std::vector<ModelMesh> m_Meshes = {};
		/// <summary>
		/// A mesh group is a singular full object within a model
		/// </summary>
		std::vector<ModelMeshGroup> m_MeshGroups = {};

		std::string ToString() const;
	};
}