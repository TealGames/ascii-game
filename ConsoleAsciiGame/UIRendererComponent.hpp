#pragma once
#include "Component.hpp"
#include "UIRect.hpp"

namespace Rendering { class Renderer; }
namespace ECS { class UIRenderSystem; }
class UIRendererData : public Component
{
private:
	Rendering::Renderer* m_renderer;
	UIRect m_lastRenderRect;
	bool m_isEventBlocker;
public:
	friend class ECS::UIRenderSystem;

private:
public:
	UIRendererData();

	Rendering::Renderer& GetRendererMutable();
	const UIRect& GetLastRenderRect() const;

	/// <summary>
	/// Although it is not a selectable, nonselectables CAN block events from propagating further
	/// and can prevent lower level selectables from receiving their events. HOWEVER, since event blockers use rendered areas
	/// for the blocked area (NOT transform rect) it must have a rendered component attached that renders in order for event blocking to work.
	/// This is most useful for overlays or other types of popups and dialogs that may block lower elements completely
	/// </summary>
	void SetEventBlocker(const bool status);
	bool IsSelectionEventBlocker() const;

	void InitFields() override;
	std::string ToString() const override;

	void Deserialize(const Json& json) override;
	Json Serialize() override;
};

