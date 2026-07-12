#pragma once
#include <string>
#include "ECS/Component/Types/World/EntityData.hpp"
#include "Core/Scene/Scene.hpp"

namespace Engine::ECS
{
	using Scene = Engine::Scenes::Scene;
	struct EntityReference
	{
		EntityData& m_Entity;
		Scene* m_Scene;

		EntityReference(EntityData& entity, Scene* scene);

		bool IsGlobal() const;
		std::string GetSceneName() const;

		bool MatchesScene(const Component* component);
	};
}


