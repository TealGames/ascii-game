#pragma once
#include "Asset.hpp"
#include <optional>
#include "InputProfile.hpp"
#include "IDependable.hpp"
#include "IJsonSerializable.hpp"

class InputProfileAsset : public Asset, public IDependable<Input::InputManager>
{
private:
	std::optional<Input::InputProfile> m_profile;
public:
	static const std::string EXTENSION;

private:
	void DeserializeFile(Input::InputManager& inputManager);

public:
	InputProfileAsset(const std::filesystem::path& path);

	Input::InputProfile& GetProfileMutable();
	const Input::InputProfile& GetProfile() const;

	void SetDependencies(Input::InputManager& input) override;
};

