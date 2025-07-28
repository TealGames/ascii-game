#include "Utils/Math.hpp"
#include <cmath>
#include <sstream>
#include <numbers>

namespace Utils
{
	bool ApproximateEqualsF(float a, float b, const float relEps, const float absEps)
	{
		return std::fabs(a - b) <= std::fmax(relEps * std::fmax(std::fabs(a), std::fabs(b)), absEps);
	}

	float Roundf(const float& decimal, const std::uint8_t& places)
	{
		float factor = std::pow(10.0f, places);
		return std::round(decimal * factor) / factor;
	}

	size_t GetDigitPlaces(const double& decimal)
	{
		std::string sigFigStr = std::to_string(decimal);
		if (sigFigStr.find(".") != std::string::npos) return sigFigStr.size() - 1;
		return sigFigStr.size();
	}

	size_t GetDecimalPlaces(const double& decimal)
	{
		std::string sigFigStr = std::to_string(decimal);
		size_t decimalPos = sigFigStr.find(".");

		if (decimalPos == std::string::npos) return 0;
		return sigFigStr.size() - decimalPos - 1;
	}

	double ToRadians(const double deg)
	{
		return deg * (std::numbers::pi / 180.0);
	}

	double ToDegrees(const double rad)
	{
		return rad * (180.0 / std::numbers::pi);
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
