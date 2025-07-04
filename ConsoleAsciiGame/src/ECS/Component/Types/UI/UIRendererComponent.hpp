#pragma once
#include "ECS/Component/Component.hpp"
#include "Core/UI/UIRect.hpp"

namespace Rendering { class Renderer; }
namespace ECS { class UIRenderSystem; }
class UIRendererData : public Component
{
private:
	Rendering::Renderer* m_renderer;
	UIRect m_lastRenderArea;
public:
	friend class ECS::UIRenderSystem;

private:
public:
	UIRendererData();

	const UIRect& GetLastRenderRect() const;
	Rendering::Renderer& GetRendererMutable();

	void InitFields() override;
	std::string ToString() const override;

	void Deserialize(const Json& json) override;
	Json Serialize() override;
};

