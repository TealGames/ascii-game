#include "pch.hpp"
#include "ContainerGUI.hpp"

ContainerGUI::ContainerGUI() : ComponentData() {}

void ContainerGUI::InitFields()
{
	m_Fields = {};
}

std::string ContainerGUI::ToString() const
{
	return std::format("[ContainerGUI]");
}

void ContainerGUI::Deserialize(const Json& json)
{
	//TODO: implement
	return;
}
Json ContainerGUI::Serialize()
{
	//TODO: implement
	return {};
}
