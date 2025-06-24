#pragma once
#include "Asset.hpp"
#include <optional>
#include "InputProfile.hpp"
#include "IDependableAsset.hpp"
#include "IJsonSerializable.hpp"

class InputProfileAsset : public Asset, public IDependableAsset<Input::InputManager>
{
private:
	Input::InputManager* m_inputManager;
	std::optional<Input::InputProfile> m_profile;
public:
	static const std::string EXTENSION;

private:
	Input::InputManager& GetInputManager();
public:
	InputProfileAsset(const std::filesystem::path& path);

	Input::InputProfile& GetProfileMutable();
	const Input::InputProfile& GetProfile() const;

	void SetDependencies(Input::InputManager& input) override;

	void UpdateAssetFromFile() override;
	void SaveToPath(const std::filesystem::path& path) override;
};

