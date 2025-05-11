#pragma once
#include "GUIElement.hpp"

enum class LayoutType
{
	Vertical,
	Horizontal,
	Grid,
};

enum class SizingType
{
	ExpandParent,
	ShrinkChildren,
};

class LayoutGUI : public GUIElement
{
private:
	LayoutType m_type;
	SizingType m_sizingType;
	NormalizedPosition m_spacing;
public:

private:
	void LayoutUpdate();
public:
	LayoutGUI(const LayoutType type, const SizingType sizing, const NormalizedPosition spacing = {});

	void AddLayoutElement(GUIElement* element);

	void Update(const float deltaTime) override;
	RenderInfo Render(const RenderInfo& renderInfo) override;
};

