#pragma once
#include <string>
#include "Entity.hpp"
#include "Scene.hpp"

struct EntityReference
{
	ECS::Entity& m_Entity;
	Scene* m_Scene;

	EntityReference(ECS::Entity& entity, Scene* scene);

	bool IsGlobal() const;
	std::string GetSceneName() const;

	bool MatchesScene(const ComponentData* component);
};

