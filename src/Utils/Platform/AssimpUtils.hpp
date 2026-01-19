#pragma once
#include "Utils/Math/Matrix.hpp"
#include "assimp/matrix4x4.h"
#include <string>

namespace AssimpUtils
{
	std::string ToString(const aiMatrix4x4 matrix);
}