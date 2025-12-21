#include "Utils/Platform/AssimpUtils.hpp"

namespace AssimpUtils
{
	std::string ToString(const aiMatrix4x4 matrix)
	{
		return "SHIT";
		/*
		return std::format("[{},{},{},{}]\n[{},{},{},{}]\n[{},{},{},{}]\n[{},{},{},{}]", 
			matrix.a1, matrix.a2, matrix.a3, matrix.a4,
			matrix.b1, matrix.b2, matrix.b3, matrix.b4,
			matrix.c1, matrix.c2, matrix.c3, matrix.c4,
			matrix.d1, matrix.d2, matrix.d3, matrix.d4);
		*/
	}
}