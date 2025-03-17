#pragma once
#include <string>

struct SerializableEntity
{
	std::string m_EntityName;
	std::string m_SceneName;

	SerializableEntity();
	SerializableEntity(const std::string& sceneName, const std::string& entityName);
};

