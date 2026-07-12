#include "pch.hpp"
#include "ECS/Component/Types/UI/UILayoutComponent.hpp"
#include "Utils/Debug.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "ECS/Component/Types/UI/UITransformComponent.hpp"
#include "Core/Serialization/Serializer.hpp"

namespace Engine::UI
{
	UILayoutComponent::UILayoutComponent(const LayoutType type, const SizingType sizing, const NormalizedVec2 spacing)
		: m_type(type), m_sizingType(sizing), m_spacing(spacing) {}

	void UILayoutComponent::AddLayoutElement(ECS::EntityData& element)
	{
		GetEntityMutable().PushChild(element);
		//TODO: an optimization could be to update the size of this newly pushed child
		//and if we know whether internal state of gui element changed, we can prevent
		//unnecessary layout updates
	}
	std::tuple<ECS::EntityData*, UITransformComponent*> UILayoutComponent::CreateLayoutElement(const std::string& name)
	{
		return GetEntityMutable().CreateChildUI(name);
	}
	void UILayoutComponent::RemoveLayoutElements(const size_t& childStartIndex, const size_t& count)
	{
		GetEntityMutable().TryPopChildren(childStartIndex, count);
	}

	Vec2 UILayoutComponent::GetTotalSizeUsed() const
	{
		Vec2 size = {};
		for (const auto& child : GetEntity().GetChildrenOfType<UITransformComponent>())
		{
			size += child->GetLocalSize().AsVec2();
		}
		return size;
	}

	void UILayoutComponent::Update(const float deltaTime)
	{
		LayoutUpdate();
	}

