#include "Core/Rendering/Model3d.hpp"
#include "Utils/ToStringFunctions.hpp"

namespace Rendering 
{
	std::string ModelMesh::ToString() const
	{
		return std::format("[MESH Vertices:{} Indices:{}]", 
			Utils::ToStringIterable<std::vector<Vertex>, Vertex>(m_Vertices),
			Utils::ToStringIterable<std::vector<IndexType>, IndexType>(m_Indices));
	}

	std::string Model3d::ToString() const
	{
		std::string result = "[MODEL3D ";
		for (const auto& group : m_MeshGroups)
		{
			result += "MeshGroup\nTransform:" + group.m_GlobalTransform.ToString() + "\nMeshes: ";
			for (const auto& meshIndex : group.m_MeshIndices)
			{
				result += m_Meshes[meshIndex].ToString() + "\n";
			}
		}
		result += "]";
		return result;
	}
}