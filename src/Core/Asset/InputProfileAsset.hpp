#pragma once
#include "Core/Asset/Asset.hpp"
#include <optional>
#include "Core/Input/InputProfile.hpp"

namespace Engine::Input
{
	class InputProfileAsset : public Assets::Asset
	{
	private:
		Input::InputManager* m_inputManager;
		std::optional<Input::InputProfile> m_profile;
	public:
		static const std::array<std::string_view,1> EXTENSIONS;

	private:
		Input::InputManager& GetInputManager();
	public:
		InputProfileAsset(const std::filesystem::path& path);

		Input::InputProfile& GetProfileMutable();
		const Input::InputProfile& GetProfile() const;

		void SetDependencies(Core::EngineState& state) override;

		void UpdateAssetFromFile() override;
		void SaveToPath(const std::filesystem::path& path) override;
	};
}


