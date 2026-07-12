#pragma once
#include "Core/Primitives/Matrix.hpp"
#include "Core/Primitives/Color.hpp"
#include "assimp/matrix4x4.h"
#include "assimp/types.h"
#include <string>

namespace AssimpUtils
{
	std::string ToString(const aiMatrix4x4& matrix);
	std::string ToString(const aiString& string);
	std::string_view ToStringView(const aiString& string);

	const Engine::Mat4* ToMatrix(const aiMatrix4x4& matrix);
	Engine::ColHDR3 ToHDRColor3(const aiColor3D& col);
}