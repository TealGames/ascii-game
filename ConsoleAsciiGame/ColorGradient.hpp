#pragma once
#include <cstdint>
#include <vector>
#include <limits>
#include <string>
#include "raylib.h"

struct ColorGradientKeyFrame
{
	Color m_Color;
	float m_Location;

	bool operator<(const ColorGradientKeyFrame& other) const;
	bool operator>(const ColorGradientKeyFrame& other) const;

	std::string ToString() const;
};


class ColorGradient
{
private:
	std::vector<ColorGradientKeyFrame> m_colorFrames;

public:
	static constexpr float MIN_LOCATION = 0;
	static constexpr float MAX_LOCATION = 1;

private:
public:
	ColorGradient();
	explicit ColorGradient(const Color& singleColor);
	ColorGradient(const Color& leftColor, const Color& rightColor);
	ColorGradient(const std::vector<ColorGradientKeyFrame>& frames);

	Color GetColorAt(float location, const bool& includeAlpha) const;

	std::string ToString() const;
};

