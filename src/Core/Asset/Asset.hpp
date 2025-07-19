#pragma once
#include <string>
#include <filesystem>
#include "Core/Serialization/IJsonSerializable.hpp"

class Asset
{
private:
	std::string m_name;
	std::filesystem::path m_path;
	bool m_dependenciesSet;

public:

private:
protected:
	void MarkDependenciesSet();

public:
	Asset(const std::filesystem::path& path, const bool hasDependencies);
	~Asset() = default;

	static std::string ExtractNameFromFile(const std::filesystem::path& path);

	std::string GetName() const;

	std::filesystem::path GetPathCopy() const;
	const std::filesystem::path& GetPath() const;
	bool AreDependenciesSet() const;

	/// <summary>
	/// Will update the asset contents to match the data found in the asset's corresponding file
	/// </summary>
	virtual void UpdateAssetFromFile() = 0;

	/// <summary>
	/// Will write the data found in the asset to the corresponding file at the path
	/// Note: by default this has no implementation because not all assets may be able
	/// to be modified from the asset since some may just be wrappers
	/// </summary>
	/// <param name="path"></param>
	virtual void SaveToPath(const std::filesystem::path& path);
	void SaveToSelf();

	virtual std::string ToString() const;
};

template<typename T>
concept IsAssetType = std::is_base_of_v<Asset, T>;

