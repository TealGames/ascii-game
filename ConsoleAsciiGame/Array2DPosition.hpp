#pragma once
#include "Point2DInt.hpp"

constexpr int NULL_INDEX = -1;
class Array2DPosition final
{
private:
	Utils::Point2DInt m_Pos;

private:
public:
	Array2DPosition();
	Array2DPosition(const int& row, const int& col);
	Array2DPosition(const Array2DPosition&) = default;

	const int& GetRow() const;
	const int& GetCol() const;

	void SetRow(const int& row);
	void SetCol(const int& col);

	std::string ToString() const;

	//TODO: maybe it would be better to create this class as more of a wrapper than inheritance to not allow for mixing up of types accidentally
	Array2DPosition operator+(const Array2DPosition&) const;
	Array2DPosition operator-(const Array2DPosition&) const;
	Array2DPosition operator*(const Array2DPosition&) const;
	Array2DPosition operator*(const int) const;
	Array2DPosition operator/(const Array2DPosition&) const;
	Array2DPosition operator/(const int) const;

	bool operator==(const Array2DPosition&) const;
	bool operator!=(const Array2DPosition&) const;
	bool operator<(const Array2DPosition&) const;
	bool operator<=(const Array2DPosition&) const;
	bool operator>(const Array2DPosition&) const;
	bool operator>=(const Array2DPosition&) const;

	//This is needed because const vars delete = operator definition
	Array2DPosition& operator=(const Array2DPosition&);
	Array2DPosition& operator=(Array2DPosition&& other) noexcept;

	explicit operator Utils::Point2DInt() const;
};

Array2DPosition GetAsArray2DPos(const Utils::Point2DInt& pos);