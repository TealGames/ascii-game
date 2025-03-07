#include "pch.hpp"
#include "ComponentField.hpp"
#include "Debug.hpp"

ComponentField::ComponentField(const std::string& name, const ComponentFieldVariant& value)
	: m_FieldName(name), m_Value(value), m_MaybeSetFunction(std::nullopt)
{
	//Assert(false, std::format("Tried to create field but wtih value: {}", std::get<Utils::Point2D*>(value)->ToString()));
}

ComponentField::ComponentField(const std::string& name, const ComponentFieldSetAction& setAction, const ComponentFieldVariant& value)
	: m_FieldName(name), m_Value(value), m_MaybeSetFunction(setAction)
{
	if (!Assert(this, setAction.index() == m_Value.index(), std::format("Tried to create a component field named: '{}' with set action, "
		"but that action does not acceot the same type of argument as the internal reference to field: {}!", 
		m_FieldName, GetCurrentType().name()))) 
		throw std::invalid_argument("Invalid set action");
}

bool ComponentField::HasSetFunction() const
{
	return m_MaybeSetFunction != std::nullopt;
}

const std::type_info& ComponentField::GetCurrentType() const
{
	if (IsCurrentType<int>()) return typeid(int);
	else if (IsCurrentType<float>()) return typeid(float);
	else if (IsCurrentType<bool>()) return typeid(bool);
	else if (IsCurrentType<std::string>()) return typeid(std::string);
	else if (IsCurrentType<Vec2>()) return typeid(Vec2);
	else if (IsCurrentType<Vec2Int>()) return typeid(Vec2Int);

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

	else if (IsCurrentType<bool>())
		valueText = std::to_string(*(TryGetValue<bool>()));

	else if (IsCurrentType<std::string>())
		valueText = *(TryGetValue<std::string>());

	else if (IsCurrentType<Vec2>())
		valueText = (TryGetValue<Vec2>())->ToString();

	else if (IsCurrentType<Vec2Int>())
		valueText = (TryGetValue<Vec2Int>())->ToString();

	return std::format("[Field:{}, Value:{}", m_FieldName, valueText);
}
