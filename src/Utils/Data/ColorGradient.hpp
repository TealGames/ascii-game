#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include "Utils/Data/Color.hpp"
//#include "raylib.h"

struct ColorGradientKeyFrame
{
	Utils::Color m_Color;
	float m_Location;

	ColorGradientKeyFrame();
	ColorGradientKeyFrame(const Utils::Color& color, const float& location);

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
	explicit ColorGradient(const Utils::Color& singleColor);
	ColorGradient(const Utils::Color& leftColor, const Utils::Color& rightColor);
	ColorGradient(const std::vector<ColorGradientKeyFrame>& frames);

	Utils::Color GetColorAt(float location, const bool& includeAlpha) const;
	Utils::Color GetFirstColor(const bool& includeAlpha) const;
	Utils::Color GetLastColor(const bool& includeAlpha) const;

	const std::vector<ColorGradientKeyFrame>& GetKeyframes() const;

	std::string ToString() const;
};

