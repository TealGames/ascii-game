#pragma once
#include "UIRendererComponent.hpp"

namespace Rendering { class Renderer; }
class Scene;
class UIHierarchy;

namespace ECS
{
	class UIRenderSystem
	{
	private:
		UIHierarchy* m_uiHierarchy;
		Rendering::Renderer* m_renderer;
		std::vector<UIRendererData*> m_uiRenderersHierarchyOrder;
	public:

	private:
		void StoreRenderers();
		UIRect RenderSingle(UIRendererData& renderer, const UIRect& rect);
		void RenderAll();
	public:
		UIRenderSystem(Rendering::Renderer& renderer, UIHierarchy& hierarchy);

		void SystemUpdate();
	};
}


