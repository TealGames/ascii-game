#pragma once
#include <filesystem>

namespace IO
{
	std::filesystem::path CleanPath(const std::filesystem::path& path);
	bool DoesPathExist(const std::filesystem::path& path);

	bool CreatePathIfNotExist(const std::filesystem::path& path, const bool forceCleanPath=false);

	bool TryWriteFile(const std::filesystem::path& path, const std::string& data);
	std::string TryReadFile(const std::filesystem::path& path);
}


