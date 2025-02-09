#include "pch.hpp"
#include "UIObjectSystem.hpp"
#include "CameraData.hpp"
#include "Debug.hpp"
#include "PositionConversions.hpp"

namespace ECS
{
	UIObjectSystem::UIObjectSystem() {}

	void UIObjectSystem::SystemUpdate(Scene& scene, const float& deltaTime)
	{
		CameraData* mainCamera = scene.TryGetMainCameraData();
		if (!Assert(this, mainCamera != nullptr, 
			std::format("Tried to run system update for UI but main camera is NULL")))
			return;

		scene.OperateOnComponents<UIObjectData>(
			[this, &scene, &deltaTime, &mainCamera](UIObjectData& data, ECS::Entity& entity)-> void
			{
				WorldPosition worldPos = Conversions::ScreenToWorldPosition(*mainCamera, Conversions::NormalizedScreenToPosition(data.GetNormalizedPos()));
				entity.m_Transform.SetPos(worldPos);
			});
	}
}

