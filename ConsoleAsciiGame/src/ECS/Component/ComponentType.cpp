#include "pch.hpp"

#include "ECS/Component/ComponentType.hpp"
#include "Utils/HelperFunctions.hpp"

std::uint8_t GetPlaceOfComponentType(const ComponentType& type)
{
	ComponentNumericType integralType = static_cast<ComponentNumericType>(type);
	Log(std::format("integral type for: {} is: {}", ToString(type), std::to_string(integralType)));
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
	if (!Assert(value <= MAX_COMPONENT_TYPES - 1, std::format("Tried to get component type by palce: {} "
		"but it is outside the possible range of enum values", std::to_string(value)))) return ComponentType::None;

	ComponentNumericType one = 1;
	ComponentNumericType componentBit = one << static_cast<ComponentNumericType>(value);
	return static_cast<ComponentType>(componentBit);
}

ComponentType MergeComponents(const std::vector<ComponentType> components)
{
	ComponentType merged = ComponentType::None;
	for (const auto& comp : components) merged |= comp;
	return merged;
}

std::string ToString(const ComponentType& type)
{
	const ComponentNumericType integralType = static_cast<ComponentNumericType>(type);
	if (integralType == 0) return "";

	std::vector<std::string> allStrs = {};
	if ((type & ComponentType::Transform) != ComponentType::None) allStrs.emplace_back("Transform");
	if ((type & ComponentType::EntityRenderer) != ComponentType::None) allStrs.emplace_back("EntityRenderer");
	if ((type & ComponentType::LightSource) != ComponentType::None) allStrs.emplace_back("LightSource");
	if ((type & ComponentType::Camera) != ComponentType::None) allStrs.emplace_back("Camera");
	if ((type & ComponentType::Player) != ComponentType::None) allStrs.emplace_back("Player");

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