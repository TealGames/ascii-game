#pragma once
#include "ECS/Component/Types/UI/UIRendererComponent.hpp"
#include "Utils/Data/Event.hpp"

namespace Rendering { class Renderer; }
class Scene;
class UIHierarchy;
class EngineState;

namespace ECS
{
	class UIRenderSystem
	{
	private:
		UIHierarchy* m_uiHierarchy;
		Rendering::Renderer* m_renderer;
		const EngineState* m_engineState;
		//std::vector<UIRendererData*> m_uiRenderersHierarchyOrder;
		//bool m_hasGuiTreeUpdated;
	public:
		Event<void, const UIRendererData*, UIRect*> m_OnElementProcessed;

	private:

		//void CreateRenderTree();
		UIRect RenderSingle(const UIHierarchy& hierarchy, UIRendererData& renderer, const UIRect& rect);
		void RenderAll();
	public:
		UIRenderSystem(const EngineState& engineState, Rendering::Renderer& renderer, UIHierarchy& hierarchy);
		void Init();
		void SystemUpdate();
	};
}


