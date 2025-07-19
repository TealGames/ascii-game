#include "pch.hpp"
#include <cmath>
#include <string>
#include <format>
#include "Utils/HelperFunctions.hpp"
#include "Vec2Int.hpp"
#include "Point2D.hpp"
#include "Point2DInt.hpp"
#include "Core/Analyzation/Debug.hpp"

const Vec2Int Vec2Int::UP(0, 1);
const Vec2Int Vec2Int::DOWN(0, -1);
const Vec2Int Vec2Int::LEFT(-1, 0);
const Vec2Int Vec2Int::RIGHT(1, 0);

const Vec2Int Vec2Int::NORTH = UP;
const Vec2Int Vec2Int::EAST = RIGHT;
const Vec2Int Vec2Int::SOUTH = DOWN;
const Vec2Int Vec2Int::WEST = LEFT;

const Vec2Int Vec2Int::ZERO(0, 0);
const Vec2Int Vec2Int::ONE(1, 1);

Vec2Int::Vec2Int() : m_X(0), m_Y(0) {}

Vec2Int::Vec2Int(const int& xComp, const int& yComp)
	: m_X(xComp), m_Y(yComp)
{

}

Vec2Int Vec2Int::GetXAsVector() const
{
	return { m_X, 0 };
}

Vec2Int Vec2Int::GetYAsVector() const
{
	return { 0, m_Y };
}

Vec2Int Vec2Int::GetFlipped() const
{
	return { m_Y, m_X };
}

float Vec2Int::GetAngle(const AngleMode& angleMode) const
{
	//Just in case to prevent implementations returning undefined
	if (Utils::ApproximateEqualsF(m_X, 0)
		&& Utils::ApproximateEqualsF(m_Y, 0)) return 0;

	const float rad = std::atan2(m_Y, m_X);
	if (angleMode == AngleMode::Degrees) return Utils::ToDegrees(rad);

	return rad;
}

float Vec2Int::GetMagnitude() const
{
	return std::sqrt(std::pow(m_X, 2) + std::pow(m_Y, 2));
}

Vec2Int Vec2Int::GetNormalized() const
{
	const float magnitude = GetMagnitude();
	if (!Utils::ApproximateEqualsF(magnitude, 0))
		return Vec2Int::ZERO;

	return Vec2Int(m_X / magnitude, m_Y / magnitude);
}

Vec2Int Vec2Int::GetNormal() const
{
	return Vec2Int(m_Y, -m_X);
}

Vec2Int Vec2Int::GetOppositeDirection() const
{
	return Vec2Int(-m_X, -m_Y);
}

bool Vec2Int::IsUnitVector() const
{
	return Utils::ApproximateEqualsF(GetMagnitude(), 1);
}

Vec2Int Vec2Int::operator+(const Vec2Int& otherVec) const
{
	return { m_X + otherVec.m_X, m_Y + otherVec.m_Y };
}

Vec2Int Vec2Int::operator-(const Vec2Int& otherVec) const
{
	return { m_X - otherVec.m_X, m_Y - otherVec.m_Y };
}

Vec2Int Vec2Int::operator*(const Vec2Int& otherVec) const
{
	return { m_X * otherVec.m_X, m_Y * otherVec.m_Y };
}

Vec2Int Vec2Int::operator*(const float& scalar) const
{
	return Vec2Int(m_X * scalar, m_Y * scalar);
}

Vec2Int Vec2Int::operator/(const float& scalar) const
{
	if (!Assert(scalar != 0,
		std::format("Tried to divide a vector: {} by a 0 value scalar", ToString())))
	{
		return *this;
	}

	return Vec2Int(m_X / scalar, m_Y / scalar);
}

bool Vec2Int::operator==(const Vec2Int& otherVec) const
{
	return m_X == otherVec.m_X && m_Y == otherVec.m_Y;
}

Vec2Int& Vec2Int::operator=(const Vec2Int& other)
{
	if (this == &other)
		return *this;

	m_X = other.m_X;
	m_Y = other.m_Y;
	return *this;
}

Vec2Int& Vec2Int::operator=(Vec2Int&& other) noexcept
{
	m_X = std::exchange(other.m_X, 0.0);
	m_Y = std::exchange(other.m_Y, 0.0);
	return *this;
}

std::string Vec2Int::ToString(VectorForm form) const
{
	std::string str;
	switch (form)
	{
	case VectorForm::Component:
		str = std::format("({},{})", std::to_string(m_X), std::to_string(m_Y));
		break;

	case VectorForm::Unit:
		//Double braces needed on the outside to escape format {}
		str = std::format("{{ {}i+{}j }}", std::to_string(m_X), std::to_string(m_Y));
		break;

	case VectorForm::MagnitudeDirection:
		str = std::format("{}@ {}°", std::to_string(GetMagnitude()), std::to_string(GetAngle(AngleMode::Degrees)));
		break;

	default:
		LogError(std::format("Tried to convert vector ({},{}) to string "
			"with undefined form {}", m_X, m_Y, ToString(form)));
		break;
	}
	return str;
}

float GetDistance(const Vec2Int& vec1, const Vec2Int& vec2)
{
	return std::sqrt(std::pow(vec1.m_X - vec2.m_X, 2) + std::pow(vec1.m_Y - vec2.m_Y, 2));
}

Vec2Int GetVector(const Vec2Int& startPos, const Vec2Int& endPos)
{
	return Vec2Int(endPos.m_X - startPos.m_X, endPos.m_Y - startPos.m_Y);
}
Vec2Int GetVector(const Vec2Int& unitVector, const float& magnitude)
{
	Vec2Int unitVectorConverted = unitVector.IsUnitVector() ? unitVector : unitVector.GetNormalized();
	return Vec2Int(unitVectorConverted.m_X * magnitude, unitVectorConverted.m_Y * magnitude);
}

Vec2Int GetVectorEndPoint(const Vec2Int& startPos, const Vec2Int& vector)
{
	return { startPos.m_X + vector.m_X, startPos.m_Y + vector.m_Y };
}

float DotProduct(const Vec2Int& vecA, const Vec2Int& vecB)
{
	return (vecA.m_X * vecB.m_X) + (vecA.m_Y * vecB.m_Y);
}

Vec2Int GenerateRandomVec2Int(const Vec2Int& minVec, const Vec2Int maxVec)
{
	if (!Assert(minVec.m_X <= maxVec.m_X, std::format("Tried to generate random vec2int between:{} "
		"and {} but min x is greater than max x", minVec.ToString(), maxVec.ToString())))
		return {};

	if (!Assert(minVec.m_Y <= maxVec.m_Y, std::format("Tried to generate random vec2int between:{} "
		"and {} but min y is greater than max y", minVec.ToString(), maxVec.ToString())))
		return {};

	return Vec2Int{ Utils::GenerateRandomInt(minVec.m_X, maxVec.m_X),
				    Utils::GenerateRandomInt(minVec.m_Y, maxVec.m_Y) };
}

