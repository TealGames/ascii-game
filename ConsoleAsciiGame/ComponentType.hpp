#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <limits>
#include <cmath>
#include "CameraData.hpp"
#include "TransformData.hpp"
#include "PlayerData.hpp"
#include "LightSourceData.hpp"
#include "EntityRendererData.hpp"
#include "HelperFunctions.hpp"

//This means that the max number of components supported is 64 
// (the amount of bits for the type)
using ComponentNumericType = std::uint64_t;
//The max componenent types supported is the number of bits since each bit is used 
//in order to allow for multiple component types
constexpr std::uint8_t MAX_COMPONENT_TYPES = sizeof(ComponentNumericType) * 8;
enum class ComponentType : ComponentNumericType
{
	None=0,
	Transform= 1<<0,
	EntityRenderer= 1 << 1,
	LightSource= 1<<2,
	Camera= 1<<3,
	Player= 1<<4
};

ComponentNumericType operator|(const ComponentType& lhs, const ComponentType& rhs);
ComponentType& operator|=(ComponentType& lhs, const ComponentType& rhs);
ComponentNumericType operator&(const ComponentType& lhs, const ComponentType& rhs);
ComponentType& operator&=(ComponentType& lhs, const ComponentType& rhs);

std::uint8_t GetPlaceOfComponentType(const ComponentType& type);

/// <summary>
/// Will get the component type based on its distance from the first one
/// For example func(0) would return Transform since it is the first one, func(1) returns
/// EntityRenderer, and so on. Returns none if the value is greater than the max amount of elements
/// </summary>
/// <param name="value"></param>
/// <returns></returns>
ComponentType GetComponentTypeByPlace(const std::uint8_t value);
std::string ToString(const ComponentType& type);

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

//NOTE: it seems this function fails (maybe the neccessary incldues cause it)?
//cuasing this include to get messsed up in enttity mapper and other places
template<typename T>
ComponentType GetComponentFromType()
{
	//TODO: maybe a static map would be faster with O(1) lookup then checking every possible option?
	if (typeid(T) == typeid(CameraData)) return ComponentType::Camera;
	if (typeid(T) == typeid(EntityRendererData)) return ComponentType::EntityRenderer;
	if (typeid(T) == typeid(LightSourceData)) return ComponentType::LightSource;
	if (typeid(T) == typeid(PlayerData)) return ComponentType::Player;
	if (typeid(T) == typeid(TransformData)) return ComponentType::Transform;
	
	Utils::Log(Utils::LogType::Error, std::format("Tried to parse type: {} "
		"to component type but failed!", typeid(T).name()));
	return ComponentType::None;
}

