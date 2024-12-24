#include <vector>
#include <format>
#include "Component.hpp"
#include "HelperFunctions.hpp"

namespace ECS
{
	//std::string ToString(const ComponentType& type)
//{
//	switch (type)
//	{
//		case ComponentType::LightSource: return "LightSource";
//		case ComponentType::AudioSource: return "AudioSource";
//		default:
//			std::string error = std::format("Tried to convert an undefined component type to string!");
//			Utils::Log(Utils::LogType::Error, error);
//			return "";
//	}
//	return "";
//}

	Component::Component() {}
	Component::~Component() {}

	UpdatePriority Component::GetUpdatePriority() const
	{
		return {};
	}
}
