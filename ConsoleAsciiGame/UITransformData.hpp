#pragma once
#include "Component.hpp"
#include "HelperMacros.hpp"
#include "RelativeUIRect.hpp"
#include "RelativeUIPadding.hpp"
#include "UIRect.hpp"

enum class UITransformFlags : std::uint8_t
{
	/// <summary>
	///Means the total normalized dimensions relative to the canvas will stay the same, but the size will NOT update with the 
	/// parent. Note: this means it WILL size down/up based on canvas size changes since the dimensions are still as percents, but
	/// if the parent size changes, it will maintain the original dimensions by updating to the corresponding values of the parent size
	/// </summary>
	FixedHorizontal = 1 << 0,
	FixedVertical = 1 << 1
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

