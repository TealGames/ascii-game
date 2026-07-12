#include "Utils/Platform/AssimpUtils.hpp"

namespace AssimpUtils
{
	std::string ToString(const aiMatrix4x4& matrix)
	{
		return std::format("[{},{},{},{}]\n[{},{},{},{}]\n[{},{},{},{}]\n[{},{},{},{}]", 
			matrix.a1, matrix.a2, matrix.a3, matrix.a4,
			matrix.b1, matrix.b2, matrix.b3, matrix.b4,
			matrix.c1, matrix.c2, matrix.c3, matrix.c4,
			matrix.d1, matrix.d2, matrix.d3, matrix.d4);
	}
	std::string ToString(const aiString& string)
	{
		return std::string(string.C_Str(), string.length);
	}
	std::string_view ToStringView(const aiString& string)
	{
		return std::string_view(string.C_Str(), string.length);
	}

	const Engine::Mat4* ToMatrix(const aiMatrix4x4& matrix)
	{
		return reinterpret_cast<const Engine::Mat4*>(&matrix);
	}
	Engine::ColHDR3 ToHDRColor3(const aiColor3D& col)
	{
		return Engine::ColHDR3(col.r, col.g, col.b);
	}
}