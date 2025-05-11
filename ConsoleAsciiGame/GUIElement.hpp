#pragma once
#include <vector>
#include "IRenderable.hpp"
#include <cstdint>
#include "RelativeGUIRect.hpp"

using GUIElementID = std::uint16_t;

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
public:
	

private:
	static GUIElementID GenerateId();
public:
	GUIElement();
	GUIElement(const RelativeGUIRect& relativeRect);
	GUIElement(const NormalizedPosition& size);
	~GUIElement() = default;

	GUIElementID GetId() const;
	
	void SetSize(const NormalizedPosition& size);
	void SetTopLeftPos(const NormalizedPosition& topLeftPos);
	void SetBottomRightPos(const NormalizedPosition& bottomRightPos);
	void SetBounds(const NormalizedPosition& topLeftPos, const NormalizedPosition& bottomRightPos);

	NormalizedPosition GetSize() const;
	const RelativeGUIRect& GetRect() const;
	RelativeGUIRect& GetRectMutable();

	void PushChild(GUIElement* element);
	GUIElement* TryPopChildAt(const size_t index);
	std::vector<GUIElement*> PopAllChildren();

	std::vector<GUIElement*>& GetChildrenMutable();
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

	/// <summary>
	/// Will render the current element and return how much space/what top left pos was ACTUALLY used
	/// </summary>
	/// <param name="renderInfo"></param>
	/// <returns></returns>
	virtual RenderInfo Render(const RenderInfo& renderInfo) = 0;
	void RenderRecursive(const RenderInfo& renderInfo);

	/// <summary>
	/// Will calculate the total space/top left pos used for rendering for the given child at index
	/// Base implementation returns the parent area
	/// </summary>
	/// <param name="parentInfo"></param>
	/// <returns></returns>
	//RenderInfo CalculateChildRenderInfo(const RenderInfo& parentInfo, const size_t childIndex) const;
};

