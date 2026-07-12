#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include "Core/Primitives/Color.hpp"

namespace Engine
{
	template<typename T, size_t N>
	class ColorGradientKeyFrame
	{
	private:
	public:
		Col<T, N> m_Color;
		float m_Location;

		ColorGradientKeyFrame() : ColorGradientKeyFrame({}, 0) {}
		ColorGradientKeyFrame(const Col<T, N>& color, const float location)
			: m_Color(color), m_Location(location) {}

	private:
	public:
		bool operator<(const ColorGradientKeyFrame& other) const
		{
			return m_Location < other.m_Location;
		}

		bool operator>(const ColorGradientKeyFrame& other) const
		{
			return m_Location > other.m_Location;
		}

		std::string ToString() const
		{
			return std::format("[Color: {} @{}]",
				m_Color.ToString(), std::to_string(m_Location));
		}
	};

	template<typename T, size_t N>
	class ColorGradient
	{
	public:
		static constexpr float MIN_LOCATION = 0;
		static constexpr float MAX_LOCATION = 1;

		using ColorKeyFrame = ColorGradientKeyFrame<T, N>;
		using Color = Col<T, N>;
	private:
		std::vector<ColorKeyFrame> m_colorFrames;
		
	private:
	public:
		ColorGradient() : ColorGradient({}, {}) {}

		explicit ColorGradient(const Color& singleColor) :
			ColorGradient(singleColor, singleColor) {}

		ColorGradient(const Color& leftColor, const Color& rightColor) :
			m_colorFrames{ ColorKeyFrame{leftColor, MIN_LOCATION},
						   ColorKeyFrame{rightColor, MAX_LOCATION} } {}

		ColorGradient(const std::vector<ColorKeyFrame>& frames)
			: m_colorFrames(frames)
		{
			if (!Assert(!frames.empty(),
				"Tried creating a gradient with no frames!"))
				return;

			if (!Assert(frames.size() != 1,
				"Tried creating a gradient with only 1 frame!"))
				return;

			ColorKeyFrame& firstFrame = m_colorFrames.front();
			if (firstFrame.m_Location != MIN_LOCATION)
				firstFrame.m_Location = MIN_LOCATION;

			ColorKeyFrame& lastFrame = m_colorFrames.back();
			if (lastFrame.m_Location != MAX_LOCATION)
				lastFrame.m_Location = MAX_LOCATION;

			std::sort(m_colorFrames.begin(), m_colorFrames.end());
		}

		Color GetColorAt(float location, const bool& includeAlpha) const
		{
			location = std::clamp(location, MIN_LOCATION, MAX_LOCATION);
			if (m_colorFrames.size() == 1) return m_colorFrames.front().m_Color;

			//NOTE: the gradient should be set up to guarantee at least 2 elements one at 0f and one at 1f pos
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
					if (::Math::ApproximateEqualsF(m_colorFrames[i].m_Location, location))
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

			const ColorKeyFrame leftKey = m_colorFrames[left];
			const ColorKeyFrame rightKey = m_colorFrames[right];
			const Color& leftColor = leftKey.m_Color;
			const Color& rightColor = rightKey.m_Color;

			float keysNormalizedVal = (location - leftKey.m_Location) / (rightKey.m_Location - leftKey.m_Location);
			unsigned char newR = std::lerp(leftColor.m_R, rightColor.m_R, keysNormalizedVal);
			unsigned char newG = std::lerp(leftColor.m_G, rightColor.m_G, keysNormalizedVal);
			unsigned char newB = std::lerp(leftColor.m_B, rightColor.m_B, keysNormalizedVal);

			unsigned char newA = std::numeric_limits<unsigned char>::max();
			if (includeAlpha) newA = std::lerp(leftColor.m_A, rightColor.m_A, keysNormalizedVal);

			/*Log(std::format("Color at {} is: {} FULL:{}", std::to_string(location),
				RaylibUtils::ToString(Color{ newR, newG, newB, newA }), ToString()));*/
			return { newR, newG, newB, newA };
		}

		Color GetFirstColor(const bool& includeAlpha) const
		{
			return GetColorAt(MIN_LOCATION, includeAlpha);
		}
		Color GetLastColor(const bool& includeAlpha) const
		{
			return GetColorAt(MAX_LOCATION, includeAlpha);
		}

		const std::vector<ColorKeyFrame>& GetKeyframes() const
		{
			return m_colorFrames;
		}

		std::string ToString() const
		{
			return ::Utils::ToStringIterable(m_colorFrames);
		}
	};

	using ColHDRGradient = ColorGradient<float, 4>;
}


