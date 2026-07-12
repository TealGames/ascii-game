#include "Core/Serialization/Model3DFileFormat.hpp"
#include "Core/Serialization/StringSerializers.hpp"

namespace Engine::VTXConverter
{
	static constexpr char TUPLE_SEPARATOR_CHAR = ',';
	static constexpr char TUPLE_START_CHAR = '[';
	static constexpr char TUPLE_END_CHAR = ']';
	static constexpr VTXFormatType FORMAT_TYPE = VTXFormatType::Binary;

	std::string VTXHeader::ToString() const
	{
		return std::format("[VTXHeader Version:{} Stride:{} VertexCount:{} IndexCount:{} MaxPos:{} MinPos:{}]", 
			m_Version, m_VertexStride, m_VertexCount, m_IndexCount, std::format("({},{},{})", m_PosMax[0], m_PosMax[1], m_PosMax[2]), 
			std::format("({},{},{})", m_PosMin[0], m_PosMin[1], m_PosMin[2]));
	}

	std::string VTXPackedVertex::ToString() const
	{
		return std::format("[VTXPackedVertex Pos:{} UV:{} Normal:{}]", 
			std::format("[{},{},{}]", m_LocalPos[0], m_LocalPos[1], m_LocalPos[2]),
			std::format("[{},{}]", m_UV[0], m_UV[1]),
			std::format("({},{})", m_Normal[0], m_Normal[1]));
	}

	static std::string SerializeVec2(const Vec2& v)
	{
		return TUPLE_START_CHAR + StringSerializers::Serialize<float, 2>(&v[0], TUPLE_SEPARATOR_CHAR) + TUPLE_END_CHAR;
	}
	static Vec2 DeserializeVec2(const std::string_view view)
	{
		return StringSerializers::Deserialize<float, 2>(view, TUPLE_SEPARATOR_CHAR);
	}
	static std::string SerializeVec3(const Vec3& v)
	{
		return TUPLE_START_CHAR + StringSerializers::Serialize<float, 3>(&v[0], TUPLE_SEPARATOR_CHAR) + TUPLE_END_CHAR;
	}
	static Vec3 DeserializeVec3(const std::string_view view)
	{
		return StringSerializers::Deserialize<float, 3>(view, TUPLE_SEPARATOR_CHAR);
	}

