#pragma once
#include <filesystem>
#include <fstream>
#include <vector>
#include <functional>
#include <optional>

namespace Utils::IO
{
	std::filesystem::path CleanPath(const std::filesystem::path& path);
	bool DoesPathExist(const std::filesystem::path& path);
	bool DoesPathHaveExtension(const std::filesystem::path& path, const std::filesystem::path& extension);
	bool DoesPathHaveExtension(const std::filesystem::path& path, const std::string_view[], const size_t extensionCount);

	template<size_t N>
	bool DoesPathHaveExtension(const std::filesystem::path& path, const std::array<std::string_view, N>& extensions)
	{
		const std::filesystem::path pathExtension = path.extension();
		for (const auto& extension : extensions)
		{
			if (pathExtension == extension)
				return true;
		}
		return false;
	}

	bool DoesDirectoryContainDirectory(const std::filesystem::path& parent, const std::filesystem::path& child);
	std::optional<std::filesystem::path> GetFirstDirectory(const std::filesystem::path& path);
	std::optional<std::filesystem::path> GetRelativePath(const std::filesystem::path& parentPath, const std::filesystem::path& childPath);
	std::filesystem::path JoinPaths(const std::filesystem::path& path0, const std::filesystem::path& path1);

	bool CreatePathIfNotFound(const std::filesystem::path& path, const bool forceCleanPath=false);
	bool IsFileEmpty(const std::filesystem::path& path);

	bool TryWriteFile(const std::filesystem::path& path, const std::string& data);
	bool TryCreateWriteFileBinaryStream(const std::filesystem::path& path, std::ofstream& outStream);

	std::string TryReadFileFull(const std::filesystem::path& path);
	std::vector<std::string> TryReadFileByLine(const std::filesystem::path& path);
	bool CreateReadFileBinaryStream(const std::filesystem::path& path, std::ifstream& outStream);

	using FileLineAction = std::function<void(const std::string*)>;
	bool TryExecuteOnFileByLine(const std::filesystem::path& path, const FileLineAction& action);
}


