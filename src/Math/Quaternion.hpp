#pragma once
#include <glm/gtc/quaternion.hpp>

namespace Math
{ 
	using Quat = glm::quat;
	namespace Consts
	{
		/// <summary>
		/// This represents no rotation
		/// </summary>
		inline constexpr Quat IDENTITY = { 0, 0, 0, 1 };
	}
}
