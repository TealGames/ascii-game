#pragma once
#include <vector>
#include <string>
#include <unordered_map>

class EntityID
{
private:
	static int m_lastId;
	static std::vector<int> m_takenIds;
	static std::unordered_map<std::string, int> m_idAliases;

	int m_id;

public:
	const int& m_Id;

private:
public:
	EntityID(const std::string& alias= "");

	static bool DoesAliasExist(const std::string& alias);
	static bool TryGetAliasId(const std::string& alias);
};

