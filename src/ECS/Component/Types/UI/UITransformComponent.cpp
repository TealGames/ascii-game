#include "pch.hpp"
#include "ECS/Component/Types/UI/UITransformComponent.hpp"
#include "Utils/HelperFunctions.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "Math/Math3d.hpp"
#include "Core/Serialization/Serializer.hpp"

namespace Engine::UI
{
	UITransformComponent::UITransformComponent() : UITransformComponent(UIRect()) {}
	UITransformComponent::UITransformComponent(const NormalizedVec2& size) : UITransformComponent(UIRect(size)) {}
	UITransformComponent::UITransformComponent(const UIRect& relativeRect)
		: m_localRect(relativeRect), m_flags(UITransformFlags::None), m_Padding(),
		m_lastGlobalScreenRect() {}

	void UITransformComponent::SetFixed(const bool horizontal, const bool vertical)
	{
		if (horizontal) ::Utils::AddFlags(m_flags, UITransformFlags::FixedHorizontal);
		else ::Utils::RemoveFlags(m_flags, UITransformFlags::FixedHorizontal);

		if (vertical) ::Utils::AddFlags(m_flags, UITransformFlags::FixedVertical);
		else ::Utils::RemoveFlags(m_flags, UITransformFlags::FixedVertical);
	}
	bool UITransformComponent::IsFixedVertical() const { return ::Utils::HasFlagAll(m_flags, UITransformFlags::FixedVertical); }
	bool UITransformComponent::IsFixedHorizontal() const { return ::Utils::HasFlagAll(m_flags, UITransformFlags::FixedHorizontal); }

	void UITransformComponent::SetLastGlobalScreenRect(const UIRect& area)
	{
		m_lastGlobalScreenRect = area;
	}
	const UIRect& UITransformComponent::GetLastGlobalScreenRect() const
	{
		return m_lastGlobalScreenRect;
	}

	void UITransformComponent::SetEventBlocker(const bool status)
	{
		if (status) ::Utils::AddFlags(m_flags, UITransformFlags::EventBlocker);
		else ::Utils::RemoveFlags(m_flags, UITransformFlags::EventBlocker);
	}
	bool UITransformComponent::IsSelectionEventBlocker() const
	{
		return ::Utils::HasFlagAll(m_flags, UITransformFlags::EventBlocker);
	}

	void UITransformComponent::UpdateFixedChildren(const Vec2& oldParentSize)
	{
		const Vec2 newParentSize = GetLocalSize().AsVec2();
		Vec2 childSize = {};
		bool isFixedHorizontal = false;
		bool isFixedVertical = false;

		if (GetEntity().GetChildCount() > 0)
		{
			for (auto& child : GetEntityMutable().GetChildrenOfTypeMutable<UITransformComponent>())
			{
				if (child == nullptr) continue;

				isFixedHorizontal = child->IsFixedHorizontal();
				isFixedVertical = child->IsFixedVertical();
				if (!isFixedHorizontal && !isFixedVertical) continue;

				childSize = child->GetLocalSize().AsVec2();
				Vec2 newSize = childSize * Vec2(isFixedHorizontal ? newParentSize.m_X / oldParentSize.m_X : 1,
					isFixedVertical ? newParentSize.m_Y / oldParentSize.m_Y : 1);

				/*if (child->GetId() == 19) LogWarning(std::format("Setting child:{} when size set for parent:{} to newsize:{} childSize:{} og parentSize:{} new parent Size:{}",
					child->ToStringBase(), ToStringBase(), newSize.ToString(), childSize.ToString(), parentSize.ToString(), size.ToString()));*/

					//Note: by default fixed horizontal/vertical elements CANNOT have those parts modified by size,
					//but we need to update fixed children from parent, so we get around these checks by using unsafe version
				if (newSize != childSize) child->SetSizeUnsafe(newSize);
			}
		}
	}
	void UITransformComponent::SetSizeUnsafe(const Vec2& size)
	{
		const Vec2 oldParentSize = GetLocalSize().AsVec2();
		m_localRect.SetSize(size);
		UpdateFixedChildren(oldParentSize);
		//m_OnSizeUpdated.Invoke(this);
	}

