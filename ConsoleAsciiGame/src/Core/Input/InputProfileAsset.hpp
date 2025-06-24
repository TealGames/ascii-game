#pragma once
#include "Core/Asset/Asset.hpp"
#include <optional>
#include "Core/Input/InputProfile.hpp"
#include "Core/Asset/IDependableAsset.hpp"
#include "Core/Serialization/IJsonSerializable.hpp"

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

