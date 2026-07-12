#include "Core/Serialization/SerializedObjects.hpp"
#include "Utils/Debug.hpp"

namespace Engine::Serialization
{
	SerializedEntity::SerializedEntity() : SerializedEntity("", "") {}
	SerializedEntity::SerializedEntity(const std::string& sceneName, const std::string& entityName)
		: m_SceneName(sceneName), m_EntityName(entityName) {}

	std::string SerializedEntity::ToString() const
	{
		return std::format("[SerializableEntity Scene:{} Entity:{}]", m_SceneName, m_EntityName);
	}

	SerializedComponent::SerializedComponent() : SerializedComponent("", "", "") {}
	SerializedComponent::SerializedComponent(const SerializedEntity& serializableEntity, const std::string& compName)
		: m_SerializedEntity(serializableEntity), m_ComponentName(compName)
	{
	}
	SerializedComponent::SerializedComponent(const std::string& sceneName, const std::string& entityName,
		const std::string& componentName) : m_SerializedEntity(sceneName, entityName), m_ComponentName(componentName)
	{
	}

	std::string SerializedComponent::ToString() const
	{
		return std::format("[SerializableComponent Scene:{} Entity:{} Comp:{}]",
			m_SerializedEntity.m_SceneName, m_SerializedEntity.m_EntityName, m_ComponentName);
	}

	SerializedField::SerializedField() : SerializedField("", "", "", "") {}
	SerializedField::SerializedField(const SerializedComponent& serializedComp, const std::string& fieldName)
		: m_SerializedComponent(serializedComp), m_FieldName(fieldName) {}

	SerializedField::SerializedField(const std::string& sceneName, const std::string& entityName,
		const std::string& componentName, const std::string& fieldName)
		: m_SerializedComponent(sceneName, entityName, componentName), m_FieldName(fieldName) {}


	SerializedAsset::SerializedAsset() : SerializedAsset("") {}
	SerializedAsset::SerializedAsset(const std::filesystem::path& assetPath) : m_AssetPath(assetPath) {}
	std::string SerializedAsset::ToString() const { return std::format("[SerializedAsset Path:{}]", m_AssetPath.string()); }
}