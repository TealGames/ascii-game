#pragma once
#include "Utils/IOHandler.hpp"
#include "Core/Rendering/Model3d.hpp"

namespace VTXConverter
{
	inline constexpr const char* MODEL_3D_FILE_EXTENSION = ".vtx";
	inline constexpr std::uint16_t CURRENT_VERSION = 1;

	struct VTXHeader
	{
		std::uint16_t m_Version;
		std::uint16_t m_VertexStride;
		std::uint32_t m_VertexCount;
		std::uint32_t m_IndexCount;
	};

//NOTE: pack push/pop pushes onto the stack new alignment for the proceeding
//data, which in this case we want 1 byte alignment for the Vertex data to save space
//when writing the vertices to disk.
//NOTE: reading/writing from this 1-aligned struct is dangerous on ARM and older-RISC
//machines because struct does not follow typicaly alignment conventions of max aligned member
//but this does NOT matter as long as we do not read/write members with data types and only work with bytes
#pragma pack(push, 1)
	struct VTXPackedVertex
	{
		std::uint16_t m_LocalPos[3];
		std::uint16_t m_UV[2];
		std::uint16_t m_Normal[2];
	};
#pragma pack(pop)

	bool TryWriteModelToPathAsString(const Rendering::Model3d& model, const std::filesystem::path& path);
	bool TryWriteModelToPathAsBytes(const Rendering::Model3d& model, const std::filesystem::path& path);
	bool TryReadModelFromPath(Rendering::Model3d& model, const std::filesystem::path& path);
}
