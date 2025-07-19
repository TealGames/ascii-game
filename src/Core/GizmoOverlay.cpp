#include "pch.hpp"
#include "Core/GizmoOverlay.hpp"
#include "Core/Rendering/GameRenderer.hpp"
#include "ECS/Systems/Types/UI/UIRendererSystem.hpp"
#include "Core/Physics/PhysicsManager.hpp"
#include "ECS/Component/Types/World/TransformData.hpp"
#include "Core/Camera/CameraController.hpp"
#include "Utils/HelperFunctions.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"

static constexpr float LINE_THICKNESS = 1;
static const Color COLLIDER_OUTLINE_COLOR = GREEN;
static const Color UI_RECT_OUTLINE_COLOR = YELLOW;

GizmoOverlay::GizmoOverlay(ECS::UIRenderSystem& renderSystem, Physics::PhysicsManager& physicsManager, const CameraController& cameraController) 
	: m_queuedCalls(), m_activeGizmos(GizmoType::None)
{
	renderSystem.m_OnElementProcessed.AddListener([this](const UIRendererData* renderer, UIRect* rect)-> void
		{
			if (!Utils::HasFlagAll(m_activeGizmos, GizmoType::RenderRectOutline))
				return;

			//LogError(std::format("Adding gizmos renderer"));
			AddRenderRectGizmo(rect->m_TopLeftPos, rect->GetSize());
		});
	
	physicsManager.GetPhysicsWorldMutable().m_OnObjectProcessed.AddListener([this, &cameraController](PhysicsBodyData* body)-> void
		{
			if (!Utils::HasFlagAll(m_activeGizmos, GizmoType::CollisionBoxOutline))
				return;

			const ScreenPosition aabbTopLeftScreenPos = cameraController.GetActiveCamera().WorldToScreenPosition(body->GetCollisionBox().GetAABBTopLeftWorldPos());
			const Vec2 aabbScreenSize = cameraController.GetActiveCamera().WorldToScreenSize(body->GetCollisionBox().GetAABB().GetSize());
			/*if (body->GetEntity().m_Name == "player") 
				LogError(std::format("Player collider at screen:{} (top left world:{} center world:{}) Player world pos:{} size:{}", aabbTopLeftScreenPos.ToString(), 
					body->GetCollisionBox().GetAABBTopLeftWorldPos().ToString(), body->GetCollisionBox().GetAABBCenterWorldPos().ToString(), body->GetEntity().GetTransform().GetGlobalPos().ToString(),
					aabbScreenSize.ToString()));*/
			AddCollisionBoxGizmo(aabbTopLeftScreenPos, aabbScreenSize);
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

void GizmoOverlay::EnableAllGizmos()
{
	m_activeGizmos = GizmoType::All;
}
void GizmoOverlay::DisableAllGizmos()
{
	m_activeGizmos = GizmoType::None;
}