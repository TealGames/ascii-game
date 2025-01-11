#pragma once
#include <string>

namespace Utils
{
	struct Point3D
	{
		float m_X;
		float m_Y;
		float m_Z;

		Point3D();
		Point3D(const float& x, const float& y, const float& z);
		Point3D(const Point3D&) = default;

		inline int XAsInt() const;
		inline int YAsInt() const;
		inline int ZAsInt() const;
		std::string ToString() const;

		//TODO: add all other operators for rvalues as well
		Point3D operator+(const Point3D&) const;
		Point3D operator-(const Point3D&) const;
		Point3D operator*(const Point3D&) const;
		Point3D operator*(const float) const;
		Point3D operator/(const Point3D&) const;
		Point3D operator/(const float) const;

		bool operator==(const Point3D&) const;

		//This is needed because const vars delete = operator definition
		Point3D& operator=(const Point3D&);
		Point3D& operator=(Point3D&&) noexcept;
	};

	float GetDistance(const Utils::Point3D& p1, const Utils::Point3D& p2);
}

namespace std
{
	template<> struct hash<Utils::Point3D>
	{
		std::size_t operator()(const Utils::Point3D& pos) const noexcept
		{
			std::hash<float> hashFloat;
			std::size_t xHash = hashFloat(pos.m_X);
			std::size_t yHash = hashFloat(pos.m_Y);
			std::size_t zHash = hashFloat(pos.m_Z);
			return xHash ^ (yHash << 1) ^ (zHash << 2);
		}
	};
}

