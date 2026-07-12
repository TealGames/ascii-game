#pragma once
#include <string>
#include "Core/Primitives/NormalizedVec2.hpp"
#include "Core/Primitives/ScreenPosition.hpp"
#include "Math/Math3d.hpp"

namespace Engine::UI
{
	inline const NormalizedVec2 UI_RECT_BOTTOM_LEFT = NormalizedVec2(NormalizedValue::MIN, NormalizedValue::MIN);
	inline const NormalizedVec2 UI_RECT_TOP_LEFT = NormalizedVec2(NormalizedValue::MIN, NormalizedValue::MAX);
	inline const NormalizedVec2 UI_RECT_TOP_RIGHT = NormalizedVec2(NormalizedValue::MAX, NormalizedValue::MAX);
	inline const NormalizedVec2 UI_RECT_BOTTOM_RIGHT = NormalizedVec2(NormalizedValue::MAX, NormalizedValue::MIN);
	inline const NormalizedVec2 UI_RECT_CENTER = NormalizedVec2(NormalizedValue::HALF, NormalizedValue::HALF);

	/// <summary>
	/// Represents a User Interface rectangle defined by a top left position, bottom right position and a pivot
	/// relative to the parent's rectangle
	/// </summary>
	class UIRect
	{
	private:
		/// <summary>
		/// The bottom left anchor of this rectangle RELATIVE TO THE PARENT'S SIZE
		/// EX. (0.2, 0.4) -> the pos is 20% of the parent's size RIGHT, 40% of the parent's size UP 
		/// from PARENT'S BOTTOM LEFT position
		/// </summary>
		NormalizedVec2 m_bottomLeftPos;
		/// <summary>
		/// The bottom right anchor of this rectangle RELATIVE TO THE PARENT'S SIZE
		/// EX. (0.2, 0.4) -> the pos is 20% of the parent's size RIGHT, 40% of the parent's size UP 
		/// from PARENT'S BOTTOM LEFT position
		/// </summary>
		NormalizedVec2 m_topRightPos;
	public:
		/// <summary>
		/// The point within this rect that determines where
		/// scaling and rotation is applied to this rect
		/// EX. Center pivot behaves as expected and size increases in all direction evenly
		/// but with BottomRight pivot, left and top directions expand
		/// </summary>
		NormalizedVec2 m_Pivot;

	private:
		bool HasValidSize() const;

	public:
		UIRect();
		UIRect(const NormalizedVec2& size);
		UIRect(const NormalizedVec2& bottomLeft, const NormalizedVec2& size, const NormalizedVec2& pivot = UI_RECT_CENTER);

		void SetSizeWithPivot(const NormalizedVec2& size, const NormalizedVec2& pivot);
		void SetSize(const NormalizedVec2& size);
		void SetMaxSize();
		void SetBottomLeft(const NormalizedVec2& bottomLeft, const bool maintainSize = true);
		void SetBottomRight(const NormalizedVec2& bottomRight, const bool maintainSize = true);
		void SetTopRight(const NormalizedVec2& topRight, const bool maintainSize = true);
		void SetTopLeft(const NormalizedVec2& topLeft, const bool maintainSize = true);

		void SetBoundsBLTR(const NormalizedVec2& bottomLeft, const NormalizedVec2& topRight);
		void SetBoundsTLBR(const NormalizedVec2& topLeft, const NormalizedVec2& bottomRight);

		NormalizedVec2 GetSize() const;
		bool ContainsPos(const NormalizedVec2& pos) const;

		const NormalizedVec2& GetBottomLeftPos() const;
		NormalizedVec2 GetTopLeftPos() const;
		const NormalizedVec2& GetTopRightPos() const;
		NormalizedVec2 GetBottomRightPos() const;

		Mat3 CalculateModelMatrix() const;

		std::string ToString() const;
	};

	UIRect CalculateWorldRect(const UIRect& parentWorldRect, const UIRect& localRect);
}