	bool UITransformComponent::DoLocksAllowSizeChange(const NormalizedVec2& proposedNewSize) const
	{
		const Vec2 currSize = GetLocalSize().AsVec2();
		if (IsFixedHorizontal() && !::Math::ApproximateEqualsF(proposedNewSize.GetX(), currSize.m_X))
			return false;

		if (IsFixedVertical() && !::Math::ApproximateEqualsF(proposedNewSize.GetY(), currSize.m_Y))
			return false;

		return true;
	}

	void UITransformComponent::SetLocalSize(const NormalizedVec2& newSize)
	{
		if (!DoLocksAllowSizeChange(newSize))
		{
			LogError(std::format("Attempted to set the size of UI TRANSFORM:{} to:{} but one or more locks "
				"do not permit size changes in their direction until the FIXED size for that direction is set to false. "
				"Fixed Vertical:{} Fixed Horizontal:{}", ToString(), newSize.ToString(), IsFixedVertical(), IsFixedHorizontal()));
			return;
		}

		SetSizeUnsafe(newSize.AsVec2());
	}
	void UITransformComponent::SetMaxRelativeSize() { SetLocalSize(NormalizedVec2(NormalizedValue::MAX, NormalizedValue::MAX)); }
	void UITransformComponent::SetRelativeSizeX(const float sizeNormalized) { SetLocalSize({ sizeNormalized, m_localRect.GetSize().GetY() }); }
	void UITransformComponent::SetRelativeSizeY(const float sizeNormalized) { SetLocalSize({ m_localRect.GetSize().GetX(), sizeNormalized }); }

	//NOTE: for setting the positions since we do true for maintaining size, messing up fixed vertical/horizontal is not a problem
	void UITransformComponent::SetLocalTopLeftPos(const NormalizedVec2& topLeftPos) { m_localRect.SetTopLeft(topLeftPos, true); }
	void UITransformComponent::SetLocalTopRightPos(const NormalizedVec2& topRightPos) { m_localRect.SetTopRight(topRightPos, true); }
	void UITransformComponent::SetLocalBottomRightPos(const NormalizedVec2& bottomRightPos) { m_localRect.SetBottomRight(bottomRightPos, true); }
	void UITransformComponent::SetLocalBottomLeftPos(const NormalizedVec2& bottomLeftPos) { m_localRect.SetBottomLeft(bottomLeftPos, true); }

	void UITransformComponent::SetLocalBoundsBLTR(const NormalizedVec2& bottomLeftPos, const NormalizedVec2& topRightPos)
	{
		const NormalizedVec2 newSize = topRightPos - bottomLeftPos;
		if (!DoLocksAllowSizeChange(newSize))
		{
			LogError(std::format("Attempted to set bounds of UI TRANSFORM:{} to BottomLeft:{} TopRight:{} (newSize:{}) but one or more locks "
				"do not permit size changes in their direction until the FIXED size for that direction is set to false. "
				"Fixed Vertical:{} Fixed Horizontal:{}", ToString(), bottomLeftPos.ToString(), topRightPos.ToString(),
				newSize.ToString(), IsFixedVertical(), IsFixedHorizontal()));
			return;
		}

		const Vec2 oldSize = m_localRect.GetSize().AsVec2();
		m_localRect.SetBoundsBLTR(bottomLeftPos, topRightPos);
		UpdateFixedChildren(oldSize);
	}
	void UITransformComponent::SetLocalBoundsTLBR(const NormalizedVec2& topLeftPos, const NormalizedVec2& bottomRightPos)
	{
		const NormalizedVec2 newSize = NormalizedVec2(bottomRightPos.m_X - topLeftPos.m_X, topLeftPos.m_Y - bottomRightPos.m_Y);
		if (!DoLocksAllowSizeChange(newSize))
		{
			LogError(std::format("Attempted to set bounds of UI TRANSFORM:{} to TopLeft:{} BottomRight:{} (newSize:{}) but one or more locks "
				"do not permit size changes in their direction until the FIXED size for that direction is set to false. "
				"Fixed Vertical:{} Fixed Horizontal:{}", ToString(), topLeftPos.ToString(), bottomRightPos.ToString(),
				newSize.ToString(), IsFixedVertical(), IsFixedHorizontal()));
			return;
		}

		const Vec2 oldSize = m_localRect.GetSize().AsVec2();
		m_localRect.SetBoundsTLBR(topLeftPos, bottomRightPos);
		UpdateFixedChildren(oldSize);
	}

