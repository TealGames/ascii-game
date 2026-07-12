#pragma once
#include "Core/ID.hpp"
#include "Core/Rendering/Vertex.hpp"
#include "Core/Asset/MaterialAsset.hpp"
#include "Core/Primitives/Matrix.hpp"
#include "Core/Collision/BVH.hpp"

namespace Engine::Rendering
{
	constexpr size_t BLAS_TREE_LEAF_COUNT = 4;

	class ModelMesh
	{
	private:
	public:
		std::vector<Vertex> m_Vertices = {};
		std::vector<IndexType> m_Indices = {};
		StaticBVHTree<IndexTriangle> m_BLASTree = {};

	private:
	public:
		const std::vector<BVHNode>& ConstructBLASTree(const size_t leafCount);
		AABB3D CalculateTightBounds() const;

		std::string ToString() const;
	};

	class ModelObject
	{
	private:
	public:
		ModelMesh m_Mesh = {};
		MaterialAsset* m_MaterialAsset = nullptr;

	private:
	public:
		std::string ToString() const;
	};

	const std::vector<BVHNode>& ConstructBVHFromTriangles(StaticBVHTree<IndexTriangle>& tree,
		IndexTriangle* triangleArray, const size_t triangleSize, const Vertex* vertexArray);
	const std::vector<BVHNode>& ConstructBVHFromIndices(StaticBVHTree<IndexTriangle>& tree,
		IndexType* indices, const size_t indexSize, const Vertex* vertexArray);

	class ModelObjectGroup
	{
	private:
	public:
		Mat4 m_GlobalTransform = Mat4::GetIdentity();
		/// <summary>
		/// The indices into the model object array
		/// </summary>
		std::vector<std::uint32_t> m_ObjectIndices = {};
	};

	class Model3d
	{
	private:
	public:
		/// <summary>
		/// A mesh is any type of separate geometric data that has its own vertex and material data
		/// </summary>
		std::vector<ModelObject> m_Objects = {};
		/// <summary>
		/// A mesh group is a singular full object within a model
		/// </summary>
		std::vector<ModelObjectGroup> m_ObjectGroups = {};

	private:
	public:
		Model3d() = default;
		Model3d(const Model3d&) = delete;
		Model3d& operator=(const Model3d&) = delete;

		std::string ToString() const;
	};
}