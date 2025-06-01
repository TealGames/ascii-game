#pragma once
#include "GUIElement.hpp"
#include "raylib.h"

enum class LayoutType
{
	Vertical,
	Horizontal,
	Grid,
};

enum class SizingType
{
	/// <summary>
	/// Will ignore any sizing and the elements will be rendered based on the type with
	/// no scale factor applied in order to fit them. If they surpass 
	/// </summary>
	None,
	/// <summary>
	/// If the amount of elements in the layout can be EXPANDED to fit the total area
	/// an appropriate scale factor will be applied while maintaining proportions
	///  Even of the size can be shrunk, it will be left as default
	/// </summary>
	ExpandOnly,
	// <summary>
	/// If the amount of elements in the layout can be SHRUNK to fit the total area
	/// an appropriate scale factor will be applied while maintaining proportions. 
	/// Even of the size can be enlarged, it will be left as default
	/// </summary>
	ShrinkOnly,
	// <summary>
	/// If the amount of elements in the layout can be expanded to fit the total area
	/// OR shrunk an appropriate scale factor will be applied while maintaining proportions
	/// depending on if it needs to be shrunk/enlarged
	/// </summary>
	ExpandAndShrink,
};

class LayoutGUI : public ComponentData
{
private:
	LayoutType m_type;
	SizingType m_sizingType;
	NormalizedPosition m_spacing; 
	Color m_backgroundColor;
public:

private:
	void LayoutUpdate();
public:
	LayoutGUI(const LayoutType type, const SizingType sizing, const NormalizedPosition spacing = {}, const Color background = {});

	void AddLayoutElement(GUIElement* element);
	void RemoveLayoutElements(const size_t& childStartIndex, const size_t& count);
	Vec2 GetTotalSizeUsed() const;

	void Update(const float deltaTime) override;
	RenderInfo Render(const RenderInfo& renderInfo) override;
};

