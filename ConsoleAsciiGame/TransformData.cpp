//NOT USED
#include "pch.hpp"

#include "TransformData.hpp"
#include "Debug.hpp"
#include "Entity.hpp"
#include "JsonSerializers.hpp"

TransformData::TransformData() : TransformData(Vec2{}) {}

TransformData::TransformData(const Json& json) : TransformData()
{
	Deserialize(json);
}

TransformData::TransformData(const Vec2& pos) :
	ComponentData(HighestDependecyLevel::None),
	m_Pos(pos), m_LastPos(NULL_POS), m_LastFramePos(NULL_POS)
{
	
}

void TransformData::SetPos(const Vec2& newPos)
{
	m_LastPos = m_Pos;
	m_Pos = newPos;
}

//TODO: do scene bound checking
void TransformData::SetPosX(const float& newX)
{
	SetPos(Vec2(newX, m_Pos.m_Y));
}

//TODO: do scene bound checking
void TransformData::SetPosY(const float& newY)
{
	SetPos(Vec2(m_Pos.m_X, newY));
}

void TransformData::SetPosDeltaX(const float& xDelta)
{
	SetPosX(m_Pos.m_X + xDelta);
}

void TransformData::SetPosDeltaY(const float& yDelta)
{
	SetPosY(m_Pos.m_Y + yDelta);
}

void TransformData::SetPosDelta(const Vec2& moveDelta)
{
	SetPosDeltaX(moveDelta.m_X);
	SetPosDeltaY(moveDelta.m_Y);
}

bool TransformData::HasMovedThisFrame() const
{
	return m_LastFramePos == NULL_POS || m_Pos != m_LastFramePos;
}

void TransformData::InitFields()
{
	m_Fields = { ComponentField("Pos", [this](Vec2 newPos)-> void { SetPos(newPos); }, &m_Pos) };
	/*Assert(false, std::format("Tried to create field AT TRANFOEMR for: {} but wtih value: {} ACTUAL{}",
		GetEntitySafe().m_Name, std::get<Vec2*>(m_Fields[0].m_Value)->ToString(), m_Pos.ToString()));*/
		/*Assert(false, std::format("Tried to create field AT TRANFOEMR for: {} but wtih value: {} ACTUAL{}",
			GetEntitySafe().m_Name, std::get<Vec2*>(m_Fields[0].m_Value)->ToString(), m_Pos.ToString()));*/
	//Assert(false, std::format("Created value: {}", std::get<Vec2*>(m_Fields[0].m_Value)->ToString()));
}

void TransformData::Deserialize(const Json& json)
{
	m_Pos = json.at("Pos").get<Vec2>();
	m_LastPos = json.at("LastPos").get<Vec2>();
	m_LastFramePos = json.at("LastFramePos").get<Vec2>();
}
Json TransformData::Serialize()
{
	return { {"Pos", m_Pos}, {"LastPos", m_LastPos}, {"LastFramePos", m_LastFramePos}};
}

std::string TransformData::ToString() const
{
	return std::format("[<Transform> Pos: {}]", m_Pos.ToString());
}