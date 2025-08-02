#include "Math/Vec3.hpp"
#ifdef GLM
#include "Utils/Math.hpp"
#include "Utils/HelperFunctions.hpp"

namespace Math
{
	bool Vec3Equal(const glm::vec3& v1, const glm::vec3& v2)
	{
		return Utils::ApproximateEqualsF(v1.x, v2.x) && Utils::ApproximateEqualsF(v1.y, v2.y)
			&& Utils::ApproximateEqualsF(v1.z, v2.z);
	}
}
#endif
