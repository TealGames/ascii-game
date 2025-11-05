#pragma once
#include "Core/Rendering/Vertex.hpp"
#include "Core/Rendering/Material.hpp"
#include "Utils/Data/Matrix.hpp"

namespace Rendering
{
	struct ModelMesh
	{
		std::vector<Vertex> m_Vertices = {};
		std::vector<IndexType> m_Indices = {};
		Rendering::Material m_Material = {};

		std::string ToString() const;
	};
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