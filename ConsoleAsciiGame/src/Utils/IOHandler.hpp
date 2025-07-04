#pragma once
#include <filesystem>
#include <vector>
#include <functional>
#include <optional>

namespace IO
{
	std::filesystem::path CleanPath(const std::filesystem::path& path);
	bool DoesPathExist(const std::filesystem::path& path);
	bool DoesPathHaveExtension(const std::filesystem::path& path, const std::filesystem::path& extension);
	bool DoesPathHaveExtension(const std::filesystem::path& path, const std::string& extension);

	bool DoesDirectoryContainDirectory(const std::filesystem::path& parent, const std::filesystem::path& child);
	std::optional<std::filesystem::path> GetFirstDirectory(const std::filesystem::path& path);
	std::optional<std::filesystem::path> GetRelativePath(const std::filesystem::path& parentPath, const std::filesystem::path& childPath);

	bool CreatePathIfNotExist(const std::filesystem::path& path, const bool forceCleanPath=false);

	bool TryWriteFile(const std::filesystem::path& path, const std::string& data);
	std::string TryReadFileFull(const std::filesystem::path& path);
	std::vector<std::string> TryReadFileByLine(const std::filesystem::path& path);

	using FileLineAction = std::function<void(const std::string*)>;
	bool TryExecuteOnFileByLine(const std::filesystem::path& path, const FileLineAction& action);
}


