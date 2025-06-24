#include "pch.hpp"
#include <stack>
#include "ECS/Systems/Types/UI/UIRendererSystem.hpp"
#include "Core/UI/UIHierarchy.hpp"
#include "Core/Scene/Scene.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"

#include "ECS/Component/Types/UI/UIPanel.hpp"
#include "ECS/Component/Types/UI/UITextureData.hpp"
#include "ECS/Component/Types/UI/UISelectableData.hpp"
#include "ECS/Component/Types/UI/UITextComponent.hpp"
#include "ECS/Component/GlobalComponentInfo.hpp"

namespace ECS
{
	UIRenderSystem::UIRenderSystem(Rendering::Renderer& renderer, UIHierarchy& hierarchy)
		: m_renderer(&renderer), m_uiHierarchy(&hierarchy), m_uiRenderersHierarchyOrder()
	{
		GlobalComponentInfo::AddComponentInfo(typeid(UIRendererData), ComponentInfo(nullptr,
			[this](EntityData& entity)-> void
			{
				entity.TryGetComponentMutable<UIRendererData>()->m_renderer = m_renderer;
			}));
	}

	void UIRenderSystem::StoreRenderers()
	{
		m_uiRenderersHierarchyOrder.clear();
		m_uiHierarchy->ElementTraversalDFS<UIRendererData>([this](UILayer layer, UIRendererData& renderer)-> void
			{
				m_uiRenderersHierarchyOrder.push_back(&renderer);
			});
	}

	UIRect UIRenderSystem::RenderSingle(UIRendererData& renderer, const UIRect& rect)
	{
		EntityData& entity = renderer.GetEntityMutable();
		UIRect renderedArea = {};
		if (UIPanel* panel = entity.TryGetComponentMutable<UIPanel>())
		{
			renderedArea = panel->Render(rect);
		}
		if (UITextureData* texture = entity.TryGetComponentMutable<UITextureData>())
		{
			renderedArea = texture->Render(rect);
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

	void UIRenderSystem::RenderAll()
	{
		const UIRect* parentRect = {};
		UIRect currentRect = {};
		std::stack<UIRect> rects = {};
		EntityData* entity = nullptr;

		rects.push(m_uiHierarchy->GetRootRect());
		//Note: since we know we got the order in preorder traversal of the ui tree (aka dfs/descending), we can assume 
		//what the relationship is for the next element and make decision whether to push/pop new rect
		for (auto& renderer : m_uiRenderersHierarchyOrder)
		{
			entity = &(renderer->GetEntityMutable());
			UITransformData& transform = *(entity->TryGetComponentMutable<UITransformData>());
			//We get this render info based on this relative pos
			parentRect = &(rects.top());
			currentRect = transform.CalculateRect(*parentRect);
			//Note: render single returns the actual area that is rendered
			renderer->m_lastRenderRect = RenderSingle(*renderer, currentRect);

			/*if (DRAW_RENDER_BOUNDS) DrawRectangleLines(thisRenderInfo.m_TopLeftPos.m_X, thisRenderInfo.m_TopLeftPos.m_Y,
				thisRenderInfo.GetSize().m_X, thisRenderInfo.GetSize().m_Y, YELLOW);*/

			
			/*LogError(std::format("Rendering element calculated from parent info:{} rect:{} current info:{}",
				renderInfo.ToString(), m_relativeRect.ToString(), thisRenderInfo.ToString()));*/

			//If we have children, we need to go a level deeper so we calculate available space from this rect that can be used as the parent rect
			//Otherwise no children means we keep the same parent rect
			if (entity->GetChildCount() > 0)
				rects.push(transform.CalculateChildRect(currentRect));
			//If this entity's parent has this entity as the last child, it means we are ready to pop back to parent
			else if (entity->GetParent()->GetChildIndex(entity->GetId()) == size_t(-1))
				rects.pop();
			
			/*if (m_padding.HasNonZeroPadding())
			{
				Assert(false, std::format("Padding:{} this render:{} child render:{}",
					m_padding.ToString(), thisRenderInfo.ToString(), childRenderInfo.ToString()));
			}*/

			//TODO: this is really slow to have to check every element in the ui tree if it has the renderer. we 
			//should instead cache a different renderer tree for optimization
		}
	}

	void UIRenderSystem::SystemUpdate()
	{
		RenderAll();
	}
}

