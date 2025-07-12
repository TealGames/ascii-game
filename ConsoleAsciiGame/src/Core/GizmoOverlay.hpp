#pragma once
#include <vector>
#include "Core/Rendering/RenderCall.hpp"

namespace Rendering { class Renderer; }
namespace ECS { class UIRenderSystem; }
class GizmoOverlay
{
private:
	std::vector<Rendering::RenderCall> m_queuedCalls;
public:

private:
public:
	GizmoOverlay(ECS::UIRenderSystem& renderSystem);

	void AddCollisionBoxGizmo(const ScreenPosition& pos, const Vec2& size);
	void AddRenderRectGizmo(const ScreenPosition& pos, const Vec2& size);

	void MoveCallsToRenderBuffer(Rendering::Renderer& renderer);
};

