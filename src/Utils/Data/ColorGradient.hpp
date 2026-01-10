#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include "Utils/Data/Color.hpp"
//#include "raylib.h"

struct ColorGradientKeyFrame
{
	HDRColor m_Color;
	float m_Location;

	ColorGradientKeyFrame();
	ColorGradientKeyFrame(const HDRColor& color, const float& location);

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
	explicit ColorGradient(const HDRColor& singleColor);
	ColorGradient(const HDRColor& leftColor, const HDRColor& rightColor);
	ColorGradient(const std::vector<ColorGradientKeyFrame>& frames);

	HDRColor GetColorAt(float location, const bool& includeAlpha) const;
	HDRColor GetFirstColor(const bool& includeAlpha) const;
	HDRColor GetLastColor(const bool& includeAlpha) const;

	const std::vector<ColorGradientKeyFrame>& GetKeyframes() const;

	std::string ToString() const;
};

