#include "pch.hpp"
#include "UIObjectData.hpp"
#include "JsonSerializers.hpp"

UIObjectData::UIObjectData() : UIObjectData(NormalizedPosition()) {}

UIObjectData::UIObjectData(const Json& json) : UIObjectData()
{
	Deserialize(json);
}
UIObjectData::UIObjectData(const NormalizedPosition& normalizedPos) :
	m_normalizedPos(normalizedPos) 
{

}

const NormalizedPosition& UIObjectData::GetNormalizedPos() const
{
	return m_normalizedPos;
}

void UIObjectData::InitFields()
{
	m_Fields = { ComponentField("Pos", [this](Vec2 newPos)-> void 
		{ m_normalizedPos.SetPos(newPos); }, &m_normalizedPos.GetPosMutable()) };
}

UIObjectData& UIObjectData::Deserialize(const Json& json)
{
	m_normalizedPos.SetPos(json.at("Pos").get<Vec2>());
	return *this;
}
Json UIObjectData::Serialize(const UIObjectData & component)
{
	return { {"Pos", m_normalizedPos.GetPos()}};
}

std::string UIObjectData::ToString() const
{
	return std::format("[UIObject Pos:{}]", m_normalizedPos.GetPos().ToString());
}