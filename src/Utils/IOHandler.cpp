#include "pch.hpp"
#include "Utils/IOHandler.hpp"
#include "Utils/Debug.hpp"
#include "StringUtil.hpp"

namespace Utils::IO
{
	std::filesystem::path CleanPath(const std::filesystem::path& path)
	{
		Utils::StringUtil cleaner(path.string());
		const std::string cleaned = cleaner.TrimSpaces().ToString();
		return cleaned;
	}

	bool DoesPathExist(const std::filesystem::path& path)
	{
		return std::filesystem::exists(path);
	}

	bool DoesPathHaveExtension(const std::filesystem::path& path, const std::filesystem::path& extension)
	{
		//if (!std::filesystem::exists(path)) return false;
		//LogError(std::format("Comaping extension:{} and {}", path.extension().string(), extension.extension().string()));
		return path.extension() == extension.extension();
	}
	bool DoesPathHaveExtension(const std::filesystem::path& path, const std::string_view extensions[], const size_t extensionCount)
	{
		const std::filesystem::path pathExtension = path.extension();
		for (int i=0; i< extensionCount; i++)
		{
			if (pathExtension == extensions[i])
				return true;
		}
		return false;
	}

	bool DoesDirectoryContainDirectory(const std::filesystem::path& parent, const std::filesystem::path& child)
	{
		auto canon_parent = std::filesystem::weakly_canonical(parent);
		auto canon_child = std::filesystem::weakly_canonical(child);

		auto mismatch_pair = std::mismatch(canon_parent.begin(), canon_parent.end(), canon_child.begin());
		return mismatch_pair.first == canon_parent.end();
	}

	std::optional<std::filesystem::path> GetFirstDirectory(const std::filesystem::path& path)
	{
		for (const auto& part : path) 
		{
			if (part != "/" && part != "\\" && part != path.root_name() && part != path.root_directory()) 
			{
				return part;
			}
		}
		return std::nullopt;
	}

	std::optional<std::filesystem::path> GetRelativePath(const std::filesystem::path& parentPath, const std::filesystem::path& childPath)
	{
		std::filesystem::path canonicalInput;
		std::filesystem::path canonicalParent;

		try 
		{
			canonicalInput = std::filesystem::weakly_canonical(childPath);
			canonicalParent = std::filesystem::weakly_canonical(parentPath);
		}
		// Handle broken symlinks or missing paths gracefully
		catch (...) 
		{
			return std::nullopt;  
		}

		// Check if the input is inside the asset path
		auto mismatch = std::mismatch(canonicalParent.begin(), canonicalParent.end(), canonicalInput.begin());
		if (mismatch.first == canonicalParent.end()) 
		{
			//If child path is inside parent path, make it relative to parent
			return std::filesystem::relative(canonicalInput, canonicalParent).string();
		}

		// Try walking up childpath to find a prefix that is inside parent path
		std::filesystem::path currentPath = canonicalInput;
		while (currentPath.has_parent_path())
		{
			currentPath = currentPath.parent_path();
			auto mismatch = std::mismatch(canonicalParent.begin(), canonicalParent.end(), currentPath.begin());
			if (mismatch.first == canonicalParent.end()) 
			{
				return std::filesystem::relative(canonicalInput, canonicalParent);
			}
		}

		return std::nullopt;
	}
	std::filesystem::path JoinPaths(const std::filesystem::path& path0, const std::filesystem::path& path1)
	{
		return path0 / path1;
	}

	bool CreatePathIfNotFound(const std::filesystem::path& path, const bool forceCleanPath)
	{
		if (!DoesPathExist(path))
		{
			const std::filesystem::path& cleanedPath = forceCleanPath ? CleanPath(path) : path;

			std::filesystem::create_directories(path.parent_path());
			std::ofstream(path).close();
			return true;
		}
		return false;
	}

	bool TryWriteFile(const std::filesystem::path& path, const std::string& content)
	{
		const std::filesystem::path cleanedPath = CleanPath(path);

		//NOTE: ofstream automatically creates path if it does not exist
		std::ofstream file(cleanedPath);
		if (!Assert(file.is_open(), "Tried to WRITE {} to file at path {} "
			"but it could not be opened", content, cleanedPath.string()))
			return false;

		file << content;
		file.close();
		return true;
	}
	bool TryCreateWriteFileBinaryStream(const std::filesystem::path& path, std::ofstream& outStream)
	{
		const std::filesystem::path cleanedPath = CleanPath(path);
		outStream = std::ofstream(cleanedPath, std::ios::binary);
		return outStream.is_open();
	}

	std::string TryReadFileFull(const std::filesystem::path& path)
	{
		const std::filesystem::path cleanedPath = CleanPath(path);

		std::ifstream file(cleanedPath);
		if (!Assert(file.is_open(), "Tried to READ from file (full) at path {} "
			"but it could not be opened", cleanedPath.string()))
			return "";

		std::stringstream stream;
		stream << file.rdbuf();
		file.close();
		return stream.str();
	}
	bool IsFileEmpty(const std::filesystem::path& path)
	{
		const std::filesystem::path cleanedPath = CleanPath(path);
		std::ifstream file(cleanedPath);
		if (!Assert(file.is_open(), "Tried to check if file is empty at path {} "
			"but it could not be opened", cleanedPath.string()))
			return false;

		std::string line = "";
		std::getline(file, line);
		return line.empty();
	}

	std::vector<std::string> TryReadFileByLine(const std::filesystem::path& path)
	{
		const std::filesystem::path cleanedPath = CleanPath(path);

		std::ifstream file(cleanedPath);
		if (!Assert(file.is_open(), "Tried to READ from file (by lines) at path {} "
			"but it could not be opened", cleanedPath.string()))
			return {};

		std::vector<std::string> lines = {};
		std::string line = "";
		while (std::getline(file, line))
		{
			lines.push_back(line);
		}
		return lines;
	}
	bool CreateReadFileBinaryStream(const std::filesystem::path& path, std::ifstream& outStream)
	{
		const std::filesystem::path cleanedPath = CleanPath(path);
		outStream = std::ifstream(cleanedPath, std::ios::binary);
		return outStream.is_open();
	}

	bool TryExecuteOnFileByLine(const std::filesystem::path& path, const FileLineAction& action)
	{
		if (!action)
		{
			LogError(std::format("Attemtped to execute on file at path:{} by line with action that is NULL", path.string()));
			return false;
		}
		const std::filesystem::path cleanedPath = CleanPath(path);

		std::ifstream file(cleanedPath);
		if (!Assert(file.is_open(), "Tried to execute an action for file (by lines) at path {} "
			"but it could not be opened", cleanedPath.string()))
			return false;

		std::vector<std::string> lines = {};
		std::string line = "";
		while (std::getline(file, line))
		{
			action(&line);
		}
		return true;
	}
}


