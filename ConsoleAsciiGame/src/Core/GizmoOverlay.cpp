#include "pch.hpp"
#include "Core/GizmoOverlay.hpp"
#include "Core/Rendering/GameRenderer.hpp"
#include "ECS/Systems/Types/UI/UIRendererSystem.hpp"

static constexpr float LINE_THICKNESS = 1;
static const Color COLLIDER_OUTLINE_COLOR = GREEN;
static const Color UI_RECT_OUTLINE_COLOR = YELLOW;

static constexpr bool SHOW_UI_RECT_OUTLINE = false;

GizmoOverlay::GizmoOverlay(ECS::UIRenderSystem& renderSystem) : m_queuedCalls()
{
	if (!SHOW_UI_RECT_OUTLINE) return;
	renderSystem.m_OnElementProcessed.AddListener([this](const UIRendererData* renderer, UIRect* rect)-> void
		{
			//LogError(std::format("Adding gizmos renderer"));
			AddRenderRectGizmo(rect->m_TopLeftPos, rect->GetSize());
		});
}

void GizmoOverlay::AddCollisionBoxGizmo(const ScreenPosition& pos, const Vec2& size)
{
	m_queuedCalls.emplace_back(Rendering::RectLineCall{ pos, LINE_THICKNESS, size, COLLIDER_OUTLINE_COLOR });
}
void GizmoOverlay::AddRenderRectGizmo(const ScreenPosition& pos, const Vec2& size)
{
	m_queuedCalls.emplace_back(Rendering::RectLineCall{ pos, LINE_THICKNESS, size, UI_RECT_OUTLINE_COLOR });
}

void GizmoOverlay::MoveCallsToRenderBuffer(Rendering::Renderer& renderer)
{
	if (m_queuedCalls.empty()) return;

	renderer.MoveCallsToBuffer(m_queuedCalls);
	m_queuedCalls.clear();
}