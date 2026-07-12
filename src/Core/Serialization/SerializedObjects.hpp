#pragma once
#include <string>
#include <filesystem>

namespace Engine::Serialization
{
	class SerializedEntity
	{
	private:
	public:
		std::string m_SceneName;
		std::string m_EntityName;

	private:
	public:
		SerializedEntity();
		SerializedEntity(const std::string& sceneName, const std::string& entityName);

		std::string ToString() const;
	};

	class SerializedComponent
	{
	private:
	public:
		SerializedEntity m_SerializedEntity;
		std::string m_ComponentName;

	private:
	public:
		SerializedComponent();
		SerializedComponent(const SerializedEntity& entity, const std::string& compName);
		SerializedComponent(const std::string& sceneName, const std::string& entityName,
			const std::string& componentName);

		std::string ToString() const;
	};

	class SerializedField
	{
	private:
	public:
		SerializedComponent m_SerializedComponent;
		std::string m_FieldName;

	private:
	public:
		SerializedField();
		SerializedField(const SerializedComponent& serializedComp, const std::string& fieldName);
		SerializedField(const std::string& sceneName, const std::string& entityName,
			const std::string& componentName, const std::string& fieldName);
	};


	class SerializedAsset
	{
	private:
	public:
		std::filesystem::path m_AssetPath;

	private:
	public:
		SerializedAsset();
		SerializedAsset(const std::filesystem::path& assetPath);

		std::string ToString() const;
	};
}