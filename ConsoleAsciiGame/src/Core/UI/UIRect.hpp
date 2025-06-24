#pragma once
#include "Utils/Data/Vec2Int.hpp"
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
	UIRect(const ScreenPosition& topLeftPos, const Vec2Int& size);

	ScreenPosition GetPosWithinRect(const NormalizedPosition& normalizedPos) const;
	bool ContainsPos(const ScreenPosition& pos) const;
	bool ContainsPos(const WorldPosition& pos) const;

	Vec2Int GetSize() const;

	void SetTopLeftPos(const ScreenPosition& pos);
	void SetSize(const Vec2Int& size);

	std::string ToString() const;
};

