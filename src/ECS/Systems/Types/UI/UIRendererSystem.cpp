#include "pch.hpp"
#include <stack>
#include "ECS/Systems/Types/UI/UIRendererSystem.hpp"
#include "Core/UI/UIHierarchy.hpp"
#include "Core/Scene/Scene.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "ECS/Component/Types/UI/UIRendererComponent.hpp"
#include "ECS/Component/Types/UI/UIPanelComponent.hpp"
#include "ECS/Component/Types/UI/UITextureComponent.hpp"
#include "ECS/Component/Types/UI/UISelectableComponent.hpp"
#include "ECS/Component/Types/UI/UITextComponent.hpp"
#include "ECS/Component/GlobalComponentInfo.hpp"
#include "Core/EngineState.hpp"

namespace Engine::UI
{
	static constexpr float DEPTH_INCREMENT = 0.001;
	static constexpr float TOPMOST_DEPTH = 0.0f;
	static constexpr float BOTTOMMOST_DEPTH = 1.0f;

	UIRenderSystem::UIRenderSystem(const Core::EngineState& engineState, Rendering::Renderer& renderer, UIHierarchy& hierarchy)
		: m_engineState(&engineState), m_renderer(&renderer), m_uiHierarchy(&hierarchy), m_OnElementProcessed() {}
		//, m_uiRenderersHierarchyOrder(), m_hasGuiTreeUpdated(true) {}

	void UIRenderSystem::Init()
	{
		ECS::GlobalComponentInfo::AddComponentInfo(typeid(UIRendererComponent),
			ECS::ComponentInfo([this](ECS::EntityData& entity)-> void
				{
					entity.TryGetComponentMutable<UIRendererComponent>()->m_renderer = m_renderer;
				}));
	}
	void UIRenderSystem::RenderSingle(const UIHierarchy& hierarchy, UIRendererComponent& renderer, const float depth, const Mat3& globalModelMatrix)
	{
		ECS::EntityData& entity = renderer.GetEntityMutable();
		if (UIPanelComponent* panel = entity.TryGetComponentMutable<UIPanelComponent>(false))
		{
			panel->Render(depth, globalModelMatrix);
			//if (renderer.GetEntity().m_Name == "EntityHeader") LogError(std::format("Rendered panel at pos:{}", rect.ToString()));
		}
		if (UITextureComponent* texture = entity.TryGetComponentMutable<UITextureComponent>(false))
		{
			//texture->Render(rect);
			/*if (entity.GetParent() != nullptr && entity.GetParent()->m_Name == "EntityActiveToggle")
				LogError(std::format("Found child with parent toggle:{} has renderer", entity.m_Name));*/
		}
		if (UITextComponent* text = entity.TryGetComponentMutable<UITextComponent>(false))
		{
			//text->Render(rect);
			//if (entity.m_Name == "EntityNameText") LogError(std::format("text reder rect:{}", rect.ToString(), renderedArea.ToString()));
		}

		if (UISelectableComponent* selectable = entity.TryGetComponentMutable<UISelectableComponent>(false))
		{
			//selectable->RenderOverlay();
		}
	}

	void UIRenderSystem::RenderAll()
	{
		std::stack<UIRect, std::vector<UIRect>> rectsStack = {};
		UIRect parentRect = {};
		UIRect currentRect = {};

		std::stack<UITransformComponent*, std::vector<UITransformComponent*>> elementStack = {};
		UITransformComponent* currentTransform = nullptr;
		ECS::EntityData* entity = nullptr;
		UIRendererComponent* renderer = nullptr;

		//TODO: depth should probably be scaled depending on how mnay objects we have to ensure they all fit
		float depth = TOPMOST_DEPTH;

		m_uiHierarchy->LayerTraversal([&, this](UILayer layer, UITransformComponent& rootTransform)-> void
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
					currentRect = currentTransform->CalculateWorldRect(parentRect);

					UIRendererComponent* renderer = entity->TryGetComponentMutable<UIRendererComponent>(false);
					if (entity->IsEntityActive() && renderer != nullptr)
					{
						//if (entity->m_Name== "DebugInfoContainer") LogError(std::format("Enttiy is active and rendered"));
						/*LogWarning(std::format("UI panel world: {} depth:{} modelmat:{}", 
							currentRect.ToString(), depth, currentRect.CalculateModelMatrix().ToString()));*/
						RenderSingle(*m_uiHierarchy, *renderer, depth, currentRect.CalculateModelMatrix());
						m_OnElementProcessed.Invoke(renderer, &(renderer->m_lastRenderArea));

						depth += DEPTH_INCREMENT;
					}
					//currentTransform->SetLastGlobalScreenRect(currentRect);

					if (entity->GetChildCount() == 0) continue;
					//Note: we go backwards to ensure proper DFS order (aka first/top elements first, then bottom)
					for (int i = entity->GetChildCount() - 1; i >= 0; i--)
					{
						UIRect availableChildRect = currentTransform->CalculateChildParentRect(currentRect);
						currentTransform = entity->TryGetChildComponentAtMutable<UITransformComponent>(i);
						if (currentTransform == nullptr)
						{
							LogError(std::format("Tried to get child ui transform at index:{} for entity:{} "
								"but it was null. Child:{}", i, entity->ToString(), entity->TryGetChildEntityAtMutable(i)->ToString()));
							return;
						}
						rectsStack.push(availableChildRect);
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

