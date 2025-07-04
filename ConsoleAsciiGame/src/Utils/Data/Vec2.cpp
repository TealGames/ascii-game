#include "pch.hpp"
#include <cmath>
#include <numbers>
#include <string>
#include <format>
#include "Utils/HelperFunctions.hpp"
#include "Vec2.hpp"
#include "Point2D.hpp"
#include "Point2DInt.hpp"
#include "Core/Analyzation/Debug.hpp"

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

Vec2 Vec2::GetFlipped() const
{
	return {m_Y, m_X};
}

int Vec2::XAsInt() const
{
	return static_cast<int>(m_X);
}

int Vec2::YAsInt() const
{
	return static_cast<int>(m_Y);
}

float Vec2::GetAngle(const AngleMode& angleMode) const
{

	//Just in case to prevent implementations returning undefined
	if (Utils::ApproximateEqualsF(m_X, 0) 
		&& Utils::ApproximateEqualsF(m_Y, 0)) return 0;

	float rad = std::atan2(m_Y, m_X);
	//Since atan2 gives you result in [-pi/2, pi/2) we can flip negaative rads
	//to their positive by doing full rotation other way

	//TODO: i think this is wrong because we get negative angles in both the second and fourth quadrants
	//which would not make sense to add 2pi to second quadrant angle. Also, this means all are in terms of -90, 90
	//which would also make the third quadrant wrong as well
	if (rad < 0) rad += 2 * std::numbers::pi;

	if (angleMode == AngleMode::Degrees) return Utils::ToDegrees(rad);
	
	return rad;
}

float Vec2::GetMagnitude() const
{
	return std::sqrt(std::pow(m_X, 2) + std::pow(m_Y, 2));
}

