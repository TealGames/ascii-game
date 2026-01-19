#pragma once
#include "ECS/Component/Component.hpp"
#include "Utils/Data/Color.hpp"
#include "Utils/Math/Matrix.hpp"
//#include "raylib.h"
#include "Core/UI/UIRect.hpp"

class UIRendererData;
namespace ECS { class UIPanelSystem; }
class UIPanelComponent : public Component
{
private:
	HDRColor m_color;
	UIRendererData* m_renderer;
public:
	friend class ECS::UIPanelSystem;

private:
public:
	UIPanelComponent();
	UIPanelComponent(const HDRColor color);

	void SetColor(const HDRColor color);

	void Render(const float depth, const Mat3& globalModelMatrix);

	void InitFields() override;
	std::string ToString() const override;

	void Deserialize(const Json& json) override;
	Json Serialize() override;
};

