#include "pch.hpp"
#include "TransformSystem.hpp"
#include "Point2DInt.hpp"
#include "TransformData.hpp"
#include "CartesianPosition.hpp"

namespace ECS
{
	void TransformSystem::SystemUpdate(Scene& scene, const float& deltaTime) {}

	void TransformSystem::UpdateLastFramePos(Scene& scene)
	{
		scene.OperateOnComponents<TransformData>(
			[](TransformData& transform, ECS::Entity& entity)->void
			{
				transform.m_LastFramePos = transform.m_Pos;
			});
	}

	//TODO: do scene bound checking
	void TransformSystem::SetPos(TransformData& data, const CartesianPosition& newPos)
	{
		data.m_LastPos = data.m_Pos;
		data.m_Pos = newPos;
	}

	//TODO: do scene bound checking
	void TransformSystem::SetPosX(TransformData& data, const int& newX)
	{
		SetPos(data, CartesianPosition(newX, data.m_Pos.m_Y));
	}

	//TODO: do scene bound checking
	void TransformSystem::SetPosY(TransformData& data, const int& newY)
	{
		SetPos(data, CartesianPosition(data.m_Pos.m_X, newY));
	}

	void TransformSystem::SetPosDeltaX(TransformData& data, const int& xDelta)
	{
		SetPosX(data, data.m_Pos.m_X + xDelta);
	}

	void TransformSystem::SetPosDeltaY(TransformData& data, const int yDelta)
	{
		SetPosY(data, data.m_Pos.m_Y + yDelta);
	}

	void TransformSystem::SetPosDelta(TransformData& data, const CartesianPosition& moveDelta)
	{
		SetPosDeltaX(data, moveDelta.m_X);
		SetPosDeltaY(data, moveDelta.m_Y);
	}

	bool TransformSystem::HasMovedThisFrame(TransformData& data) const
	{
		return data.m_LastFramePos==NULL_POS || data.m_Pos != data.m_LastFramePos;
	}
}

