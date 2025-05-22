#pragma once
#include <vector>
#include "IRenderable.hpp"
#include <cstdint>
#include "RelativeGUIRect.hpp"
#include "Event.hpp"
#include "HelperMacros.hpp"
#include "RelativeGUIPadding.hpp"
#include "GUIRect.hpp"

using GUIElementID = std::uint16_t;

enum class GUIElementFlags : std::uint8_t
{
	/// <summary>
	///Means the total normalized dimensions relative to the canvas will stay the same, but the size will NOT update with the 
	/// parent. Note: this means it WILL size down/up based on canvas size changes since the dimensions are still as percents, but
	/// if the parent size changes, it will maintain the original dimensions by updating to the corresponding values of the parent size
	/// </summary>
	FixedHorizontal =		1 << 0,
	FixedVertical	=		1 << 1,
	/// <summary>
	/// Although it is not a selectable, nonselectables CAN block events from propagating further
	/// and can prevent lower level selectables from receiving their events. This is most useful
	/// for overlays or other types of popups and dialogs that may block lower elements completely
	/// </summary>
	BlockSelectionEvents=	1 << 2,
};
FLAG_ENUM_OPERATORS(GUIElementFlags)

/// <summary>
/// A base class for gui elements whether basic types like input fields, text, buttons or containers.
/// Note: there is 2 ways the UI system can be used:
/// 
///	1) Set each component of a larger ui system (like health, inventory, etc) as gui element and attach each one to the hierarchy
///	   in the preferred order and using any containers so each one get rendered separately. This is better for formatting it based
///	   on specific calculations and measurements using already made containers and components
/// 
/// 2) Set the parent system as the gui element and only attach the parent system to the hierarchy and any other gui elements should NOT
///	   be added to the hierarchy and instead allow the render function of the system class to call each of the render functions for the gui elements.
///	   This is best when you do not need specific placements and/or you need placements that cannot be done with existing components, allowing
///	   for a more customized look to the UI. NOTE: IN THIS CASE ALL SELECTALBES NEED TO BE REGISTERED IF THEY ARE NOT ADDED TO THE UI TREE
/// </summary>
class GUIElement : public IRenderable
{
private:
	GUIElementID m_id;
	RelativeGUIRect m_relativeRect;
	std::vector<GUIElement*> m_children;
	GUIElement* m_parent;
	GUIElementFlags m_flags;

	GUIRect m_lastRenderInfo;

	/// <summary>
	/// This is how much the CHILDREN AREA is padded within this element. 
	/// Values are relative to THIS ELEMENT"S SIZE
	/// </summary>
	RelativeGUIPadding m_padding;

public:
	//Event<void, GUIElement*> m_OnSizeUpdated;
	Event<void, GUIElement*> m_OnFarthestChildElementAttached;

private:
	static GUIElementID GenerateId();
	/// <summary>
	/// A less safe version of size settings that has less checks and assumes new size is valid
	/// It is most often used to get around checks/for performance
	/// </summary>
	/// <param name="vec"></param>
	void SetSizeUnsafe(const Vec2& size);

	RenderInfo CalculateChildRenderInfo(const RenderInfo& thisRenderInfo) const;

public:
	GUIElement();
	GUIElement(const RelativeGUIRect& relativeRect);
	GUIElement(const NormalizedPosition& size);
	~GUIElement() = default;

	GUIElementID GetId() const;
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
	const RelativeGUIPadding& GetPadding();
	RelativeGUIPadding& GetPaddingMutable();

	NormalizedPosition GetSize() const;
	const RelativeGUIRect& GetRect() const;
	RelativeGUIRect& GetRectMutable();

	GUIElement* GetParentMutable();
	const GUIElement* GetParent() const;

	/// <summary>
	/// Adds the element as a child and returns its child index
	/// </summary>
	/// <param name="element"></param>
	/// <returns></returns>
	size_t PushChild(GUIElement* element);
	GUIElement* TryPopChildAt(const size_t index);
	std::vector<GUIElement*> TryPopChildren(const size_t& startIndex, const size_t& count);
	std::vector<GUIElement*> PopAllChildren();

	std::vector<GUIElement*>& GetChildrenMutable();
	const std::vector<GUIElement*>& GetChildren() const;
	GUIElement* TryGetChildAtMutable(const size_t index);
	GUIElement* TryGetChildMutable(const GUIElementID id);
	/// <summary>
	/// Will attempt to find the element from ID starting with THIS id and searching
	/// recursively through all of the children
	/// </summary>
	/// <param name="id"></param>
	/// <returns></returns>
	GUIElement* FindRecursiveMutable(const GUIElementID id);
	/// <summary>
	/// Will attempt to find the element that contains a child with the id, beginning with this
	/// parent element. Child index is an out argument that will contain the index of the child of the parent (if found)
	/// </summary>
	/// <param name="id"></param>
	/// <param name="childIndex"></param>
	/// <returns></returns>
	GUIElement* FindParentRecursiveMutable(const GUIElementID id, size_t* childIndex = nullptr);

	size_t GetChildCount() const;

	virtual void Update(const float deltaTime) = 0;
	void UpdateRecursive(const float deltaTime);

	RenderInfo CalculateRenderInfo(const RenderInfo& parentInfo) const;
	const GUIRect& GetLastFrameRect() const;
	/// <summary>
	/// Will render the current element and return how much space/what top left pos was ACTUALLY used
	/// </summary>
	/// <param name="renderInfo"></param>
	/// <returns></returns>
	virtual RenderInfo Render(const RenderInfo& parentInfo) = 0;
	void RenderRecursive(const RenderInfo& renderInfo);

	std::string ToStringBase() const;
	std::string ToStringRecursive(std::string startNewLine) const;

	/// <summary>
	/// Will calculate the total space/top left pos used for rendering for the given child at index
	/// Base implementation returns the parent area
	/// </summary>
	/// <param name="parentInfo"></param>
	/// <returns></returns>
	//RenderInfo CalculateChildRenderInfo(const RenderInfo& parentInfo, const size_t childIndex) const;
};

