#pragma once
#include <string>
#include <string_view>
#include <filesystem>
#include "Core/ID.hpp"

namespace Engine::Core { class EngineState; }
namespace Engine::Assets
{
#define ASSET_EXTENSION_CHECK \
ENGINE_ASSERT(::Utils::IO::DoesPathHaveExtension(path, EXTENSIONS), \
	"Tried to create an asset from path:{} (extension:{})" \
	"but it does not have required extensions:'{}'", path.string(), path.extension().string(), \
	::Utils::ToStringArray(EXTENSIONS)); \

	class Asset
	{
	private:
		std::string m_name;
		std::filesystem::path m_absolutePath;

	public:
		static constexpr char WORD_SEPARATOR = '_';

	private:
	protected:
		/// <summary>
		/// Overrides the default assetname generated from the asset file name to a custom one.
		/// Note: this should rarely be used as it can mess with looking up asset by name.
		/// </summary>
		/// <param name="name"></param>
		void OverrideAssetName(const std::string& name);
		void OverrideAssetName(const std::string_view& name);

	public:
		Asset(const std::filesystem::path& path);
		~Asset() = default;

		static std::string ExtractNameFromFile(const std::filesystem::path& path);

		const std::string& GetName() const;

		std::filesystem::path GetAbsolutePathCopy() const;
		const std::filesystem::path& GetAbsolutePath() const;
		bool AbsolutePathEndsWith(const std::filesystem::path& subPath);

		virtual void SetDependencies(Core::EngineState& state);

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
}