#include "pch.hpp"
#include "UIObjectData.hpp"
#include "JsonSerializers.hpp"

UIObjectData::UIObjectData() : UIObjectData(NormalizedPosition()) {}

UIObjectData::UIObjectData(const Json& json) : UIObjectData()
{
	Deserialize(json);
}
UIObjectData::UIObjectData(const NormalizedPosition& normalizedPos) :
	ComponentData(), m_normalizedPos(normalizedPos)
{

}

const NormalizedPosition& UIObjectData::GetNormalizedPos() const
{
	return m_normalizedPos;
}

void UIObjectData::InitFields()
{
	//TODO: we do not want to allow normalized pos to have getposmutable avaialbe so we do
	//not mess up internal vec2, but var for field needs to be mutable?
	m_Fields = {};
	/*m_Fields = { ComponentField("Pos", [this](Vec2 newPos)-> void 
		{ m_normalizedPos.SetPos(newPos); }, &m_normalizedPos.GetPosMutable()) };*/
}
std::vector<std::string> UIObjectData::GetDependencyFlags() const
{
	return {};
}

void UIObjectData::Deserialize(const Json& json)
{
	m_normalizedPos.SetPos(json.at("Pos").get<Vec2>());
}
Json UIObjectData::Serialize()
{
	return { {"Pos", m_normalizedPos.GetPos()}};
}

std::string UIObjectData::ToString() const
{
	return std::format("[UIObject Pos:{}]", m_normalizedPos.GetPos().ToString());
}