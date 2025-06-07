#include "pch.hpp"
#include "UIContainer.hpp"

UIContainer::UIContainer() : Component() {}

void UIContainer::InitFields()
{
	m_Fields = {};
}

std::string UIContainer::ToString() const
{
	return std::format("[ContainerGUI]");
}

void UIContainer::Deserialize(const Json& json)
{
	//TODO: implement
	return;
}
Json UIContainer::Serialize()
{
	//TODO: implement
	return {};
}
