#pragma once
#include "Utils/Data/ScreenPosition.hpp"
#include "Utils/Data/WorldPosition.hpp"
#include "Utils/Data/NormalizedPosition.hpp"

class UIRect
{
private:
public:
	ScreenPosition m_TopLeftPos;
	//Vec2Int m_size;
	ScreenPosition m_BottomRightPos;

private:
public:
	UIRect();
	UIRect(const ScreenPosition& topLeftPos, const Vec2& size);

	ScreenPosition GetPosWithinRect(const NormalizedPosition& normalizedPos) const;
	bool ContainsPos(const ScreenPosition& pos) const;

	Vec2 GetSize() const;

	void SetTopLeftPos(const ScreenPosition& pos);
	void SetSize(const Vec2& size);

	std::string ToString() const;
};

