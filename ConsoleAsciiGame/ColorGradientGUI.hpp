#pragma once
#include "SelectableGUI.hpp"
#include "IRenderable.hpp"
#include "ColorGradient.hpp"

class ColorGradientGUI : public SelectableGUI
{
private:
	ColorGradient m_gradient;
public:

private:
public:
	ColorGradientGUI(const ColorGradient& gradient);

	void SetGradient(const ColorGradient& gradient);
	const ColorGradient& GetGradient() const;

	void Update(const float deltaTime) override;
	RenderInfo ElementRender(const RenderInfo& renderInfo) override;
};

