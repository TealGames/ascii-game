#include "Math/Vec3.hpp"
#include "Utils/Math.hpp"
#include "Utils/HelperFunctions.hpp"

namespace Math
{
	bool Vec3Equal(const Vec3& v1, const Vec3& v2)
	{
		return Utils::ApproximateEqualsF(v1.x, v2.x) && Utils::ApproximateEqualsF(v1.y, v2.y)
			&& Utils::ApproximateEqualsF(v1.z, v2.z);
	}
	std::string ToString(const Vec3& vec, const std::uint8_t decimalPlaces)
	{
		return std::format("({},{},{})",
			Utils::ToStringDouble(Utils::Roundf(vec.x, decimalPlaces), decimalPlaces),
			Utils::ToStringDouble(Utils::Roundf(vec.y, decimalPlaces), decimalPlaces),
			Utils::ToStringDouble(Utils::Roundf(vec.z, decimalPlaces), decimalPlaces));
	}
}
