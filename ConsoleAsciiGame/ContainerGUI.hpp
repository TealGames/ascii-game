#pragma once
#include "GUIElement.hpp"

class ContainerGUI : public GUIElement
{
private:
public:

private:
public:
	ContainerGUI();

	void Update(const float deltaTime) override;
	RenderInfo Render(const RenderInfo& renderInfo) override;
};