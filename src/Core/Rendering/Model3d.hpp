#pragma once
#include "Core/Rendering/Vertex.hpp"
#include "Core/Rendering/Material.hpp"
#include "Utils/Data/Matrix.hpp"

namespace Rendering
{
	struct Mesh
	{
		std::vector<Vertex> m_Vertices = {};
		std::vector<IndexType> m_Indices = {};
		Rendering::Material m_Material = {};

		std::string ToString() const;
	};
	struct MeshGroup
	{
		Mat4 m_GlobalTransform = Mat4::GetIdentity();
		/// <summary>
		/// The indices into the model mesh array
		/// </summary>
		std::vector<size_t> m_MeshIndices = {};
	};

	struct Model3d
	{
		std::vector<Mesh> m_Meshes = {};
		std::vector<MeshGroup> m_MeshGroups = {};

		std::string ToString() const;
	};
}