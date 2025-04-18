#include "pch.hpp"
#include "EntityReference.hpp"

EntityReference::EntityReference(ECS::Entity& entity, Scene* scene)
	: m_Entity(entity), m_Scene(scene) {}

bool EntityReference::IsGlobal() const
{
	return m_Scene == nullptr;
}

std::string EntityReference::GetSceneName() const
{
	if (IsGlobal()) return ECS::Entity::GLOBAL_SCENE_NAME;
	return m_Scene->GetName();
}

bool EntityReference::MatchesScene(const ComponentData* component)
{
	if (component == nullptr) return false;
	return m_Scene->GetName() == component->GetEntitySafe().GetSceneName();
}