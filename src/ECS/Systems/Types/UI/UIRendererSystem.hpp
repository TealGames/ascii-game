#pragma once
#include "Core/Primitives/Matrix.hpp"
#include "Utils/Data/Event.hpp"

namespace Engine::Rendering { class Renderer; }
namespace Engine::Core { class EngineState; }
namespace Engine::UI
{
	class UIHierarchy;
	class UIRendererComponent;
	class UIRect;
	class UIRenderSystem
	{
	private:
		UIHierarchy* m_uiHierarchy;
		Rendering::Renderer* m_renderer;
		const Core::EngineState* m_engineState;
		//std::vector<UIRendererComponent*> m_uiRenderersHierarchyOrder;
		//bool m_hasGuiTreeUpdated;
	public:
		Event<void, const UIRendererComponent*, UIRect*> m_OnElementProcessed;

	private:

		//void CreateRenderTree();
		void RenderSingle(const UIHierarchy& hierarchy, UIRendererComponent& renderer, const float depth, const Mat3& globalModelMatrix);
		void RenderAll();
	public:
		UIRenderSystem(const Core::EngineState& engineState, Rendering::Renderer& renderer, UIHierarchy& hierarchy);
		void Init();
		void SystemUpdate();
	};
}


