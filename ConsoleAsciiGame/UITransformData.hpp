#pragma once
#include "ComponentData.hpp"
#include "HelperMacros.hpp"
#include "RelativeGUIRect.hpp"
#include "RelativeGUIPadding.hpp"
#include "GUIRect.hpp"

enum class UITransformFlags : std::uint8_t
{
	/// <summary>
	///Means the total normalized dimensions relative to the canvas will stay the same, but the size will NOT update with the 
	/// parent. Note: this means it WILL size down/up based on canvas size changes since the dimensions are still as percents, but
	/// if the parent size changes, it will maintain the original dimensions by updating to the corresponding values of the parent size
	/// </summary>
	FixedHorizontal = 1 << 0,
	FixedVertical = 1 << 1,
	/// <summary>
	/// Although it is not a selectable, nonselectables CAN block events from propagating further
	/// and can prevent lower level selectables from receiving their events. This is most useful
	/// for overlays or other types of popups and dialogs that may block lower elements completely
	/// </summary>
	BlockSelectionEvents = 1 << 2,
};
FLAG_ENUM_OPERATORS(UITransformFlags)

class UITransformData : public Component
{
private:
	RelativeGUIRect m_relativeRect;
	UITransformFlags m_flags;

	/// <summary>
	/// This is how much the CHILDREN AREA is padded within this element. 
	/// Values are relative to THIS ELEMENT"S SIZE
	/// </summary>
	RelativeGUIPadding m_padding;
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
	UITransformData(const RelativeGUIRect& relativeRect);
	UITransformData(const NormalizedPosition& size);
	~UITransformData() = default;

	void SetFixed(const bool horizontal, const bool vertical);
	bool IsFixedVertical() const;
	bool IsFixedHorizontal() const;

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

	void SetPadding(const RelativeGUIPadding& padding);
	const RelativeGUIPadding& GetPadding() const;
	RelativeGUIPadding& GetPaddingMutable();

	NormalizedPosition GetSize() const;
	const RelativeGUIRect& GetRect() const;
	RelativeGUIRect& GetRectMutable();

	void InitFields() override;

	std::string ToString() const override;

	void Deserialize(const Json& json) override;
	Json Serialize() override;
};

