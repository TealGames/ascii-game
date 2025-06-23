#pragma once
#include "Component.hpp"
#include "Color.hpp"
#include "raylib.h"
#include "UIRect.hpp"

class UIRendererData;
namespace ECS { class UIPanelSystem; }
class UIPanel : public Component
{
private:
	Color m_color;
	UIRendererData* m_renderer;
public:
	friend class ECS::UIPanelSystem;

private:
public:
	UIPanel();
	UIPanel(const Color color);

	void SetColor(const Color color);

	UIRect Render(const UIRect& rect);

	void InitFields() override;
	std::string ToString() const override;

	void Deserialize(const Json& json) override;
	Json Serialize() override;
};

