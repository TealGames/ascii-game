#pragma once
#include "ComponentData.hpp"
#include "Color.hpp"
#include "raylib.h"
#include "GUIRect.hpp"

class UIPanel : public Component
{
private:
	Color m_color;
public:

private:
public:
	UIPanel();
	UIPanel(const Color color);

	void SetColor(const Color color);

	GUIRect Render(const GUIRect& rect);

	void InitFields() override;
	std::string ToString() const override;

	void Deserialize(const Json& json) override;
	Json Serialize() override;
};

