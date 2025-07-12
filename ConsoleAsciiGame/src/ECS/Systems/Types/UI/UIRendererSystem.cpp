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
#include "Core/EngineState.hpp"

namespace ECS
{
	UIRenderSystem::UIRenderSystem(const EngineState& engineState, Rendering::Renderer& renderer, UIHierarchy& hierarchy)
		: m_engineState(&engineState), m_renderer(&renderer), m_uiHierarchy(&hierarchy), m_OnElementProcessed() {}
		//, m_uiRenderersHierarchyOrder(), m_hasGuiTreeUpdated(true) {}

	void UIRenderSystem::Init()
	{
		GlobalComponentInfo::AddComponentInfo(typeid(UIRendererData),
			ComponentInfo([this](EntityData& entity)-> void
				{
					entity.TryGetComponentMutable<UIRendererData>()->m_renderer = m_renderer;
				}));

		/*m_uiHierarchy->m_OnElementAdded.AddListener([this](const UITransformData* element)-> void
			{
				if (m_engineState->GetExecutionState() != ExecutionState::Update) return;
				m_hasGuiTreeUpdated = true;
			});

		m_uiHierarchy->m_OnElementRemoved.AddListener([this](const UITransformData* element)-> void
			{
				if (m_engineState->GetExecutionState() != ExecutionState::Update) return;
				m_hasGuiTreeUpdated = true;
			});
		*/
	}
	/*
	void UIRenderSystem::CreateRenderTree()
	{
		//m_uiRenderersHierarchyOrder.clear();
		m_uiHierarchy->ElementTraversalDFS<UIRendererData>([this](UILayer layer, UIRendererData& renderer)-> void
			{
				m_uiRenderersHierarchyOrder.push_back(&renderer);
			});
	}
	*/
	UIRect UIRenderSystem::RenderSingle(const UIHierarchy& hierarchy, UIRendererData& renderer, const UIRect& rect)
	{
		UIRect renderedArea = {};
		try
		{
			EntityData& entity = renderer.GetEntityMutable();
			if (UIPanel* panel = entity.TryGetComponentMutable<UIPanel>(false))
			{
				renderedArea = panel->Render(rect);
				//if (renderer.GetEntity().m_Name == "EntityHeader") LogError(std::format("Rendered panel at pos:{}", rect.ToString()));
			}
			if (UITextureData* texture = entity.TryGetComponentMutable<UITextureData>(false))
			{
				renderedArea = texture->Render(rect);
				/*if (entity.GetParent() != nullptr && entity.GetParent()->m_Name == "EntityActiveToggle")
					LogError(std::format("Found child with parent toggle:{} has renderer", entity.m_Name));*/
			}
			if (UITextComponent* text = entity.TryGetComponentMutable<UITextComponent>(false))
			{
				renderedArea = text->Render(rect);
				//if (entity.m_Name == "EntityNameText") LogError(std::format("text reder rect:{}", rect.ToString(), renderedArea.ToString()));
			}

			if (UISelectableData* selectable = entity.TryGetComponentMutable<UISelectableData>(false))
			{
				selectable->RenderOverlay(renderedArea);
			}
		}
		catch (const std::exception& e)
		{
			LogError(std::format("Attempted to render single entity:{} but encountered error:{}. Full Tree:{}", 
				renderer.GetEntity().ToString(), e.what(), hierarchy.ToStringTree()));
		}
		return renderedArea;
	}

