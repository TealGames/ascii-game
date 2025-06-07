#pragma once
#include "GUIRect.hpp"
#include "UITransformData.hpp"

namespace ECS
{
	class UITransformSystem
	{
	private:
	public:

	private:
	public:
		UITransformSystem();

		GUIRect CalculateRect(const UITransformData& transform, const GUIRect& parentInfo) const;
		GUIRect CalculateChildRect(const UITransformData& transform, const GUIRect& thisRenderInfo) const;
	};
}


