#include <limits>
#include "EntityID.hpp"
#include "HelperFunctions.hpp"

std::vector<int> EntityID::m_takenIds = {};
std::unordered_map<std::string, int> EntityID::m_idAliases;
int EntityID::m_lastId = -1;

EntityID::EntityID(const std::string& alias) : m_id(++m_lastId), m_Id(m_id)
{
	if (!Utils::Assert(m_id != std::numeric_limits<int>::max(), 
		"Created the max possible entity id!")) 
		return;

	m_takenIds.push_back(m_id);
	if (!alias.empty())
	{
		if (!Utils::Assert(!DoesAliasExist(alias),
			std::format("Tried creating an entity id with alias: {} that already exists", alias)))
			return;

		m_idAliases.emplace(alias, m_id);
	}
}

bool EntityID::DoesAliasExist(const std::string& alias)
{
	return m_idAliases.find(alias) != m_idAliases.end();
}

bool EntityID::TryGetAliasId(const std::string& alias)
{
	if (!DoesAliasExist(alias)) return false;
	return m_idAliases[alias];
}
