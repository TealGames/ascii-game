#include "pch.hpp"
#include "ComponentField.hpp"
#include "Debug.hpp"

ComponentField::ComponentField(const std::string& name, const ComponentFieldVariant& value)
	: m_FieldName(name), m_Value(value) 
{
	//Assert(false, std::format("Tried to create field but wtih value: {}", std::get<Utils::Point2D*>(value)->ToString()));
}

const std::type_info& ComponentField::GetCurrentType() const
{
	if (IsCurrentType<int>()) return typeid(int);
	else if (IsCurrentType<float>()) return typeid(float);
	else if (IsCurrentType<std::string>()) return typeid(std::string);
	else if (IsCurrentType<Utils::Point2D>()) return typeid(Utils::Point2D);

	LogError(this, std::format("Tried to retrieve current type for component field but no type was defined for it"));
	throw std::invalid_argument("Invalid type actions");
}

std::string ComponentField::ToString() const
{
	std::string valueText = "";
	if (IsCurrentType<int>())
		valueText = std::to_string(*(TryGetValue<int>()));

	else if (IsCurrentType<float>())
		valueText = std::to_string(*(TryGetValue<float>()));

	else if (IsCurrentType<std::string>())
		valueText = *(TryGetValue<std::string>());

	else if (IsCurrentType<Utils::Point2D>())
		valueText = (TryGetValue<Utils::Point2D>())->ToString();

	return std::format("[Field:{}, Value:{}", m_FieldName, valueText);
}
