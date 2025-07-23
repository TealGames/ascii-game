#pragma once
#include "ECS/Component/Component.hpp"
#include "Utils/Data/Color.hpp"
//#include "raylib.h"
#include "Core/UI/UIRect.hpp"

class UIRendererData;
namespace ECS { class UIPanelSystem; }
class UIPanel : public Component
{
private:
	Utils::Color m_color;
	UIRendererData* m_renderer;
public:
	friend class ECS::UIPanelSystem;

private:
public:
	UIPanel();
	UIPanel(const Utils::Color color);

	void SetColor(const Utils::Color color);

	UIRect Render(const UIRect& rect);

	void InitFields() override;
	std::string ToString() const override;

	void Deserialize(const Json& json) override;
	Json Serialize() override;
};

