#pragma once
#include <string>

namespace Utils
{
	/// <summary>
	/// Computes a power as just repeated multiplication
	/// Faster than std::pow because std::pow must account for complex
	/// edge cases so it uses a process that uses logs and exponents (which are 
	/// typically approximated with series expansion = SLOW) and this does not 
	/// need to account for those edge cases so it can do it fast
	/// </summary>
	/// <param name="base"></param>
	/// <param name="exponent"></param>
	/// <returns></returns>
	float FastIntPow(float base, int exponent);

	/// <summary>
	/// Handles approximate values for floats for values close to 0 (by checking absolute diff < abs epsilon)
	/// and also for large positive and small negatives aka big deltas/big abs values 
	/// (by checking scaled relative epsilon based on max abs value < diff)
	/// </summary>
	/// <param name="f1"></param>
	/// <param name="f2"></param>
	/// <param name="relEps"></param>
	/// <param name="absEps"></param>
	/// <returns></returns>
	bool ApproximateEqualsF(float f1, float f2, const float relEps = 1e-5f, const float absEps = 1e-8f);

	/// <summary>
	/// Will round a float to contain the set amount of places after DECIMAL
	/// </summary>
	/// <param name="decimal"></param>
	/// <param name="places"></param>
	/// <returns></returns>
	float Roundf(const float& decimal, const std::uint8_t& places);

	/// <summary>
	/// Will get the number of places the signficiant digits of a decimal occupies
	/// 1.2 -> 2
	/// 0.3456 -> 5
	/// </summary>
	size_t GetDigitPlaces(const double& decimal);
	/// <summary>
	/// Will get ONLY the deciaml places for significant digits of decimal
	/// 1.2 -> 1
	/// 0.345 -> 3
	/// 68 -> 0
	/// </summary>
	/// <param name="decimal"></param>
	/// <returns></returns>
	size_t GetDecimalPlaces(const double& decimal);

	double ToRadians(const double);
	double ToDegrees(const double);

	//Returns the sign of the number, except for 0
	//example: 5 -> 1, -5 -> -1, 0 -> 0
	int GetSign(double);
	std::string GetSignSymbol(double);

	bool IsPosInifinity(double);
	bool IsNegInifinity(double);
}
