#pragma once
#include "SelectableGUI.hpp"
#include "IRenderable.hpp"
#include "ColorGradient.hpp"

class ColorGradientGUI : public SelectableGUI, IRenderable
{
private:
	ColorGradient m_gradient;
public:

private:
public:
	ColorGradientGUI(const ColorGradient& gradient);

	void SetGradient(const ColorGradient& gradient);
	const ColorGradient& GetGradient() const;

	void Update();
	ScreenPosition Render(const RenderInfo& renderInfo) override;
};

