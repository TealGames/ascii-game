#pragma once
#include "ECS/Component/Types/UI/UIInputFieldComponent.hpp"

class GlobalEntityManager;
namespace ECS
{
	class UIInputFieldSystem
	{
	private:
	public:

	private:
	public:
		UIInputFieldSystem();
		void Init();
		void SystemUpdate(GlobalEntityManager& globalEntityManager, const float& deltaTime);
	};
}


