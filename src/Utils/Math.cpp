#include "Utils/Math.hpp"
#include <cmath>
#include <sstream>

namespace Utils
{
	float FastIntPow(float base, int exponent)
	{
		if (exponent == 0) return 1.0f;
		
		if (exponent > 0)
		{
			while (exponent != 0)
			{
				base *= base;
				exponent--;
			}
		}
		else
		{
			exponent *= -1;
			while (exponent != 0)
			{
				base /= base;
				exponent--;
			}
		}
		return base;
	}

	std::uint32_t NextPowerOf2(std::uint32_t num)
	{
		if (num == 0) return 1;
		num--;
		num |= num >> 1;
		num |= num >> 2;
		num |= num >> 4;
		num |= num >> 8;
		num |= num >> 16;
		num++;
		return num;

	}

	bool ApproximateEqualsF(float a, float b, const float relEps, const float absEps)
	{
		return std::fabs(a - b) <= std::fmax(relEps * std::fmax(std::fabs(a), std::fabs(b)), absEps);
	}

	float Roundf(const float& decimal, const std::uint8_t& places)
	{
		float factor = FastIntPow(10.0f, places);
		return std::round(decimal * factor) / factor;
	}

	double ToRadians(const double deg)
	{
		return deg * DEG_TO_RAD_CONSTANT;
	}
	double ToDegrees(const double rad)
	{
		return rad * RAD_TO_DEG_CONSTANT;
	}

	bool IsPosInifinity(double value)
	{
		//Note: sign bit returns true if negative value
		return std::isinf(value) && !std::signbit(value);
	}

	bool IsNegInifinity(double value)
	{
		//Note: sign bit returns true if negative value
		return std::isinf(value) && std::signbit(value);
	}

	int GetSign(double num)
	{
		if (ApproximateEqualsF(num, 0)) return 0;
		return num >= 0 ? 1 : -1;
	}
	std::string GetSignSymbol(double num)
	{
		if (ApproximateEqualsF(num, 0)) return "";
		return num >= 0 ? "+" : "-";
	}
}
