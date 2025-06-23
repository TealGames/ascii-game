#pragma once
#include "NormalizedPosition.hpp"
#include "ScreenPosition.hpp"
#include <string>

class RelativeUIRect
{
private:
	NormalizedPosition m_topLeftPos;
	NormalizedPosition m_bottomRightPos;
public:

private:
	bool HasInvalidSize() const;

public:
	RelativeUIRect();
	RelativeUIRect(const NormalizedPosition& size);
	RelativeUIRect(const NormalizedPosition& topLeft, const NormalizedPosition& size);

	void SetSize(const NormalizedPosition& size);
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

ScreenPosition GetSizeFromRelativePos(const NormalizedPosition& relativePos, const ScreenPosition& parentSize);
ScreenPosition GetSizeFromFactor(const Vec2& factor, const ScreenPosition& parentSize);


