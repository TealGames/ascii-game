#pragma once
#include "ComponentData.hpp"

class ContainerGUI : public ComponentData
{
private:
public:

private:
public:
	ContainerGUI();

	void InitFields() override;

	std::string ToString() const override;

	void Deserialize(const Json& json) override;
	Json Serialize() override;
};