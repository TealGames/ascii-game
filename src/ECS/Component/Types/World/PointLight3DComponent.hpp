#pragma once
#include "ECS/Component/Component.hpp"
#include "Utils/Data/Color.hpp"

class PointLight3DComponent : public Component
{
private:
public:
	float m_Radius;
	Color m_Color;

private:
public:
	PointLight3DComponent();
	PointLight3DComponent(const float radius, const Color color);

	void InitFields() override;
	std::string ToString() const override;

	void Deserialize(const Json& json) override;
	Json Serialize() override;
};