	void UITransformComponent::CenterWithinParent(const bool centerX, const bool centerY)
	{
		if (!centerX && !centerY)
			return;

		const Vec2 size = GetLocalSize().AsVec2();
		const Vec2 emptySpaceSize = Vec2(NormalizedValue::MAX, NormalizedValue::MAX) - size;
		const NormalizedVec2 currBottomLeft = GetLocalRect().GetBottomLeftPos();

		SetLocalBottomLeftPos(currBottomLeft + NormalizedVec2{ centerX ? emptySpaceSize.m_X / 2 : 0.0f,
															  centerY ? emptySpaceSize.m_Y / 2 : 0.0f });
	}

	NormalizedVec2 UITransformComponent::GetLocalSize() const { return m_localRect.GetSize(); }
	const UIRect& UITransformComponent::GetLocalRect() const { return m_localRect; }

	UIRect UITransformComponent::CalculateWorldRect(const UIRect& parentGlobalRect) const
	{
		return UI::CalculateWorldRect(parentGlobalRect, m_localRect);
	}
	UIRect UITransformComponent::CalculateChildParentRect(const UIRect& thisGlobalRect) const
	{
		if (m_Padding.HasNoPadding())
			return thisGlobalRect;

		const NormalizedVec2 topLeftPadding = NormalizedVec2(m_Padding.m_Left, m_Padding.m_Top);
		return UIRect(m_localRect.GetTopLeftPos() - topLeftPadding,
			m_localRect.GetSize() - topLeftPadding - NormalizedVec2(m_Padding.m_Right, m_Padding.m_Bottom));
	}

	Mat3 UITransformComponent::CalculateLocalModelMatrix() const
	{
		return Math::CalculateUIModelMatrix(nullptr, m_localRect.GetTopLeftPos().AsVec2(),
			m_localRect.GetSize().AsVec2(), m_localRect.m_Pivot.AsVec2());
	}
	Mat3 UITransformComponent::CalculateWorldModelMatrix() const
	{
		Mat3 globalMatrix = CalculateLocalModelMatrix();

		const ECS::EntityData* parent = GetEntity().GetParent();
		if (parent != nullptr)
		{
			//NOTE: it is a requirement for all ui elements to be an ui context ONLY so parent should have a transform
			globalMatrix = globalMatrix * parent->TryGetComponent<UITransformComponent>()->CalculateLocalModelMatrix();
			parent = parent->GetParent();
		}
		return globalMatrix;
	}

	void UITransformComponent::InitFields()
	{
		m_Fields = {};
	}
	void UITransformComponent::Serialize(Serialization::Serializer& serializer) const
	{
	}
	void UITransformComponent::Deserialize(Serialization::Deserializer& deserializer)
	{
	}
	std::string UITransformComponent::ToString() const
	{
		return std::format("[Id:{} BL:{} TR:{} Size:{} LAST WA:{}]", GetEntity().ToStringId(),
			m_localRect.GetBottomLeftPos().ToString(), m_localRect.GetBottomRightPos().ToString(),
			GetLocalSize().ToString(), m_lastGlobalScreenRect.ToString());
	}
}
