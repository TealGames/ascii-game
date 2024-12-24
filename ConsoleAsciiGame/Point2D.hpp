#pragma once
#include <string>

namespace Utils
{
	struct Point2D
	{
		float m_X;
		float m_Y;

		Point2D();
		Point2D(float, float);
		Point2D(const Point2D&) = default;

		inline int XAsInt() const;
		inline int YAsInt() const;
		std::string ToString() const;

		Utils::Point2D GetFlipped() const;

		//TODO: add all other operators for rvalues as well
		Point2D operator+(const Point2D&) const;
		Point2D operator-(const Point2D&) const;
		Point2D operator*(const Point2D&) const;
		Point2D operator*(const float) const;
		Point2D operator/(const Point2D&) const;
		Point2D operator/(const float) const;

		bool operator==(const Point2D&) const;

		//This is needed because const vars delete = operator definition
		Point2D& operator=(const Point2D&);
		Point2D& operator=(Point2D&&) noexcept;
	};

	float GetDistance(const Utils::Point2D& p1, const Utils::Point2D& p2);
}

namespace std
{
	template<> struct hash<Utils::Point2D>
	{
		std::size_t operator()(const Utils::Point2D& pos) const noexcept
		{
			std::hash<float> hashFloat;
			std::size_t xHash = hashFloat(pos.m_X);
			std::size_t yHash = hashFloat(pos.m_Y);
			return xHash ^ (yHash << 1);
		}
	};
}

