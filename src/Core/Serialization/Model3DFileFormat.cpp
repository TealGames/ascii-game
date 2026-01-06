#include "Core/Serialization/Model3DFileFormat.hpp"
#include "Core/Serialization/StringSerializers.hpp"

static constexpr char TUPLE_SEPARATOR_CHAR = ',';
static constexpr char TUPLE_START_CHAR = '[';
static constexpr char TUPLE_END_CHAR = ']';

namespace VTXConverter
{
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

	bool TryWriteModelToPathAsString(const Rendering::Model3d& model, const std::filesystem::path& path)
	{
		const Rendering::ModelObject& obj = model.m_Objects[0];
		std::string fileContents = "";
		for (const auto& vertex : obj.m_Mesh.m_Vertices)
		{
			fileContents += SerializeVec3(vertex.m_LocalPos) + SerializeVec2(vertex.m_UVPos) + SerializeVec3(vertex.m_Normal);
		}
		std::string indicesStr = StringSerializers::Serialize<Rendering::IndexType>(obj.m_Mesh.m_Indices, TUPLE_SEPARATOR_CHAR);
		fileContents += indicesStr;

		return IO::TryWriteFile(path, fileContents);
	}
	bool TryWriteModelToPathAsBytes(const Rendering::Model3d& model, const std::filesystem::path& path)
	{
		const Rendering::ModelObject& obj = model.m_Objects[0];
		VTXHeader header = { CURRENT_VERSION, sizeof(VTXPackedVertex), obj.m_Mesh.m_Vertices.size(), obj.m_Mesh.m_Indices.size() };

		std::string fileContents = "";
		for (const auto& vertex : obj.m_Mesh.m_Vertices)
		{
			fileContents += SerializeVec3(vertex.m_LocalPos) + SerializeVec2(vertex.m_UVPos) + SerializeVec3(vertex.m_Normal);
		}
		fileContents += StringSerializers::Serialize<Rendering::IndexType>(obj.m_Mesh.m_Indices, TUPLE_SEPARATOR_CHAR);

		return IO::TryWriteFile(path, fileContents);
	}

	bool TryReadModelFromPath(Rendering::Model3d& model, const std::filesystem::path& path)
	{
		std::string fileContents = IO::TryReadFileFull(path);
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
		WorldPosition3D currPosition = {};
		UV currUV = {};
		WorldPosition3D currNormal = {};

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

}