#pragma once
#include "NormalizedPosition.hpp"
#include "ScreenPosition.hpp"
#include <string>

class RelativeGUIRect
{
private:
	NormalizedPosition m_topLeftPos;
	NormalizedPosition m_bottomRightPos;
public:

private:
public:
	RelativeGUIRect();
	RelativeGUIRect(const NormalizedPosition& topLeft, const NormalizedPosition& size);

	/// <summary>
	/// Will update the size of the rect. If keeptopleft is TRUE, it will update bottom right pos
	/// otherwise, if FALSE, will update the topleftpos
	/// </summary>
	/// <param name="size"></param>
	/// <param name="keepTopLeft"></param>
	void SetSize(const NormalizedPosition& size, const bool keepTopLeft);
	void SetMaxSize();
	void SetTopLeft(const NormalizedPosition& topLeft);
	void SetBottomRight(const NormalizedPosition& bottomRight);
	void SetBounds(const NormalizedPosition& topLeft, const NormalizedPosition& bottomRight);

	NormalizedPosition GetSize() const;
	ScreenPosition GetSize(const ScreenPosition parentSize) const;

	const NormalizedPosition& GetTopLeftPos() const;
	ScreenPosition GetTopLeftPos(const ScreenPosition parentSize) const;

	const NormalizedPosition& GetBottomRighttPos() const;
	ScreenPosition GetBottomRighttPos(const ScreenPosition parentSize) const;

	std::string ToString() const;
};

ScreenPosition GetRealPos(const NormalizedPosition relativePos, const ScreenPosition& parentSize);


