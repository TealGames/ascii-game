#pragma once
#include "UISelectableData.hpp"
#include "IRenderable.hpp"
#include "ColorGradient.hpp"

class UIPanel;
class UIColorGradient : public UISelectableData
{
private:
	ColorGradient m_gradient;
public:

private:
public:
	UIColorGradient(const ColorGradient& gradient);

	void SetGradient(const ColorGradient& gradient);
	const ColorGradient& GetGradient() const;

	void Update(const float deltaTime) override;
};

