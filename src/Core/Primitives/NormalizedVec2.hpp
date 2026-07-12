#pragma once
#include "Core/Primitives/Vector.hpp"
#include "Core/Primitives/NormalizedValue.hpp"

namespace Engine
{
	/// <summary>
/// Represents a normalized coordinate where x is in range [0,1] 
/// and y is in range [0,1]
/// </summary>
	class NormalizedVec2
	{
	private:
	public:
		NormalizedValue m_X;
		NormalizedValue m_Y;

	private:
		//bool IsValidPos() const;
	public:

		NormalizedVec2();
		NormalizedVec2(const float& x, const float& y);
		NormalizedVec2(const Vec2& pos);
		NormalizedVec2(const NormalizedValue x, const NormalizedValue y);
		NormalizedVec2(const float x, const NormalizedValue y);
		NormalizedVec2(const NormalizedValue x, const float y);

		float GetX() const;
		float GetY() const;
		Vec2 AsVec2() const;

		void SetPos(const Vec2& relativePos);
		//void SetPosX(const float x);
		//void SetPosY(const float y);
		//void SetPosDeltaX(const float deltaX);
		//void SetPosDeltaY(const float deltaY);
		bool IsZero() const;

		std::string ToString() const;

		NormalizedVec2 operator+(const NormalizedVec2& other) const;
		NormalizedVec2 operator-(const NormalizedVec2& other) const;
		NormalizedVec2 operator*(const NormalizedVec2& other) const;
		NormalizedVec2 operator/(const NormalizedVec2& other) const;
		NormalizedVec2 operator*(const float& scalar) const;
		NormalizedVec2 operator/(const float& scalar) const;

		NormalizedVec2& operator+=(const NormalizedVec2& other);
		NormalizedVec2& operator-=(const NormalizedVec2& other);

		bool operator==(const NormalizedVec2& other) const;
		bool operator!=(const NormalizedVec2& other) const;
		bool operator>(const NormalizedVec2& other) const;
		bool operator>=(const NormalizedVec2& other) const;
		bool operator<(const NormalizedVec2& other) const;
		bool operator<=(const NormalizedVec2& other) const;
	};
}


