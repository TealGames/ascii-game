#include "pch.hpp"
#include "IOHandler.hpp"
#include <fstream>
#include "Debug.hpp"

namespace IO
{
	std::filesystem::path CleanPath(const std::filesystem::path& path)
	{
		Utils::StringUtil cleaner(path.string());
		const std::string cleaned = cleaner.TrimSpaces().ToString();
		return cleaned;
	}

	bool DoesPathExist(const std::filesystem::path& path)
	{
		if (!std::filesystem::exists(path))
		{
			std::string err = std::format("File path {} does not exist", path.string());
			std::cout << err << std::endl;
			return false;
		}
		return true;
	}

	bool DoesPathHaveExtension(const std::filesystem::path& path, const std::filesystem::path& extension)
	{
		//if (!std::filesystem::exists(path)) return false;
		LogError(std::format("Comaping extension:{} and {}", path.extension().string(), extension.extension().string()));
		return path.extension() == extension.extension();
	}
	bool DoesPathHaveExtension(const std::filesystem::path& path, const std::string& extension)
	{
		return path.extension().string() == extension;
	}

	bool CreatePathIfNotExist(const std::filesystem::path& path, const bool forceCleanPath)
	{
		if (!DoesPathExist(path))
		{
			std::filesystem::create_directories(forceCleanPath? CleanPath(path) : path);
			return true;
		}
		return false;
	}

	bool TryWriteFile(const std::filesystem::path& path, const std::string& content)
	{
		const std::filesystem::path cleanedPath = CleanPath(path);
		CreatePathIfNotExist(path);

		std::ofstream file(cleanedPath);
		if (!Assert(file.is_open(), std::format("Tried to WRITE {} to file at path {} "
			"but it could not be opened", content, cleanedPath.string())))
			return false;

		file << content;
		file.close();
		return true;
	}

	std::string TryReadFileFull(const std::filesystem::path& path)
	{
		const std::filesystem::path cleanedPath = CleanPath(path);

		std::ifstream file(cleanedPath);
		if (!Assert(file.is_open(), std::format("Tried to READ from file (full) at path {} "
			"but it could not be opened", cleanedPath.string())))
			return "";

		std::stringstream stream;
		stream << file.rdbuf();
		file.close();
		return stream.str();
	}

	std::vector<std::string> TryReadFileByLine(const std::filesystem::path& path)
	{
		const std::filesystem::path cleanedPath = CleanPath(path);

		std::ifstream file(cleanedPath);
		if (!Assert(file.is_open(), std::format("Tried to READ from file (by lines) at path {} "
			"but it could not be opened", cleanedPath.string())))
			return {};

		std::vector<std::string> lines = {};
		std::string line = "";
		while (std::getline(file, line))
		{
			lines.push_back(line);
		}
		return lines;
	}

	bool TryExecuteOnFileByLine(const std::filesystem::path& path, const FileLineAction& action)
	{
		const std::filesystem::path cleanedPath = CleanPath(path);

		std::ifstream file(cleanedPath);
		if (!Assert(file.is_open(), std::format("Tried to execute an action for file (by lines) at path {} "
			"but it could not be opened", cleanedPath.string())))
			return false;

		std::vector<std::string> lines = {};
		std::string line = "";
		while (std::getline(file, line))
		{
			if (action) action(&line);
		}
		return true;
	}
}