	static bool TryWriteAsString(const Rendering::Model3d& model, const std::filesystem::path& path)
	{
		const Rendering::ModelObject& obj = model.m_Objects[0];
		std::string fileContents = "";
		for (const auto& vertex : obj.m_Mesh.m_Vertices)
		{
			fileContents += SerializeVec3(vertex.m_LocalPos) + SerializeVec2(vertex.m_UVPos) + SerializeVec3(vertex.m_Normal);
		}
		std::string indicesStr = StringSerializers::Serialize<Rendering::IndexType>(obj.m_Mesh.m_Indices, TUPLE_SEPARATOR_CHAR);
		fileContents += indicesStr;

		return ::Utils::IO::TryWriteFile(path, fileContents);
	}
	static bool TryWriteAsBinary(const Rendering::Model3d& model, const std::filesystem::path& path)
	{
		const Rendering::ModelObject& obj = model.m_Objects[0];
		VTXHeader header = { CURRENT_VERSION, sizeof(VTXPackedVertex), obj.m_Mesh.m_Vertices.size(), obj.m_Mesh.m_Indices.size() };
		const AABB3D& bounds = obj.m_Mesh.CalculateTightBounds();
		const Vec3 boundSize = bounds.GetSize();
		header.m_PosMin[0] = bounds.m_MinPos[0];
		header.m_PosMin[1] = bounds.m_MinPos[1];
		header.m_PosMin[2] = bounds.m_MinPos[2];

		header.m_PosMax[0] = bounds.m_MaxPos[0];
		header.m_PosMax[1] = bounds.m_MaxPos[1];
		header.m_PosMax[2] = bounds.m_MaxPos[2];

		std::ofstream writeStream;
		if (!::Utils::IO::TryCreateWriteFileBinaryStream(path, writeStream))
			return false;

		writeStream.write(reinterpret_cast<char*>(&header), sizeof(VTXHeader));

		VTXPackedVertex packedVertex = {};
		Vec2 octahedralEncodedNormal = {};
		constexpr std::uint16_t maxUint16 = std::numeric_limits<std::uint16_t>::max();
		for (const Rendering::Vertex& vertex : obj.m_Mesh.m_Vertices)
		{
			packedVertex.m_LocalPos[0] = (vertex.m_LocalPos.m_X - bounds.m_MinPos.m_X) / boundSize.m_X * maxUint16;
			packedVertex.m_LocalPos[1] = (vertex.m_LocalPos.m_Y - bounds.m_MinPos.m_Y) / boundSize.m_Y * maxUint16;
			packedVertex.m_LocalPos[2] = (vertex.m_LocalPos.m_Z - bounds.m_MinPos.m_Z) / boundSize.m_Z * maxUint16;

			//Since UV is in range [0, 1] we can quantize it to its corresponding value in uint16 to get 4 bytes 
			//for the UV pos rather than 8 bytes with 2 float32
			packedVertex.m_UV[0] = vertex.m_UVPos.m_X * maxUint16;
			packedVertex.m_UV[1] = vertex.m_UVPos.m_Y * maxUint16;

			octahedralEncodedNormal = Math::OctahedralEncodeNormal(vertex.m_Normal);
			packedVertex.m_Normal[0] = octahedralEncodedNormal.m_X * maxUint16;
			packedVertex.m_Normal[1] = octahedralEncodedNormal.m_Y * maxUint16;
			writeStream.write(reinterpret_cast<char*>(&packedVertex), sizeof(VTXPackedVertex));
		}

		writeStream.write(reinterpret_cast<const char*>(&obj.m_Mesh.m_Indices[0]),
			obj.m_Mesh.m_Indices.size() * sizeof(Rendering::IndexType));

		return true;
	}
	bool TryWriteModelToPath(const Rendering::Model3d& model, const std::filesystem::path& path)
	{
		if constexpr (FORMAT_TYPE == VTXFormatType::String)
			return TryWriteAsString(model, path);
		else return TryWriteAsBinary(model, path);
	}

	static bool TryReadAsString(Rendering::Model3d& model, const std::filesystem::path& path)
	{
		std::string fileContents = ::Utils::IO::TryReadFileFull(path);
		if (fileContents.empty())
		{
			return false;
		}

		model.m_ObjectGroups.clear();
		model.m_Objects.clear();

		Rendering::ModelObjectGroup& currGroup = model.m_ObjectGroups.emplace_back();
		Rendering::ModelObject& currObj = model.m_Objects.emplace_back();
		currGroup.m_ObjectIndices.push_back(0);

		using IntegralVertexDataType = std::uint8_t;
		enum class VertexDataType : IntegralVertexDataType
		{
			Position = 0,
			UV = 1,
			Normal = 2
		};
		VertexDataType readingVertexData = VertexDataType::Position;
		WorldPosition3D currPosition;
		UV currUV;
		WorldPosition3D currNormal;

		const char* currCharPtr = &fileContents[0];
		size_t currSize = 0;
		size_t i = 1;
		for (; i < fileContents.size(); i++)
		{
			const char c = fileContents[i];
			if (c == TUPLE_START_CHAR)
			{
				if (i - 1 < fileContents.size()) currCharPtr = &fileContents[i + 1];
				currSize = 0;
			}
			else if (c == TUPLE_END_CHAR)
			{
				if (readingVertexData == VertexDataType::Position)
				{
					currPosition = DeserializeVec3(std::string_view(currCharPtr, currSize));
				}
				else if (readingVertexData == VertexDataType::UV)
				{
					currUV = DeserializeVec2(std::string_view(currCharPtr, currSize));
				}
				else if (readingVertexData == VertexDataType::Normal)
				{
					currNormal = DeserializeVec3(std::string_view(currCharPtr, currSize));
					currObj.m_Mesh.m_Vertices.emplace_back(currPosition, currUV, currNormal);
					if (i < fileContents.size() - 1 && fileContents[i + 1] != TUPLE_START_CHAR)
						break;
				}

				readingVertexData = static_cast<VertexDataType>((static_cast<IntegralVertexDataType>(readingVertexData) + 1) % 3);
			}
			else
			{
				currSize++;
			}
		}

		if (i + 2 >= fileContents.size())
		{
			return false;
		}

		std::string_view indexView = std::string_view(&fileContents[i + 1], fileContents.size() - (i + 1));
		currObj.m_Mesh.m_Indices = std::move(StringSerializers::Deserialize<Rendering::IndexType>(indexView, TUPLE_SEPARATOR_CHAR));

		currObj.m_Mesh.ConstructBLASTree(Rendering::BLAS_TREE_LEAF_COUNT);
	}

