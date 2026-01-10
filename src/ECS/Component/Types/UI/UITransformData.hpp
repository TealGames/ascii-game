#pragma once
#include "ECS/Component/Component.hpp"
#include "Utils/HelperMacros.hpp"
#include "Core/UI/UIRect.hpp"
#include "Core/UI/UIPadding.hpp"
#include "Utils/Data/Matrix.hpp"

enum class UITransformFlags : std::uint8_t
{
	/// <summary>
	///Means the total normalized dimensions relative to the canvas will stay the same, but the size will NOT update with the 
	/// parent. Note: this means it WILL size down/up based on canvas size changes since the dimensions are still as percents, but
	/// if the parent size changes, it will maintain the original dimensions by updating to the corresponding values of the parent size
	/// </summary>
	None = 0,
	FixedHorizontal = 1 << 0,
	FixedVertical = 1 << 1,
	EventBlocker =1 << 2,
	All= 0xFF
};
FLAG_ENUM_OPERATORS(UITransformFlags)

class UITransformData : public Component
{
private:
	UIRect m_localRect;
	UITransformFlags m_flags;

	UIRect m_lastGlobalScreenRect;
public:
	/// <summary>
	/// This is how much the CHILDREN AREA is padded within this element. 
	/// Values are relative to THIS ELEMENT'S SIZE
	/// </summary>
	UIPadding m_Padding;

private:
	bool DoLocksAllowSizeChange(const NormalizedPos& proposedNewSize) const;

	void UpdateFixedChildren(const Vec2& oldSize);
	/// <summary>
	/// A less safe version of size settings that has less checks and assumes new size is valid
	/// It is most often used to get around checks/for performance
	/// </summary>
	/// <param name="vec"></param>
	void SetSizeUnsafe(const Vec2& size);
public:
	UITransformData();
	UITransformData(const UIRect& relativeRect);
	UITransformData(const NormalizedPos& size);
	~UITransformData() = default;

	void SetFixed(const bool horizontal, const bool vertical);
	bool IsFixedVertical() const;
	bool IsFixedHorizontal() const;

	void SetLastGlobalScreenRect(const UIRect& area);
	const UIRect& GetLastGlobalScreenRect() const;

	/// <summary>
	/// Although it is not a selectable, nonselectables CAN block events from propagating further
	/// and can prevent lower level selectables from receiving their events. HOWEVER, since event blockers use rendered areas
	/// for the blocked area (NOT transform rect) it must have a rendered component attached that renders in order for event blocking to work.
	/// This is most useful for overlays or other types of popups and dialogs that may block lower elements completely
	/// </summary>
	void SetEventBlocker(const bool status);
	bool IsSelectionEventBlocker() const;

	void SetLocalSize(const NormalizedPos& size);
	void SetMaxRelativeSize();
	void SetRelativeSizeX(const float sizeNormalized);
	void SetRelativeSizeY(const float sizeNormalized);

	void SetLocalTopLeftPos(const NormalizedPos& topLeftPos);
	void SetLocalTopRightPos(const NormalizedPos& topRightPos);
	void SetLocalBottomRightPos(const NormalizedPos& bottomRightPos);
	void SetLocalBottomLeftPos(const NormalizedPos& bottomleftPos);
	void SetLocalBoundsBLTR(const NormalizedPos& bottomLeftPos, const NormalizedPos& topRightPos);
	void SetLocalBoundsTLBR(const NormalizedPos& topLeftPos, const NormalizedPos& bottomRightPos);
	/// <summary>
	/// Will center this rect's corresponding axes within the parent rect
	/// by changing anchor positions (SIZE DOES NOT CHANGE)
	/// </summary>
	/// <param name="centerX"></param>
	/// <param name="centerY"></param>
	void CenterWithinParent(const bool centerX, const bool centerY);

	NormalizedPos GetLocalSize() const;
	const UIRect& GetLocalRect() const;

	UIRect CalculateWorldRect(const UIRect& parentGlobalRect) const;
	/// <summary>
	/// Calculates the available rect for the child to use as its parent
	/// </summary>
	/// <param name="thisGlobalRect"></param>
	/// <returns></returns>
	UIRect CalculateChildParentRect(const UIRect& thisGlobalRect) const;

	Mat3 CalculateLocalModelMatrix() const;
	Mat3 CalculateWorldModelMatrix() const;

	void InitFields() override;
	std::string ToString() const override;
	void Deserialize(const Json& json) override;
	Json Serialize() override;
};

