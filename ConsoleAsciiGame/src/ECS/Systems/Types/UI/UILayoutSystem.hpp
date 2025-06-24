#pragma once
#include "ECS/Component/Types/UI/UILayout.hpp"

class GlobalEntityManager;
namespace ECS
{
	class UILayoutSystem
	{
	private:
	public:

	private:
	public:
		UILayoutSystem();

		void SystemUpdate(GlobalEntityManager& globalEntityManager, const float& deltaTime);
	};
}


