#include "pch.hpp"
#include "ComponentData.hpp"
#include "HelperFunctions.hpp"

ComponentData::ComponentData() : m_MutatedThisFrame(false), m_Entity(nullptr) {}

ECS::Entity& ComponentData::GetEntitySafeMutable()
{
	if (!Utils::Assert(this, m_Entity != nullptr, std::format("Tried to retrieve entity from component safely but it is NULLPTR "
		"(it means a function creating or adding component probably did not update this setting)")))
		throw std::invalid_argument("Tried to retrieve invalid entity with component");

	return *m_Entity;
}

const ECS::Entity& ComponentData::GetEntitySafe() const
{
	if (!Utils::Assert(this, m_Entity != nullptr, std::format("Tried to retrieve entity from component safely but it is NULLPTR "
		"(it means a function creating or adding component probably did not update this setting)")))
		throw std::invalid_argument("Tried to retrieve invalid entity with component");

	return *m_Entity;
}