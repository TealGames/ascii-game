#pragma once
//#include "GUIElement.hpp"

class EntityData;
class UITransformData;
struct PopupUI //: public GUIElement
{
	UITransformData* m_Container;

	PopupUI();

	void CreatePopup(EntityData& parent);
	virtual void AddPopupElements() = 0;
	virtual ~PopupUI() = default;

	/*RenderInfo Render(const RenderInfo& parentInfo) override;
	virtual void InsideRender(const RenderInfo& parentInfo) = 0;

	virtual void Update(const float deltaTime) = 0;*/
};

