#pragma once
#include "UIRendererComponent.hpp"

namespace Rendering { class Renderer; }
class Scene;

namespace ECS
{
	class UIRenderSystem
	{
	private:
		Rendering::Renderer* m_renderer;
	public:

	private:
	public:
		UIRenderSystem(Rendering::Renderer& renderer);

		void Start(Scene& scene);
		GUIRect RenderUIEntity(UIRendererData& renderer, const GUIRect& rect);
	};
}


