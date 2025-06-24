#pragma once
#include "ECS/Component/Types/UI/UISelectableData.hpp"
#include "Utils/Data/ColorGradient.hpp"

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

