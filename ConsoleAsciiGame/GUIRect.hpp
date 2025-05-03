#pragma once
#include "Vec2Int.hpp"
#include "ScreenPosition.hpp"
#include "WorldPosition.hpp"
#include "NormalizedPosition.hpp"

class GUIRect
{
private:
	ScreenPosition m_topLeftPos;
	Vec2Int m_size;
	ScreenPosition m_bottomRightPos;

public:
	GUIRect();
	GUIRect(const ScreenPosition& topLeftPos, const Vec2Int& size);

	ScreenPosition GetPosWithinRect(const NormalizedPosition& normalizedPos) const;
	bool ContainsPos(const ScreenPosition& pos) const;
	bool ContainsPos(const WorldPosition& pos) const;

	const ScreenPosition& GetTopleftPos() const;
	const ScreenPosition& GetBottomRightPos() const;
	const Vec2Int& GetSize() const;

	void SetTopLeftPos(const ScreenPosition& pos);
	void SetSize(const Vec2Int& size);

	std::string ToString() const;
};

