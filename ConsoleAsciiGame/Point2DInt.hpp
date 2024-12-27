#pragma once
#include <string>

namespace Utils
{
	struct Point2DInt
	{
		int m_X;
		int m_Y;

		constexpr Point2DInt();
		constexpr Point2DInt(const int& x, const int& y);
		Point2DInt(const Point2DInt&) = default;

		std::string ToString() const;

		Utils::Point2DInt GetFlipped() const;

		//TODO: add all other operators for rvalues as well
		Point2DInt operator+(const Point2DInt&) const;
		Point2DInt operator-(const Point2DInt&) const;
		Point2DInt operator*(const Point2DInt&) const;
		Point2DInt operator*(const int) const;
		Point2DInt operator/(const Point2DInt&) const;
		Point2DInt operator/(const int) const;

		bool operator==(const Point2DInt&) const;
		bool operator!=(const Point2DInt&) const;
		bool operator<(const Point2DInt&) const;
		bool operator<=(const Point2DInt&) const;
		bool operator>(const Point2DInt&) const;
		bool operator>=(const Point2DInt&) const;

		//This is needed because const vars delete = operator definition
		Point2DInt& operator=(const Point2DInt&);
		Point2DInt& operator=(Point2DInt&& other) noexcept;
	};

	int GetDistance(const Utils::Point2DInt& p1, const Utils::Point2DInt& p2);
}

namespace std
{
	template<> struct hash<Utils::Point2DInt>
	{
		std::size_t operator()(const Utils::Point2DInt& pos) const noexcept
		{
			std::hash<int> hashDouble;
			std::size_t xHash = hashDouble(pos.m_X);
			std::size_t yHash = hashDouble(pos.m_Y);
			return xHash ^ (yHash << 1);
		}
	};
}




