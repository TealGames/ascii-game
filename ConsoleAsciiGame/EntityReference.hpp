#pragma once
#include <string>
#include "EntityData.hpp"
#include "Scene.hpp"

struct EntityReference
{
	EntityData& m_Entity;
	Scene* m_Scene;

	EntityReference(EntityData& entity, Scene* scene);

	bool IsGlobal() const;
	std::string GetSceneName() const;

	bool MatchesScene(const ComponentData* component);
};

