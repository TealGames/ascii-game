#include "pch.hpp"
#include "UIRendererSystem.hpp"
#include "Scene.hpp"
#include "EntityData.hpp"

#include "UIPanel.hpp"
#include "UITextureData.hpp"
#include "UISelectableData.hpp"
#include "UITextComponent.hpp"

namespace ECS
{
	UIRenderSystem::UIRenderSystem(Rendering::Renderer& renderer)
		: m_renderer(&renderer) {}

	void UIRenderSystem::Start(Scene& scene)
	{
		scene.OperateOnComponents<UIRendererData>(
			[this, &scene](UIRendererData& renderer)-> void
			{
				renderer.SetRenderer(*m_renderer);
			});
	}

	GUIRect UIRenderSystem::RenderUIEntity(UIRendererData& renderer, const GUIRect& rect)
	{
		EntityData& entity = renderer.GetEntitySafeMutable();
		GUIRect renderedArea = {};
		if (UIPanel* panel = entity.TryGetComponentMutable< UIPanel>())
		{
			renderedArea= panel->Render(rect);
		}
		if (UITextureData* texture = entity.TryGetComponentMutable<UITextureData>())
		{
			renderedArea= texture->Render(rect);
		}
		if (UITextComponent* text = entity.TryGetComponentMutable<UITextComponent>())
		{
			renderedArea = text->Render(rect);
		}

		if (UISelectableData* selectable = entity.TryGetComponentMutable<UISelectableData>())
		{
			selectable->RenderOverlay(renderedArea);
		}

		return renderedArea;
	}
}

