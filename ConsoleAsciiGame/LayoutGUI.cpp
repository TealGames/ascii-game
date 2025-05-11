#include "pch.hpp"
#include "LayoutGUI.hpp"

LayoutGUI::LayoutGUI(const LayoutType type, const SizingType sizing, const NormalizedPosition spacing)
	: m_type(type), m_sizingType(sizing), m_spacing(spacing) {}

void LayoutGUI::AddLayoutElement(GUIElement* element)
{
	PushChild(element);
	//TODO: an optimization could be to update the size of this newly pushed child
	//and if we know whether internal state of gui element changed, we can prevent
	//unnecessary layout updates
}

void LayoutGUI::Update(const float deltaTime) 
{
	LayoutUpdate();
}
RenderInfo LayoutGUI::Render(const RenderInfo& renderInfo)
{
	return renderInfo;
}

void LayoutGUI::LayoutUpdate()
{
	if (GetChildCount() == 0) return;

	NormalizedPosition totalSizeNorm = {};
	for (const auto& child : GetChildrenMutable())
	{
		totalSizeNorm = totalSizeNorm+ child->GetSize();
	}
	totalSizeNorm = totalSizeNorm+ m_spacing * (GetChildCount() - 1);
	if (m_sizingType == SizingType::ExpandParent) SetSize(totalSizeNorm);

	//TODO: this currently only does vertical layout
	NormalizedPosition currentPosNorm = NormalizedPosition::TOP_LEFT;
	float sizeFactorY = 1;
	if (m_sizingType == SizingType::ShrinkChildren && totalSizeNorm.GetY() > GetSize().GetY())
		sizeFactorY = GetSize().GetY() / totalSizeNorm.GetY();

	for (auto& child : GetChildrenMutable())
	{
		child->SetTopLeftPos(currentPosNorm);
		child->SetSize(child->GetSize() * sizeFactorY);

		currentPosNorm.GetPosMutable().m_Y -= m_spacing.GetY();
	}
}
