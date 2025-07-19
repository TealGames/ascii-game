#pragma once
#include <string>

struct SerializableEntity
{
	std::string m_SceneName;
	std::string m_EntityName;

	SerializableEntity();
	SerializableEntity(const std::string& sceneName, const std::string& entityName);

	std::string ToString() const;
};

