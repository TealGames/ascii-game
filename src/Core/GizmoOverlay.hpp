#pragma once
#include <vector>
#include <cstdint>
#include "Utils/HelperMacros.hpp"
#include "Core/Rendering/RenderCall.hpp"

namespace Rendering { class Renderer; }
namespace ECS { class UIRenderSystem; }
namespace Physics { class PhysicsManager; }
class CameraController;

enum class GizmoType : std::uint8_t
{
	None					= 0,
	CollisionBoxOutline		= 1,
	RenderRectOutline		= 1<<1,
	All						= 0xFF
};
FLAG_ENUM_OPERATORS(GizmoType)

class GizmoOverlay
{
private:
	GizmoType m_activeGizmos;
	std::vector<Rendering::RenderCall> m_queuedCalls;
public:

private:
public:
	GizmoOverlay(ECS::UIRenderSystem& renderSystem, Physics::PhysicsManager& physicsManager, const CameraController& cameraController);

	void AddCollisionBoxGizmo(const WorldPosition2D& pos, const Vec2& size);
	void AddRenderRectGizmo(const WorldPosition2D& pos, const Vec2& size);

	void MoveCallsToRenderBuffer(Rendering::Renderer& renderer);

	void EnableAllGizmos();
	void DisableAllGizmos();
};

