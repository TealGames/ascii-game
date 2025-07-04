#pragma once
#include "ECS/Component/Component.hpp"
#include "Utils/HelperMacros.hpp"
#include "Core/UI/RelativeUIRect.hpp"
#include "Core/UI/RelativeUIPadding.hpp"
#include "Core/UI/UIRect.hpp"

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
	RelativeUIRect m_relativeRect;
	UITransformFlags m_flags;

	/// <summary>
	/// This is how much the CHILDREN AREA is padded within this element. 
	/// Values are relative to THIS ELEMENT"S SIZE
	/// </summary>
	RelativeUIPadding m_padding;
	UIRect m_lastWorldArea;
public:

private:
	/// <summary>
	/// A less safe version of size settings that has less checks and assumes new size is valid
	/// It is most often used to get around checks/for performance
	/// </summary>
	/// <param name="vec"></param>
	void SetSizeUnsafe(const Vec2& size);
public:
	UITransformData();
	UITransformData(const RelativeUIRect& relativeRect);
	UITransformData(const NormalizedPosition& size);
	~UITransformData() = default;

	void SetFixed(const bool horizontal, const bool vertical);
	bool IsFixedVertical() const;
	bool IsFixedHorizontal() const;

	void SetLastWorldArea(const UIRect& area);
	const UIRect& GetLastWorldArea() const;

	/// <summary>
	/// Although it is not a selectable, nonselectables CAN block events from propagating further
	/// and can prevent lower level selectables from receiving their events. HOWEVER, since event blockers use rendered areas
	/// for the blocked area (NOT transform rect) it must have a rendered component attached that renders in order for event blocking to work.
	/// This is most useful for overlays or other types of popups and dialogs that may block lower elements completely
	/// </summary>
	void SetEventBlocker(const bool status);
	bool IsSelectionEventBlocker() const;

	void SetSize(const NormalizedPosition& size);
	void SetMaxSize();
	void SetSizeX(const float sizeNormalized);
	void SetSizeY(const float sizeNormalized);

	void SetTopLeftPos(const NormalizedPosition& topLeftPos);
	void SetBottomRightPos(const NormalizedPosition& bottomRightPos);
	void SetBounds(const NormalizedPosition& topLeftPos, const NormalizedPosition& bottomRightPos);
	void TryCenter(const bool centerX, const bool centerY);

	void SetPadding(const RelativeUIPadding& padding);
	const RelativeUIPadding& GetPadding() const;
	RelativeUIPadding& GetPaddingMutable();

	NormalizedPosition GetSize() const;
	const RelativeUIRect& GetRect() const;
	RelativeUIRect& GetRectMutable();

	UIRect CalculateRect(const UIRect& parentInfo) const;
	UIRect CalculateChildRect(const UIRect& thisRenderInfo) const;

	void InitFields() override;

	std::string ToString() const override;

	void Deserialize(const Json& json) override;
	Json Serialize() override;
};

