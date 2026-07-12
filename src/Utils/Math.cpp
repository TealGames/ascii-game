#include "Utils/Math.hpp"
#include <cmath>
#include <sstream>

namespace Math
{
	float FastRandom(std::uint32_t& input)
	{
		uint32_t state = input * 747796405u + 2891336453u;
		uint32_t word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
		input = (word >> 22u) ^ word;
		return float(state) / std::numeric_limits<std::uint32_t>::max();
	}

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

	int GetSign(int num)
	{
		return (num > 0) - (num < 0);
	}
	int GetSign(float num)
	{
		return (num > 0.0f) - (num < 0.0f);
	}
	int GetSignNonZero(int num)
	{
		//Since >> is implementation defined, 
		//also valid: 1 | -(num < 0);
		return (num >> 31) | 1;
	}
	std::string GetSignSymbol(double num)
	{
		if (ApproximateEqualsF(num, 0)) return "";
		return num >= 0 ? "+" : "-";
	}
	float MinAbs(const float num1, const float num2)
	{
		if (std::abs(num1) < std::abs(num2)) return num1;
		return num2;
	}

	float Float16BytesToFloat32(const std::uint16_t f16Bytes)
	{
		//Float layout: (-1)^SIGN * 1.MANTISSA * 2^(EXPONENT)
		//float16 layout: [1 Bit sign][5 Bit Exponent][10 Bit Mantissa]
		constexpr std::uint16_t MAX_5_BIT = 0b11111;
		constexpr std::uint16_t MAX_8_BIT = 0b11111111;
		constexpr std::uint16_t MAX_10_BIT = 0b1111111111;

		std::uint16_t f16Sign = f16Bytes >> 15;
		std::uint16_t f16Exp = (f16Bytes >> 10) & MAX_5_BIT;
		std::uint16_t f16Man = f16Bytes & MAX_10_BIT;

		std::uint32_t f32 = f16Sign << 31;

		//float32 layout: [1 Bit sign][8 Bit Exponent][23 Bit Mantissa]
		//0 Exponent, 0 mantissa -> 0 so we can just not do anything since it should already be 0 in those spots
		//0 Exponent, non-0 mantissa -> subnormal/denormal (values close to 0)
		if (f16Exp == 0 && f16Man != 0)
		{
			//Subnormal layout: (-1^)^SIGN * 0.MANTISSA * 2^(1-EXPONENT)
			int shift = 0;
			while ((f16Man & 0x400) == 0) 
			{
				f16Man <<= 1;
				++shift;
			}

			f16Man &= MAX_10_BIT;

			// exponent = (-14 - shift)
			f32 |= (127 - 14 - shift) << 23;
			f32 |= uint32_t(f16Man) << 13;
		}
		//If exponent is all 1-> either infinity (0 mantissa) or Nan (non-0 mantissa)
		//and retains all 1 just with more bits for exponent, and same mantissa
		else if (f16Exp == MAX_5_BIT)
		{
			f32 |= (MAX_8_BIT) << 23;
			//Mantissa moves 13 (23 bits - 10 bits) since larger mantissa -> more decimals rightward
			f32 |= std::uint32_t(f16Man) << 13;
		}
		else
		{
			//Exponent is technically a decimal too, so half negative, half positive
			//So we add HALF - 1 (f16: 5 bits -> +(2^5 / 2 - 1)= +15, (f32: 8 bits -> +(2^8 / 2 - 1)= +127
			//so to account for 15 already added to the number, we add 127 -15 = 112
			f32 |= std::uint32_t(f16Exp + 112) << 23;
			f32 |= std::uint32_t(f16Man) << 13;
		}

		float result;
		memcpy(&result, &f32, sizeof(float));
		return result;
	}

	std::uint16_t Float32ToFloat16Bytes(const float f32)
	{
		//TODO: implement
		return 0;
	}
}
