#pragma once
#include <string>
#include <numbers>
#include "Utils/TemplateConcepts.hpp"

namespace Math
{
	inline constexpr float PI = std::numbers::pi;
	inline constexpr float RAD_90 = PI / 2.0f;
	inline constexpr float RAD_180 = PI;
	inline constexpr float RAD_270 = 3.0f * PI / 2.0f;
	inline constexpr float RAD_360 = 2.0f * PI;

	inline constexpr float RAD_30 = PI / 6.0f;
	inline constexpr float RAD_150 = 5.0f * PI / 6.0f;
	inline constexpr float RAD_210 = 7.0f * PI / 6.0f;
	inline constexpr float RAD_330 = 11.0f * PI / 6.0f;

	inline constexpr float RAD_60 = PI / 3.0f;
	inline constexpr float RAD_120 = 2.0f * PI / 3.0f;
	inline constexpr float RAD_240 = 4.0f * PI / 3.0f;
	inline constexpr float RAD_300 = 5.0f * PI / 3.0f;

	inline constexpr float RAD_45 = PI / 4.0f;
	inline constexpr float RAD_135 = 3.0f * PI / 4.0f;
	inline constexpr float RAD_225 = 5.0f * PI / 4.0f;
	inline constexpr float RAD_315 = 7.0f * PI / 4.0f;

	inline constexpr double RAD_TO_DEG_CONSTANT = std::numbers::pi / 180.0;
	inline constexpr double DEG_TO_RAD_CONSTANT = 180.0 / std::numbers::pi;
	inline constexpr double EPSILON = 1e-8f;
	inline constexpr float EPSILON_F = 1e-8f;

	template<typename T>
	T Max(T first) { return first; }

	template<typename T, typename... OtherT>
	requires ::Utils::AllSameType<T, OtherT...>
	T Max(T first, OtherT... next)
	{
		return std::max(first, Max(next...));
	}

	template<typename T>
	T Min(T first) { return first; }

	template<typename T, typename... OtherT>
	requires ::Utils::AllSameType<T, OtherT...>
	T Min(T first, OtherT... next)
	{
		return std::min(first, Min(next...));
	}

	/// <summary>
	/// Generates a random number deterministically in range [0,1] using PCG hash 
	/// and a seed value that is mutated to ensure random value every time.
	/// </summary>
	/// <param name="state"></param>
	/// <returns></returns>
	float FastRandom(std::uint32_t& state);

	template<typename T>
	requires (std::is_integral_v<T>)
	bool FastIsOdd(const int num)
	{
		return (num & 1) != 0;
	}
	template<typename T>
	requires (std::is_integral_v<T>)
	bool FastIsEven(const int num)
	{
		return (num & 1) == 0;
	}

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
	/// Calculates next power of 2 of a number by filling all bits lower than higher level bit
	/// which we can then add 1 to get the next base2 value (which essentially gets next power of 2)
	/// </summary>
	/// <param name="num"></param>
	/// <returns></returns>
	std::uint32_t NextPowerOf2(std::uint32_t num);

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
	bool ApproximateEqualsF(float f1, float f2, const float relEps = 1e-5f, const float absEps = EPSILON);

	/// <summary>
	/// Will round a float to contain the set amount of places after DECIMAL
	/// </summary>
	/// <param name="decimal"></param>
	/// <param name="places"></param>
	/// <returns></returns>
	float Roundf(const float& decimal, const std::uint8_t& places);

	double ToRadians(const double);
	double ToDegrees(const double);

	/// <summary>
	/// Returns the sign of the number except for 0.
	/// ex. -5 -> -1, 0 -> 0, 5 -> +1
	/// </summary>
	/// <param name=""></param>
	/// <returns></returns>
	int GetSign(int);
	/// <summary>
	/// Returns the sign of the number except for 0.
	/// ex. -5.5 -> -1, 0.2 -> +1, 0 -> 0, 5.5 -> +1
	/// </summary>
	/// <param name=""></param>
	/// <returns></returns>
	int GetSign(float);
	/// <summary>
	/// Same as GetSignf() except 0 -> +1
	/// Most of the time this version is only used if guaranteed 0 does NOT occur
	/// </summary>
	/// <param name=""></param>
	/// <returns></returns>
	int GetSignNonZero(int);
	std::string GetSignSymbol(double);
	float MinAbs(const float num1, const float num2);


	bool IsPosInifinity(double);
	bool IsNegInifinity(double);

	float Float16BytesToFloat32(const std::uint16_t);
	std::uint16_t Float32ToFloat16Bytes(const float);
}
