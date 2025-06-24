#include "pch.hpp"
#include "Core/Serialization/EntityReference.hpp"

EntityReference::EntityReference(EntityData& entity, Scene* scene)
	: m_Entity(entity), m_Scene(scene) {}

bool EntityReference::IsGlobal() const
{
	return m_Scene == nullptr;
}

std::string EntityReference::GetSceneName() const
{
	if (IsGlobal()) return EntityData::GLOBAL_SCENE_NAME;
	return m_Scene->GetName();
}

bool EntityReference::MatchesScene(const Component* component)
{
	if (component == nullptr) return false;
	return m_Scene->GetName() == component->GetEntity().m_SceneName;
}