#pragma once
#include "Core/Serialization/SerializableEntity.hpp"
#include <cstdint>

struct SerializableComponent
{
	SerializableEntity m_SerializedEntity;
	std::string m_ComponentName;

	static const char* SELF_COMPONENT_ENTITY_KEYWORD;

	SerializableComponent();
	SerializableComponent(const SerializableEntity& entity, const std::string& compName);
	SerializableComponent(const std::string& sceneName, const std::string& entityName,
		const std::string& componentName);

	/// <summary>
	/// Reserved for self components
	/// </summary>
	/// <param name="compName"></param>
	SerializableComponent(const std::string& compName);

	std::string ToString() const;

	/// <summary>
	/// If true, the component stored is from the entity itself
	/// </summary>
	/// <returns></returns>
	bool IsComponentOfEntitySelf() const;
};


