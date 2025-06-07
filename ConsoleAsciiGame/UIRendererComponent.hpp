#pragma once
#include "ComponentData.hpp"
#include "GUIRect.hpp"

namespace Rendering { class Renderer; }
namespace ECS { class UIRenderSystem; }
class UIRendererData : public Component
{
private:
	Rendering::Renderer* m_renderer;
	GUIRect m_lastRenderRect;
public:

private:
public:
	UIRendererData();

	void SetRenderer(Rendering::Renderer& renderer);
	Rendering::Renderer& GetRendererMutable();

	void SetLastRenderRect(const GUIRect& rect);
	const GUIRect& GetLastRenderRect() const;

	void InitFields() override;
	std::string ToString() const override;

	void Deserialize(const Json& json) override;
	Json Serialize() override;
};

