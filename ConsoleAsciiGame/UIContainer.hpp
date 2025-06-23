#pragma once
#include "Component.hpp"

class UIContainer : public Component
{
private:
public:

private:
public:
	UIContainer();

	void InitFields() override;

	std::string ToString() const override;

	void Deserialize(const Json& json) override;
	Json Serialize() override;
};