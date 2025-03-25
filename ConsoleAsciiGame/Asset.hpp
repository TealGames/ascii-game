#pragma once
#include <string>
#include <filesystem>

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
	Asset(const std::filesystem::path& path);
	~Asset() = default;

	std::string GetName() const;
	std::filesystem::path GetPath() const;
	bool AreDependenciesSet() const;

	virtual std::string ToString() const;
};

