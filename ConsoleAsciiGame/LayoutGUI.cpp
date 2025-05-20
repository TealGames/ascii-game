#include "pch.hpp"
#include "LayoutGUI.hpp"
#include "Debug.hpp"

LayoutGUI::LayoutGUI(const LayoutType type, const SizingType sizing, const NormalizedPosition spacing, const Color background)
	: m_type(type), m_sizingType(sizing), m_spacing(spacing), m_backgroundColor(background) {}

void LayoutGUI::AddLayoutElement(GUIElement* element)
{
	PushChild(element);
	//TODO: an optimization could be to update the size of this newly pushed child
	//and if we know whether internal state of gui element changed, we can prevent
	//unnecessary layout updates
}
void LayoutGUI::RemoveLayoutElements(const size_t& childStartIndex, const size_t& count)
{
	TryPopChildren(childStartIndex, count);
}

Vec2 LayoutGUI::GetTotalSizeUsed() const
{
	Vec2 size = {};
	for (const auto& child : GetChildren())
	{
		size += child->GetSize().GetPos();
	}
	return size;
}

void LayoutGUI::Update(const float deltaTime) 
{
	LayoutUpdate();
}
RenderInfo LayoutGUI::Render(const RenderInfo& renderInfo)
{
	if (m_backgroundColor.a != 0)
	{
		DrawRectangle(renderInfo.m_TopLeftPos.m_X, renderInfo.m_TopLeftPos.m_Y,
			renderInfo.m_RenderSize.m_X, renderInfo.m_RenderSize.m_Y, m_backgroundColor);
	}
	return renderInfo;
}

void LayoutGUI::LayoutUpdate()
{
	if (GetChildCount() == 0) return;

	//SetMaxSize();
	Vec2 currentSizeTakenNorm = {};
	NormalizedPosition currentSize = {};
	//float layoutTypeBasedSizeMax = 0;

	float currentRowLenNorm = 0;
	float totalHeight = 0;
	for (const auto& child : GetChildrenMutable())
	{
		currentSize = child->GetSize();
		
		//If the type is a grid, we then need to apply grid-like sizing to ensure we can apply
		//a correct scale factor
		if (m_type == LayoutType::Grid)
		{
			if (currentSizeTakenNorm.m_X + currentSize.GetX() + m_spacing.GetX() < NormalizedPosition::MAX)
				currentSizeTakenNorm.m_X += currentSize.GetX() + m_spacing.GetX();
			else
			{
				currentSizeTakenNorm.m_X = currentSize.GetX() + m_spacing.GetX();
				currentSizeTakenNorm.m_Y += currentSize.GetY() + m_spacing.GetY();
			}
		}
		else
		{
			currentSizeTakenNorm += currentSize.GetPos();
		}

		/*if (m_type == LayoutType::Horizontal && layoutTypeBasedSizeMax > currentSize.GetY()) 
			layoutTypeBasedSizeMax = currentSize.GetY();
		else if (m_type == LayoutType::Vertical && layoutTypeBasedSizeMax > currentSize.GetX())
			layoutTypeBasedSizeMax = currentSize.GetX();*/
	}
	if (m_type!=LayoutType::Grid) currentSizeTakenNorm+= m_spacing.GetPos() * (GetChildCount() - 1);

	float sizeFactor = 1;
	//If we have no sizing, we leave the size factor as one
	if (m_sizingType != SizingType::None)
	{
		//We base size factor off of total x for HORIZONTAL
		if (m_type == LayoutType::Horizontal) sizeFactor = 1 / currentSizeTakenNorm.m_X;
		//If we have VERTICAL layout or GRID layoyt, we base size factor off of total y
		else sizeFactor = 1 / currentSizeTakenNorm.m_Y;

		//If we have a special sizing type, we must make sure to apply min/max to ensure we do not do the wrong option
		if (m_sizingType == SizingType::ExpandOnly) sizeFactor = std::max(float(1), sizeFactor);
		else if (m_sizingType == SizingType::ShrinkOnly) sizeFactor = std::min(float(1), sizeFactor);
	}
	//if (m_sizingType == SizingType::ExpandParent) SetSize(totalSizeNorm);

	//TODO: this currently only does vertical layout
	NormalizedPosition currentPosNorm = NormalizedPosition::TOP_LEFT;
	/*float sizeFactorY = 1;
	if (m_sizingType == SizingType::ShrinkChildren && totalSizeNorm.GetY() > GetSize().GetY())
		sizeFactorY = GetSize().GetY() / totalSizeNorm.GetY();*/

	const auto& children = GetChildrenMutable();
	for (size_t i=0; i<children.size(); i++)
	{
		if (children[i] == nullptr) continue;

		if (currentPosNorm == NormalizedPosition::BOTTOM_LEFT)
		{
			LogError(std::format("Tried to update layout of id:{} but current pos norm:{} "
				"reached a point where size would be 0. Size factor:{} totalSizeNorm:{}",
				std::to_string(GetId()), currentPosNorm.ToString(), std::to_string(sizeFactor), currentSizeTakenNorm.ToString()));

			//Note: sizing sizing of none allows for elements that do not fit, we just ignore those and can leave without any errors
			if (m_sizingType != SizingType::None) throw std::invalid_argument("Invalid layout state");
			return;
		}

		auto before = children[i]->GetRect();
		children[i]->SetTopLeftPos(currentPosNorm);
		if (sizeFactor != 1)
		{
			if (children[i]->IsFixedHorizontal()) children[i]->SetSizeY(children[i]->GetSize().GetY() * sizeFactor);
			else if (children[i]->IsFixedVertical()) children[i]->SetSizeX(children[i]->GetSize().GetX() * sizeFactor);
			else children[i]->SetSize(children[i]->GetSize() * sizeFactor);
		}
		//LogWarning(std::format("Setting children layout(ID:{}) gui:{} -> {} SCALE:{}", std::to_string(GetId()), before.ToString(), children[i]->ToStringBase(), std::to_string(sizeFactor)));

		/*LogWarning(std::format("child factor:{} total layout space:{} posNorma;{} sizeBefore:{} sizeAfter:{}", std::to_string(sizeFactor), 
			currentSizeTakenNorm.ToString(), currentPosNorm.ToString(), before.ToString(), children[i]->GetRect().ToString()));*/

		if (m_type == LayoutType::Vertical) currentPosNorm.SetPosDeltaY(-(m_spacing.GetY() + children[i]->GetSize().GetY()));
		else if (m_type == LayoutType::Horizontal) currentPosNorm.SetPosDeltaX(m_spacing.GetX() + children[i]->GetSize().GetX());
		else
		{
			currentPosNorm.SetPosDeltaX(children[i]->GetSize().GetX() + m_spacing.GetX());
			const float nextLineWidth = currentPosNorm.GetX() + (i < children.size() - 1 ? children[i + 1]->GetSize().GetX() : 0);
			if (nextLineWidth > NormalizedPosition::MAX)
			{
				currentPosNorm.SetPosX(0);
				currentPosNorm.SetPosDeltaY(-(m_spacing.GetY() + children[i]->GetSize().GetY()));
			}
			//LogError(std::format("Checking layout child:{} next line width:{} posNorm:{}", children[i]->ToStringBase(), std::to_string(nextLineWidth), currentPosNorm.ToString()));
		}

		LogError(std::format("While doing layout update for:{} child:{} old rect:{} new:{}", ToStringBase(), 
			children[i]->ToStringBase(), before.ToString(), children[i]->GetRect().ToString()));
	}

	//Assert(false, std::format("END"));
}
