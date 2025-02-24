#pragma once
#include "Point2DInt.hpp"
#include "ScreenPosition.hpp"
#include "NormalizedPosition.hpp"

class GUIRect
{
private:
	ScreenPosition m_topLeftPos;
	Utils::Point2DInt m_size;
	ScreenPosition m_bottomRightPos;

public:
	GUIRect();
	GUIRect(const ScreenPosition& topLeftPos, const Utils::Point2DInt& size);

	ScreenPosition GetPosWithinRect(const NormalizedPosition& normalizedPos) const;
	bool ContainsPos(const ScreenPosition& pos) const;

	const ScreenPosition& GetTopleftPos() const;
	const ScreenPosition& GetBottomRightPos() const;
	const Utils::Point2DInt& GetSize() const;

	void SetTopLeftPos(const ScreenPosition& pos);
	void SetSize(const Utils::Point2DInt& size);

	std::string ToString() const;
};

