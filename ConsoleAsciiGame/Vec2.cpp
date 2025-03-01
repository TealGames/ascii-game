#include "pch.hpp"
#include <cmath>
#include <string>
#include <format>
#include "HelperFunctions.hpp"
#include "Vec2.hpp"
#include "Point2D.hpp"
#include "Point2DInt.hpp"
#include "Debug.hpp"

const Vec2 Vec2::UP(0, 1);
const Vec2 Vec2::DOWN(0, -1);
const Vec2 Vec2::LEFT(-1, 0);
const Vec2 Vec2::RIGHT(1, 0);

const Vec2 Vec2::NORTH =	UP;
const Vec2 Vec2::NORTHEAST = { 0.5, 0.5 };
const Vec2 Vec2::EAST =		RIGHT;
const Vec2 Vec2::SOUTHEAST= { 0.5, -0.5 };
const Vec2 Vec2::SOUTH =	DOWN;
const Vec2 Vec2::SOUTHWEST= { -0.5, -0.5 };
const Vec2 Vec2::WEST =		LEFT;
const Vec2 Vec2::NORTHWEST= { -0.5, 0.5 };

const Vec2 Vec2::ZERO(0, 0);
const Vec2 Vec2::ONE(1, 1);

Vec2::Vec2() : m_X(0), m_Y(0) {}

Vec2::Vec2(const float& xComp, const float& yComp)
	: m_X(xComp), m_Y(yComp)
{

}

Vec2 Vec2::GetXAsVector() const
{
	return { m_X, 0 };
}

Vec2 Vec2::GetYAsVector() const
{
	return {0, m_Y};
}

float Vec2::GetAngle(const AngleMode& angleMode) const
{
	//Just in case to prevent implementations returning undefined
	if (Utils::ApproximateEqualsF(m_X, 0) 
		&& Utils::ApproximateEqualsF(m_Y, 0)) return 0;

	float rad = std::atan2(m_Y, m_X);
	if (angleMode == AngleMode::Degrees) return Utils::ToDegrees(rad);
	
	return rad;
}

float Vec2::GetMagnitude() const
{
	return std::sqrt(std::pow(m_X, 2) + std::pow(m_Y, 2));
}

Vec2 Vec2::GetNormalized() const
{
	float magnitude = GetMagnitude();
	if (!Assert(this, !Utils::ApproximateEqualsF(magnitude, 0),
		std::format("Tried to normalize vector: {} but it has magnitude 0", ToString())))
		return Vec2::ZERO;

	return Vec2(m_X / magnitude, m_Y / magnitude);
}

Vec2 Vec2::GetNormal() const
{
	return Vec2(m_Y, -m_X);
}

Vec2 Vec2::GetOppositeDirection() const
{
	return Vec2(-m_X, -m_Y);
}

bool Vec2::IsUnitVector() const
{
	return Utils::ApproximateEqualsF(GetMagnitude(), 1);
}

std::string Vec2::ToString(VectorForm form) const
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
		str = std::format("{}@ {}�", std::to_string(GetMagnitude()), std::to_string(GetAngle(AngleMode::Degrees)));
		break;

	default:
		LogError(this, std::format("Tried to convert vector ({},{}) to string "
			"with undefined form {}", m_X, m_Y, ToString(form)));
		break;
	}
	return str;
}

Vec2 Vec2::operator+(const Vec2& otherVec) const
{
	return {m_X + otherVec.m_X, m_Y + otherVec.m_Y};
}

Vec2 Vec2::operator-(const Vec2& otherVec) const
{
	return { m_X - otherVec.m_X, m_Y - otherVec.m_Y };
}

Vec2 Vec2::operator*(const Vec2& otherVec) const
{
	return { m_X * otherVec.m_X, m_Y * otherVec.m_Y };
}

Vec2 Vec2::operator*(const float& scalar) const
{
	return { m_X * scalar, m_Y * scalar };
}

Vec2 Vec2::operator/(const float& scalar) const
{
	if (!Assert(this, scalar != 0,
		std::format("Tried to divide a vector: {} by a 0 value scalar", ToString())))
	{
		return *this;
	}

	return { m_X / scalar, m_Y / scalar };
}

bool Vec2::operator==(const Vec2& otherVec) const
{
	bool sameX = Utils::ApproximateEquals(m_X, otherVec.m_X);
	bool sameY = Utils::ApproximateEquals(m_Y, otherVec.m_Y);
	return sameX && sameY;
}

Vec2& Vec2::operator=(const Vec2& other)
{
	if (this == &other)
		return *this;

	m_X = other.m_X;
	m_Y = other.m_Y;
	return *this;
}

Vec2& Vec2::operator=(Vec2&& other) noexcept
{
	m_X = std::exchange(other.m_X, 0.0);
	m_Y = std::exchange(other.m_Y, 0.0);
	return *this;
}

Vec2 GetVector(const Utils::Point2D& startPos, const Utils::Point2D& endPos)
{
	Vec2 result(endPos.m_X - startPos.m_X, endPos.m_Y - startPos.m_Y);
	return result;
}
Vec2 GetVector(const Utils::Point2DInt& startPos, const Utils::Point2DInt& endPos)
{
	Vec2 result(static_cast<double>(endPos.m_X - startPos.m_X), endPos.m_Y - startPos.m_Y);
	return result;
}
Vec2 GetVector(const Utils::Point2DInt& intVec)
{
	return { static_cast<float>(intVec.m_X), static_cast<float>(intVec.m_Y) };
}
Vec2 GetVector(const Vec2& unitVector, const float& magnitude)
{
	Vec2 unitVectorConverted = unitVector.IsUnitVector() ? unitVector : unitVector.GetNormalized();
	return { unitVectorConverted.m_X * magnitude, unitVectorConverted.m_Y * magnitude};
}

Utils::Point2D GetVectorEndPoint(const Utils::Point2D& startPos, const Vec2& vector)
{
	return { startPos.m_X + vector.m_X, startPos.m_Y + vector.m_Y };
}
Utils::Point2DInt GetVectorEndPoint(const Utils::Point2DInt& startPos, const Vec2& vector)
{
	return { static_cast<int>(startPos.m_X + vector.m_X), static_cast<int>(startPos.m_Y + vector.m_Y) };
}

std::string ToString(const AngleMode& mode)
{
	if (mode == AngleMode::Degrees) return "Degrees";
	else if (mode == AngleMode::Radians) return "Radians";
	else
	{
		LogError("Tried to convert undefined Vector2 angle mode to string");
		return "";
	}
}

std::string ToString(const VectorForm& form)
{
	if (form == VectorForm::Component) return "Component";
	else if (form == VectorForm::MagnitudeDirection) return "Magnitude@Direction";
	else if (form == VectorForm::Unit) return "Unit";
	else
	{
		LogError("Tried to convert undefined Vec2 form mode to string");
		return "";
	}
}

float DotProduct(const Vec2& vecA, const Vec2& vecB)
{
	return (vecA.m_X * vecB.m_X) + (vecA.m_Y * vecB.m_Y);
}