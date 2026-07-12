#pragma once
#include "Core/Asset/Asset.hpp"
#include "Core/Rendering/Model3d.hpp"

namespace Engine::Core { class EngineState; }
namespace Engine::Rendering
{
	class Model3dAsset : public Assets::Asset
	{
	private:
		Rendering::Model3d m_model;
		Core::EngineState* m_engineState;
	public:
		static const std::array<std::string_view, 2> EXTENSIONS;
	private:
	public:
		Model3dAsset(const std::filesystem::path& path);
		~Model3dAsset() = default;

		const Rendering::Model3d& GetModel() const;
		Rendering::Model3d& GetModelMutable();
		void UpdateAssetFromFile() override;

		/// <summary>
		/// Will write all vertices in the model to a VTX file
		/// format to the same path as the current asset.
		/// This is useful for reducing size of 3d model formats
		/// to a smaller custom-file type
		/// </summary>
		/// <returns></returns>
		void WriteModelAsCompressedFormat() const;
		void ReadModelAsCompressedFormat();

		void SetDependencies(Core::EngineState& state) override;
	};
}
