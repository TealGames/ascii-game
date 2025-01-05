#include "pch.hpp"

#include "ComponentType.hpp"
#include "HelperFunctions.hpp"

ComponentNumericType operator|(const ComponentType& lhs, const ComponentType& rhs)
{
	return static_cast<ComponentNumericType>(lhs) | static_cast<ComponentNumericType>(rhs);
}

ComponentType& operator|=(ComponentType& lhs, const ComponentType& rhs)
{
	lhs = static_cast<ComponentType>(lhs | rhs);
	return lhs;
}

ComponentNumericType operator&(const ComponentType& lhs, const ComponentType& rhs)
{
	return static_cast<ComponentNumericType>(lhs) & static_cast<ComponentNumericType>(rhs);
}

ComponentType& operator&=(ComponentType& lhs, const ComponentType& rhs)
{
	lhs = static_cast<ComponentType>(lhs & rhs);
	return lhs;
}

std::uint8_t GetPlaceOfComponentType(const ComponentType& type)
{
	ComponentNumericType integralType = static_cast<ComponentNumericType>(type);
	Utils::Log(std::format("integral type for: {} is: {}", ToString(type), std::to_string(integralType)));
	return static_cast<std::uint8_t>(std::log2(integralType));
}

/// <summary>
/// Will get the component type based on its distance from the first one
/// For example func(0) would return Transform since it is the first one, func(1) returns
/// EntityRenderer, and so on. Returns none if the value is greater than the max amount of elements
/// </summary>
/// <param name="value"></param>
/// <returns></returns>
ComponentType GetComponentTypeByPlace(const std::uint8_t value)
{
	if (!Utils::Assert(value <= MAX_COMPONENT_TYPES - 1, std::format("Tried to get component type by palce: {} "
		"but it is outside the possible range of enum values", std::to_string(value)))) return ComponentType::None;

	ComponentNumericType one = 1;
	ComponentNumericType componentBit = one << (static_cast<ComponentNumericType>(value) + 1);
	return static_cast<ComponentType>(componentBit);
}

std::string ToString(const ComponentType& type)
{
	const ComponentNumericType integralType = static_cast<ComponentNumericType>(type);
	if (integralType == 0) return "";

	std::vector<std::string> allStrs = {};
	if ((type & ComponentType::Transform) != 0) allStrs.push_back("Transform");
	if ((type & ComponentType::EntityRenderer) != 0) allStrs.push_back("EntityRenderer");
	if ((type & ComponentType::LightSource) != 0) allStrs.push_back("LightSource");
	if ((type & ComponentType::Camera) != 0) allStrs.push_back("Camera");
	if ((type & ComponentType::Player) != 0)allStrs.push_back("Player");

	return Utils::ToStringIterable<std::vector<std::string>, std::string>(allStrs);
}

//template<typename T>
//ComponentType GetComponentFromType()
//{
//	ComponentNumericType currentBit = 1;
//	ComponentNumericType finalBit = 1 << Utils::GetTypeBitSize<ComponentNumericType>();
//
//	const std::string typeStr = typeid(T).name();
//	std::string currentCompStr = "";
//	ComponentType currentType = ComponentType::None;
//	while (currentBit <= finalBit)
//	{
//		currentType = static_cast<ComponentType>(currentBit);
//		currentCompStr = ToString(currentType);
//		if (currentCompStr == typeStr) return currentType;
//
//		if (currentBit!=finalbit) currentBit << 1;
//	}
//	return ComponentType::None;
//}