	void UILayoutComponent::LayoutUpdate()
	{
		//TODO: fix this so we do not need to return early
		return;
		if (GetEntity().GetChildCount() == 0) return;

		//SetMaxSize();
		Vec2 totalElementSizeUsed = {};
		NormalizedVec2 currentSize = {};
		//float layoutTypeBasedSizeMax = 0;

		float currentRowLenNorm = 0;
		int gridRows = 0;
		for (const auto& child : GetEntity().GetChildrenOfType<UITransformComponent>())
		{
			currentSize = child->GetLocalSize();

			if (m_type == LayoutType::Grid)
			{
				if (totalElementSizeUsed.m_X + currentSize.GetX() + m_spacing.GetX() < NormalizedValue::MAX)
					totalElementSizeUsed.m_X += currentSize.GetX() + m_spacing.GetX();
				else
				{
					totalElementSizeUsed.m_X = currentSize.GetX() + m_spacing.GetX();
					//Note: since we need the total element size, we ignore vertical spacing
					totalElementSizeUsed.m_Y += currentSize.GetY();
					gridRows++;
				}
			}
			else
			{
				totalElementSizeUsed += currentSize.AsVec2();
			}
		}

		float sizeFactor = 1;
		//If we have no sizing, we leave the size factor as one
		if (m_sizingType != SizingType::None)
		{
			const size_t childCount = GetEntity().GetChildCount();
			//We base size factor off of remaining horizontal space after removing x spacing and how much total elements x size (without spaces) goes over that limit
			if (m_type == LayoutType::Horizontal) sizeFactor = (NormalizedValue::MAX - m_spacing.GetX() * (childCount - 1)) / totalElementSizeUsed.m_X;
			else if (m_type == LayoutType::Vertical) sizeFactor = (NormalizedValue::MAX - m_spacing.GetY() * (childCount - 1)) / totalElementSizeUsed.m_Y;
			//Since grid does not have vertical elements the same as total children, we use total grid rows
			else sizeFactor = (NormalizedValue::MAX - m_spacing.GetY() * (gridRows - 1)) / totalElementSizeUsed.m_Y;

			//If we have a special sizing type, we must make sure to apply min/max to ensure we do not do the wrong option
			if (m_sizingType == SizingType::ExpandOnly) sizeFactor = std::max(float(1), sizeFactor);
			else if (m_sizingType == SizingType::ShrinkOnly) sizeFactor = std::min(float(1), sizeFactor);
		}
		//if (m_sizingType == SizingType::ExpandParent) SetSize(totalSizeNorm);

		//TODO: this currently only does vertical layout
		NormalizedVec2 currentPosNorm = UI_RECT_TOP_LEFT;
		const auto& children = GetEntityMutable().GetChildrenOfTypeMutable<UITransformComponent>();
		for (size_t i = 0; i < children.size(); i++)
		{
			if (children[i] == nullptr) continue;

			if (currentPosNorm == UI_RECT_BOTTOM_LEFT)
			{
				LogError(std::format("Tried to update layout of id:{} but current pos norm:{} "
					"reached a point where size would be 0. Size factor:{} totalSizeNorm:{}",
					GetEntity().ToStringId(), currentPosNorm.ToString(), std::to_string(sizeFactor), totalElementSizeUsed.ToString()));

				//Note: sizing sizing of none allows for elements that do not fit, we just ignore those and can leave without any errors
				if (m_sizingType != SizingType::None) throw std::invalid_argument("Invalid layout state");
				return;
			}

			auto before = children[i]->GetLocalRect();
			if (sizeFactor != 1)
			{
				if (children[i]->IsFixedHorizontal()) children[i]->SetRelativeSizeY(children[i]->GetLocalSize().GetY() * sizeFactor);
				else if (children[i]->IsFixedVertical()) children[i]->SetRelativeSizeX(children[i]->GetLocalSize().GetX() * sizeFactor);
				else children[i]->SetLocalSize(children[i]->GetLocalSize() * sizeFactor);
			}

			//We do error checking to prevent awkward position setting with invalid sizes
			if (m_type == LayoutType::Horizontal && currentPosNorm == UI_RECT_TOP_RIGHT)
			{
				LogError(std::format("Attempted to perform layout update for entity:{} but reached max horizontal layout pos, "
					"leaving no room for child:{}. Note : this could be due to having large children that are not sized property "
					"to fit area because SizingType is NONE!", GetEntity().ToString(), children[i]->GetEntity().ToString()));
				return;
			}
			//Note: pos should ALWAYS be updated after size in case size was max before update and new top left
			//would become unchanged due to no space to move
			children[i]->SetLocalTopLeftPos(currentPosNorm);

			if (m_type == LayoutType::Vertical)
			{
				const auto old = currentPosNorm;
				currentPosNorm.m_Y -= (m_spacing.GetY() + children[i]->GetLocalSize().GetY());
				//LogWarning(std::format("VERTICAL POS UPDATE OLD:{} NEW:{}", old.ToString(), currentPosNorm.ToString()));
			}
			else if (m_type == LayoutType::Horizontal)
			{
				currentPosNorm.m_X += (m_spacing.GetX() + children[i]->GetLocalSize().GetX());
			}
			else
			{
				currentPosNorm.m_X += (children[i]->GetLocalSize().GetX() + m_spacing.GetX());
				const float nextLineWidth = currentPosNorm.GetX() + (i < children.size() - 1 ? children[i + 1]->GetLocalSize().GetX() : 0);
				if (nextLineWidth > NormalizedValue::MAX)
				{
					currentPosNorm.m_X = 0;
					currentPosNorm.m_Y -= (m_spacing.GetY() + children[i]->GetLocalSize().GetY());
				}
				//LogError(std::format("Checking layout child:{} next line width:{} posNorm:{}", children[i]->ToStringBase(), std::to_string(nextLineWidth), currentPosNorm.ToString()));
			}

			/*LogError(std::format("While doing layout update for:{} child:{} old rect:{} new:{} size factor:{} spacing:{}", ToString(),
				children[i]->GetEntity().ToStringId(), before.ToString(), children[i]->GetRect().ToString(), std::to_string(sizeFactor), m_spacing.ToString()));*/
		}

		//Assert(false, std::format("END"));
	}

	void UILayoutComponent::InitFields()
	{
		m_Fields = {};
	}
	void UILayoutComponent::Serialize(Serialization::Serializer& serializer) const
	{
	}
	void UILayoutComponent::Deserialize(Serialization::Deserializer& deserializer)
	{
	}

	std::string UILayoutComponent::ToString() const
	{
		return std::format("[UILayout]");
	}
}
