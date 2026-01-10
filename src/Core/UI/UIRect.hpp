#pragma once
#include <string>
#include "Utils/Data/NormalizedPosition.hpp"
#include "Utils/Data/ScreenPosition.hpp"
#include "Utils/MathAdvanced.hpp"

inline const NormalizedPos UI_RECT_BOTTOM_LEFT = NormalizedPos(NormalizedValue::MIN, NormalizedValue::MIN);
inline const NormalizedPos UI_RECT_TOP_LEFT = NormalizedPos(NormalizedValue::MIN, NormalizedValue::MAX);
inline const NormalizedPos UI_RECT_TOP_RIGHT = NormalizedPos(NormalizedValue::MAX, NormalizedValue::MAX);
inline const NormalizedPos UI_RECT_BOTTOM_RIGHT = NormalizedPos(NormalizedValue::MAX, NormalizedValue::MIN);
inline const NormalizedPos UI_RECT_CENTER = NormalizedPos(NormalizedValue::HALF, NormalizedValue::HALF);

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
	NormalizedPos m_bottomLeftPos;
	/// <summary>
	/// The bottom right anchor of this rectangle RELATIVE TO THE PARENT'S SIZE
	/// EX. (0.2, 0.4) -> the pos is 20% of the parent's size RIGHT, 40% of the parent's size UP 
	/// from PARENT'S BOTTOM LEFT position
	/// </summary>
	NormalizedPos m_topRightPos;
public:
	/// <summary>
	/// The point within this rect that determines where
	/// scaling and rotation is applied to this rect
	/// EX. Center pivot behaves as expected and size increases in all direction evenly
	/// but with BottomRight pivot, left and top directions expand
	/// </summary>
	NormalizedPos m_Pivot;

private:
	bool HasValidSize() const;

public:
	UIRect();
	UIRect(const NormalizedPos& size);
	UIRect(const NormalizedPos& bottomLeft, const NormalizedPos& size, const NormalizedPos& pivot = UI_RECT_CENTER);

	void SetSizeWithPivot(const NormalizedPos& size, const NormalizedPos& pivot);
	void SetSize(const NormalizedPos& size);
	void SetMaxSize();
	void SetBottomLeft(const NormalizedPos& bottomLeft, const bool maintainSize = true);
	void SetBottomRight(const NormalizedPos& bottomRight, const bool maintainSize = true);
	void SetTopRight(const NormalizedPos& topRight, const bool maintainSize = true);
	void SetTopLeft(const NormalizedPos& topLeft, const bool maintainSize = true);

	void SetBoundsBLTR(const NormalizedPos& bottomLeft, const NormalizedPos& topRight);
	void SetBoundsTLBR(const NormalizedPos& topLeft, const NormalizedPos& bottomRight);

	NormalizedPos GetSize() const;
	bool ContainsPos(const NormalizedPos& pos) const;

	const NormalizedPos& GetBottomLeftPos() const;
	NormalizedPos GetTopLeftPos() const;
	const NormalizedPos& GetTopRightPos() const;
	NormalizedPos GetBottomRightPos() const;

	Mat3 CalculateModelMatrix() const;

	std::string ToString() const;
};

UIRect CalculateWorldRect(const UIRect& parentWorldRect, const UIRect& localRect);


