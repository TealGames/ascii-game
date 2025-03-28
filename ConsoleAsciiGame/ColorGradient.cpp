#include <limits>

#include "pch.hpp"
#include "ColorGradient.hpp"
#include "raylib.h"
#include "HelperFunctions.hpp"
#include "RaylibUtils.hpp"
#include "Debug.hpp"

ColorGradientKeyFrame::ColorGradientKeyFrame() : ColorGradientKeyFrame({}, 0) {}
ColorGradientKeyFrame::ColorGradientKeyFrame(const Color& color, const float& location) 
	: m_Color(color), m_Location(location) {}

bool ColorGradientKeyFrame::operator<(const ColorGradientKeyFrame& other) const
{
	return m_Location < other.m_Location;
}

bool ColorGradientKeyFrame::operator>(const ColorGradientKeyFrame& other) const
{
	return m_Location > other.m_Location;
}

std::string ColorGradientKeyFrame::ToString() const
{
	return std::format("[Color: {} @{}]", 
		RaylibUtils::ToString(m_Color), std::to_string(m_Location));
}

ColorGradient::ColorGradient() : ColorGradient({}, {}) {}

ColorGradient::ColorGradient(const Color& singleColor) : 
	ColorGradient(singleColor, singleColor) {}

ColorGradient::ColorGradient(const Color& leftColor, const Color& rightColor) :
	m_colorFrames{ ColorGradientKeyFrame{leftColor, MIN_LOCATION}, 
	ColorGradientKeyFrame{rightColor, MAX_LOCATION} } {}

ColorGradient::ColorGradient(const std::vector<ColorGradientKeyFrame>& frames) 
	: m_colorFrames(frames)
{
	if (!Assert(!frames.empty(),
		std::format("Tried creating a gradient with no frames!")))
		return;

	if (!Assert(frames.size()!=1,
		std::format("Tried creating a gradient with only 1 frame!")))
		return;

	ColorGradientKeyFrame& firstFrame = m_colorFrames.front();
	if (firstFrame.m_Location != MIN_LOCATION)
		firstFrame.m_Location = MIN_LOCATION;

	ColorGradientKeyFrame& lastFrame = m_colorFrames.back();
	if (lastFrame.m_Location != MAX_LOCATION)
		lastFrame.m_Location = MAX_LOCATION;

	std::sort(m_colorFrames.begin(), m_colorFrames.end());
}

Color ColorGradient::GetColorAt(float location, const bool& includeAlpha) const
{
	location = std::clamp(location, MIN_LOCATION, MAX_LOCATION);
	if (m_colorFrames.size() == 1) return m_colorFrames.front().m_Color;

	if (location == MIN_LOCATION) return m_colorFrames.front().m_Color;
	if (location == MAX_LOCATION) return m_colorFrames.back().m_Color;

	int left = -1;
	int right = -1;
	if (m_colorFrames.size() == 2)
	{
		left = 0;
		right = 1;
	}
	else
	{
		for (int i = 1; i <= m_colorFrames.size() - 1; i++)
		{
			if (Utils::ApproximateEqualsF(m_colorFrames[i].m_Location, location))
				return m_colorFrames[i].m_Color;

			/*Log(std::format("Searching gradient loc: {} with {} - > {}",
				std::to_string(location), m_colorFrames[i - 1].ToString(), m_colorFrames[i].ToString()));*/
			if (m_colorFrames[i - 1].m_Location < location &&
				location < m_colorFrames[i].m_Location)
			{
				left = i - 1;
				right = i;
				break;
			}
		}
	}
	
	/*if (!Assert(left != -1 && right != -1,
		std::format("Tried searching for a gradient at location: {} but it could not be found. Full Gradient:{}", 
			std::to_string(location), ToString())))
		return {};*/
	if (left == -1 || right == -1) return {};

	const ColorGradientKeyFrame leftKey = m_colorFrames[left];
	const ColorGradientKeyFrame rightKey = m_colorFrames[right];
	const Color& leftColor = leftKey.m_Color;
	const Color& rightColor = rightKey.m_Color;

	float keysNormalizedVal = (location - leftKey.m_Location) / (rightKey.m_Location - leftKey.m_Location);
	unsigned char newR = std::lerp(leftColor.r, rightColor.r, keysNormalizedVal);
	unsigned char newG = std::lerp(leftColor.g, rightColor.g, keysNormalizedVal);
	unsigned char newB = std::lerp(leftColor.b, rightColor.b, keysNormalizedVal);

	unsigned char newA = std::numeric_limits<unsigned char>::max();
	if (includeAlpha) newA = std::lerp(leftColor.a, rightColor.a, keysNormalizedVal);

	/*Log(std::format("Color at {} is: {} FULL:{}", std::to_string(location), 
		RaylibUtils::ToString(Color{ newR, newG, newB, newA }), ToString()));*/
	return {newR, newG, newB, newA};
}

Color ColorGradient::GetFirstColor(const bool& includeAlpha) const
{
	return GetColorAt(MIN_LOCATION, includeAlpha);
}
Color ColorGradient::GetLastColor(const bool& includeAlpha) const
{
	return GetColorAt(MAX_LOCATION, includeAlpha);
}

const std::vector<ColorGradientKeyFrame>& ColorGradient::GetKeyframes() const
{
	return m_colorFrames;
}

std::string ColorGradient::ToString() const
{
	return Utils::ToStringIterable<std::vector<ColorGradientKeyFrame>, 
		ColorGradientKeyFrame>(m_colorFrames);
}