	static bool TryReadAsBinary(Rendering::Model3d& model, const std::filesystem::path& path)
	{
		model.m_ObjectGroups.clear();
		model.m_Objects.clear();
		Rendering::ModelObjectGroup& currGroup = model.m_ObjectGroups.emplace_back();
		Rendering::ModelObject& currObj = model.m_Objects.emplace_back();
		currGroup.m_ObjectIndices.push_back(0);

		std::ifstream readStream;
		if (!::Utils::IO::CreateReadFileBinaryStream(path, readStream))
			return false;

		VTXHeader header;
		readStream.read(reinterpret_cast<char*>(&header), sizeof(VTXHeader));
		if (!readStream)
		{
			LogError(std::format("Attempted to read model3d VTX format at path:{}"
				"but reached end of file with more data left to read.Header:{}", path.string(), header.ToString()));
			return false;
		}

		const float boundSizeX = header.m_PosMax[0] - header.m_PosMin[0];
		const float boundSizeY = header.m_PosMax[1] - header.m_PosMin[1];
		const float boundSizeZ = header.m_PosMax[2] - header.m_PosMin[2];

		VTXPackedVertex packedVertex;
		WorldPosition3D localPos; 
		Vec3 normal;
		Vec2 octahedralNormal;
		UV uv;

		constexpr std::uint16_t maxUint16 = std::numeric_limits<std::uint16_t>::max();
		for (int i = 0; i < header.m_VertexCount; i++)
		{
			readStream.read(reinterpret_cast<char*>(&packedVertex), sizeof(VTXPackedVertex));
			if (!readStream)
			{
				LogError(std::format("Attempted to read model3d VTX format at path:{}"
					"but reached end of file with more data left to read.Header:{}", path.string(), header.ToString()));
				return false;
			}

			localPos =
			{
				packedVertex.m_LocalPos[0] / float(maxUint16) * boundSizeX + header.m_PosMin[0],
				packedVertex.m_LocalPos[1] / float(maxUint16) * boundSizeY + header.m_PosMin[1],
				packedVertex.m_LocalPos[2] / float(maxUint16) * boundSizeZ + header.m_PosMin[2],
			};

			uv = { packedVertex.m_UV[0] / float(maxUint16), packedVertex.m_UV[1] / float(maxUint16) };

			octahedralNormal = {packedVertex.m_Normal[0] / float(maxUint16), packedVertex.m_Normal[1] / float(maxUint16) };
			normal = Math::OctahedralDecodeNormal(octahedralNormal);

			currObj.m_Mesh.m_Vertices.emplace_back(localPos, uv, normal);
		}

		currObj.m_Mesh.m_Indices.resize(header.m_IndexCount);
		readStream.read(reinterpret_cast<char*>(&currObj.m_Mesh.m_Indices[0]), currObj.m_Mesh.m_Indices.size() * sizeof(Rendering::IndexType));
		currObj.m_Mesh.ConstructBLASTree(Rendering::BLAS_TREE_LEAF_COUNT);

		return true;
	}
	bool TryReadModelFromPath(Rendering::Model3d& model, const std::filesystem::path& path)
	{
		if constexpr (FORMAT_TYPE == VTXFormatType::String)
			return TryReadAsString(model, path);
		else return TryReadAsBinary(model, path);
	}
}