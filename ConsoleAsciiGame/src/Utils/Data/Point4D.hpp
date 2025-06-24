#pragma once
#include <string>
#include "Point2D.hpp"
#include "Point3D.hpp"

namespace Utils
{
	struct Point4D
	{
		float m_X;
		float m_Y;
		float m_Z;
		float m_W;

		Point4D();
		Point4D(const float& x, const float& y, const float& z, const float& w);

		Point4D(const Point4D&) = default;

		inline int XAsInt() const;
		inline int YAsInt() const;
		inline int ZAsInt() const;
		inline int WAsInt() const;

		std::string ToString() const;


		/// <summary>
		/// Returns just the x and y value to point2d
		/// </summary>
		/// <returns></returns>
		Utils::Point2D ToPoint2D() const;

		/// <summary>
		/// Returns the x, y and z as a point3d
		/// </summary>
		/// <returns></returns>
		Utils::Point3D ToPoint3D() const;

		//TODO: add all other operators for rvalues as well
		Point4D operator+(const Point4D&) const;
		Point4D operator-(const Point4D&) const;
		Point4D operator*(const Point4D&) const;
		Point4D operator*(const float) const;
		Point4D operator/(const Point4D&) const;
		Point4D operator/(const float) const;

		bool operator==(const Point4D&) const;

		//This is needed because const vars delete = operator definition
		Point4D& operator=(const Point4D&);
		Point4D& operator=(Point4D&&) noexcept;
	};
}

namespace std
{
	template<> struct hash<Utils::Point4D>
	{
		std::size_t operator()(const Utils::Point4D& pos) const noexcept
		{
			std::hash<float> hashFloat;
			std::size_t xHash = hashFloat(pos.m_X);
			std::size_t yHash = hashFloat(pos.m_Y);
			std::size_t zHash = hashFloat(pos.m_Z);
			std::size_t wHash = hashFloat(pos.m_W);
			return xHash ^ (yHash << 1) ^ (zHash << 2) ^ (wHash << 3);
		}
	};
}