	void UIRenderSystem::RenderAll()
	{
		//if (m_hasGuiTreeUpdated)
		//{
		//	CreateRenderTree();
		//	m_hasGuiTreeUpdated = false;
		//}

		std::stack<UIRect, std::vector<UIRect>> rectsStack = {};
		UIRect parentRect = {};
		UIRect currentRect = {};

		std::stack<UITransformData*, std::vector<UITransformData*>> elementStack = {};
		UITransformData* currentTransform = nullptr;
		EntityData* entity = nullptr;
		UIRendererData* renderer = nullptr;

		/*for (const auto& r : m_uiRenderersHierarchyOrder)
		{
			LogWarning(r->GetEntity().ToString());
		}*/

		m_uiHierarchy->LayerTraversal([&, this](UILayer layer, UITransformData& rootTransform)-> void
			{
				while (!rectsStack.empty()) rectsStack.pop();
				rectsStack.push(m_uiHierarchy->GetRootRect());

				while (!elementStack.empty()) elementStack.pop();
				elementStack.push(&rootTransform);

				while (!elementStack.empty())
				{
					currentTransform = elementStack.top();
					elementStack.pop();
					entity = &(currentTransform->GetEntityMutable());

					parentRect = rectsStack.top();
					rectsStack.pop();
					currentRect = currentTransform->CalculateRect(parentRect);

					Vec2 rectSize = currentRect.GetSize();
					if (rectSize.m_X == 0 || rectSize.m_Y == 0)
					{
						LogError(std::format("Attempted to calculate rect for ui object:{} "
							"but its render rect has a 0 component:{}. Parent rect:{} ui transform:{}. UI TREE:{}", entity->ToString(),
							rectSize.ToString(), parentRect.ToString(), currentTransform->ToString(), m_uiHierarchy->ToStringTree()));
						return;
					}
					/*if (!Assert(currentRect.GetSize().m_X != 0 && currentRect.GetSize().m_Y != 0,
						std::format("Attempted to render all ui elements, but element:{}({}) calculated from parent rect:{} had rect with 0 size component:{}",
							transform.ToString(), transform.GetEntity().ToString(), parentRect->ToString(), currentRect.ToString())))
						return;*/

						//Note: render single returns the actual area that is rendered
					UIRendererData* renderer = entity->TryGetComponentMutable<UIRendererData>(false);
					if (entity->IsEntityActive() && renderer != nullptr)
					{
						//if (entity->m_Name== "DebugInfoContainer") LogError(std::format("Enttiy is active and rendered"));
						renderer->m_lastRenderArea = RenderSingle(*m_uiHierarchy, *renderer, currentRect);
						m_OnElementProcessed.Invoke(renderer, &(renderer->m_lastRenderArea));
					}
					currentTransform->SetLastWorldArea(currentRect);

					if (entity->GetChildCount() == 0) continue;
					//Note: we go backwards to ensure proper DFS order (aka first/top elements first, then bottom)
					for (int i = entity->GetChildCount() - 1; i >= 0; i--)
					{
						UIRect childRect = currentTransform->CalculateChildRect(currentRect);
						Vec2 rectSize = childRect.GetSize();
						if (rectSize.m_X == 0 || rectSize.m_Y == 0)
						{
							LogError(std::format("Attempted to calculate child rect of ui object:{} "
								"but its render rect has a 0 component:{}. Parent rect:{} calcualted child rect:{} parent ui transform:{}. UI TREE:{}",
								entity->ToString(), rectSize.ToString(), currentRect.ToString(), childRect.ToString(),
								currentTransform->ToString(), m_uiHierarchy->ToStringTree()));
							return;
						}

						currentTransform = entity->TryGetChildComponentAtMutable<UITransformData>(i);
						if (currentTransform == nullptr)
						{
							LogError(std::format("Tried to get child ui transform at index:{} for entity:{} "
								"but it was null. Child:{}", i, entity->ToString(), entity->TryGetChildEntityAtMutable(i)->ToString()));
							return;
						}
						rectsStack.push(childRect);
						elementStack.push(currentTransform);
					}
				}
			});
		//LogWarning(std::format("AFTER CALCULATING RENDER AAREAS:{}", m_uiHierarchy->ToStringTree()));
	}

	void UIRenderSystem::SystemUpdate()
	{
		RenderAll();
	}
}