Vec2 Vec2::GetNormalized() const
{
	const float magnitude = GetMagnitude();
	if (Utils::ApproximateEqualsF(magnitude, 0))
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

std::string Vec2::ToString(const std::uint8_t& decimalPlaces, VectorForm form) const
{
	std::string str;
	std::string xRounded = Utils::ToStringDouble(Utils::Roundf(m_X, decimalPlaces), decimalPlaces);
	std::string yRounded = Utils::ToStringDouble(Utils::Roundf(m_Y, decimalPlaces), decimalPlaces);
	switch (form)
	{
	case VectorForm::Component:
		str = std::format("({},{})", xRounded, yRounded);
		break;

	case VectorForm::Unit:
		//Double braces needed on the outside to escape format {}
		str = std::format("{{ {}i+{}j }}", xRounded, yRounded);
		break;

	case VectorForm::MagnitudeDirection:
		str = std::format("{}@ {}°", std::to_string(Utils::Roundf(m_X, GetMagnitude())), 
			std::to_string(Utils::Roundf(GetAngle(AngleMode::Degrees), decimalPlaces)));
		break;

	default:
		LogError(std::format("Tried to convert vector ({},{}) to string "
			"with undefined form {}", m_X, m_Y, ::ToString(form)));
		break;
	}
	return str;
}

Vec2 Vec2::operator+(const Vec2& otherVec) const
{
	return {m_X + otherVec.m_X, m_Y + otherVec.m_Y};
}
Vec2& Vec2::operator+=(const Vec2& other)
{
	m_X += other.m_X;
	m_Y += other.m_Y;
	return *this;
}

Vec2 Vec2::operator-(const Vec2& otherVec) const
{
	return { m_X - otherVec.m_X, m_Y - otherVec.m_Y };
}
Vec2& Vec2::operator-=(const Vec2& other)
{
	m_X -= other.m_X;
	m_Y -= other.m_Y;
	return *this;
}

Vec2 Vec2::operator*(const Vec2& otherVec) const
{
	return { m_X * otherVec.m_X, m_Y * otherVec.m_Y };
}

Vec2 Vec2::operator*(const float& scalar) const
{
	return { m_X * scalar, m_Y * scalar };
}
Vec2 Vec2::operator/(const Vec2& other) const
{
	if (!Assert(other.m_X != 0 && other.m_Y!=0,
		std::format("Tried to divide a vector: {} by a 0-value vector:{}", ToString(), other.ToString())))
	{
		return *this;
	}
	return {m_X/ other.m_X, m_Y/other.m_Y};
}

Vec2 Vec2::operator/(const float& scalar) const
{
	if (!Assert(scalar != 0,
		std::format("Tried to divide a vector: {} by a 0 value scalar", ToString())))
	{
		return *this;
	}

	return { m_X / scalar, m_Y / scalar };
}

bool Vec2::operator==(const Vec2& otherVec) const
{
	return Utils::ApproximateEquals(m_X, otherVec.m_X) && 
		Utils::ApproximateEquals(m_Y, otherVec.m_Y);
}

bool Vec2::operator!=(const Vec2& other) const
{
	return !(*this == other);
}
bool Vec2::operator>(const Vec2& other) const
{
	return m_X > other.m_X && m_Y > other.m_Y;
}
bool Vec2::operator>=(const Vec2& other) const
{
	return m_X >= other.m_X && m_Y >= other.m_Y;
}
bool Vec2::operator<(const Vec2& other) const
{
	return m_X < other.m_X && m_Y < other.m_Y;
}
bool Vec2::operator<=(const Vec2& other) const
{
	return m_X <= other.m_X && m_Y <= other.m_Y;
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

float GetDistance(const Vec2& vec1, const Vec2& vec2)
{
	return std::sqrt(std::pow(vec1.m_X - vec2.m_X, 2) + std::pow(vec1.m_Y - vec2.m_Y, 2));
}

float GetXComponent(const float& speed, const float& angle, const AngleMode& mode)
{
	const float angleRad = mode == AngleMode::Degrees ? static_cast<float>(Utils::ToRadians(angle)) : angle;
	return speed * std::cosf(angleRad);
}
float GetYComponent(const float& speed, const float& angle, const AngleMode& mode)
{
	const float angleRad = mode == AngleMode::Degrees ? static_cast<float>(Utils::ToRadians(angle)) : angle;
	return speed * std::sinf(angleRad);
}

Vec2 GetVector(const Vec2& startPos, const Vec2& endPos)
{
	return Vec2(endPos.m_X - startPos.m_X, endPos.m_Y - startPos.m_Y);
}
Vec2 GetVector(const Vec2& unitVector, const float& magnitude)
{
	Vec2 unitVectorConverted = unitVector.IsUnitVector() ? unitVector : unitVector.GetNormalized();
	return { unitVectorConverted.m_X * magnitude, unitVectorConverted.m_Y * magnitude};
}
Vec2 GetVector(const float& speed, const float& angle, const AngleMode& mode)
{
	return { GetXComponent(speed, angle, mode), GetYComponent(speed, angle, mode)};
}

Vec2 GetVectorEndPoint(const Vec2& startPos, const Vec2& vector)
{
	return { startPos.m_X + vector.m_X, startPos.m_Y + vector.m_Y };
}

float DotProduct(const Vec2& vecA, const Vec2& vecB)
{
	return (vecA.m_X * vecB.m_X) + (vecA.m_Y * vecB.m_Y);
}

Vec2 Abs(const Vec2& vec)
{
	return Vec2(std::abs(vec.m_X), std::abs(vec.m_Y));
}

Vec2 GenerateRandomVec2(const Vec2& minVec, const Vec2 maxVec)
{
	if (!Assert(minVec.m_X <= maxVec.m_X, std::format("Tried to generate random vec between:{} "
		"and {} but min x is greater than max x", minVec.ToString(), maxVec.ToString())))
		return {};

	if (!Assert(minVec.m_Y <= maxVec.m_Y, std::format("Tried to generate random vec between:{} "
		"and {} but min y is greater than max y", minVec.ToString(), maxVec.ToString())))
		return {};

	return Vec2{ static_cast<float>(Utils::GenerateRandomDouble(minVec.m_X, maxVec.m_X)),
			static_cast<float>(Utils::GenerateRandomDouble(minVec.m_Y, maxVec.m_Y)) };
}
Vec2 GenerateRandomDir()
{
	return GenerateRandomVec2({-1, -1}, {1, 1});
}