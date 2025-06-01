//NOT USED
#include "pch.hpp"

#include "TransformData.hpp"
#include "Debug.hpp"
#include "EntityData.hpp"
#include "JsonSerializers.hpp"

TransformData::TransformData() : TransformData(Vec2{}) {}

TransformData::TransformData(const Json& json) : TransformData()
{
	Deserialize(json);
}

TransformData::TransformData(const Vec2& pos) :
	ComponentData(),
	m_localPos(pos), m_localPosLastFrame(NULL_POS)
{
	
}

void TransformData::SetLocalPos(const Vec2& newPos)
{
	SetLocalPosX(newPos.m_X);
	SetLocalPosY(newPos.m_Y);
}

void TransformData::SetLocalPosX(const float& newX)
{
	m_localPos.m_X = newX;
}

void TransformData::SetLocalPosY(const float& newY)
{
	m_localPos.m_Y = newY;
}

void TransformData::SetLocalPosDeltaX(const float& xDelta)
{
	SetLocalPosX(m_localPos.m_X + xDelta);
}

void TransformData::SetLocalPosDeltaY(const float& yDelta)
{
	SetLocalPosY(m_localPos.m_Y + yDelta);
}

void TransformData::SetLocalPosDelta(const Vec2& moveDelta)
{
	SetLocalPosDeltaX(moveDelta.m_X);
	SetLocalPosDeltaY(moveDelta.m_Y);
}

Vec2 TransformData::GetLocalPos() const
{
	return m_localPos;
}
Vec2 TransformData::GetGlobalPos() const
{
	const EntityData* parentEntity = GetEntitySafe().GetParent();
	return parentEntity != nullptr ? parentEntity->GetTransform().GetGlobalPos() + m_localPos : m_localPos;
}
Vec2 TransformData::GetLocalPosLastFrame() const
{
	return m_localPosLastFrame;
}

void TransformData::SetLocalPosLastFrame(const Vec2& pos)
{
	m_localPosLastFrame = pos;
}

bool TransformData::HasMovedThisFrame() const
{
	return m_localPosLastFrame == NULL_POS || m_localPos != m_localPosLastFrame;
}

void TransformData::InitFields()
{
	m_Fields = { ComponentField("Pos", [this](Vec2 newPos)-> void { SetLocalPos(newPos); }, &m_localPos) };
	/*Assert(false, std::format("Tried to create field AT TRANFOEMR for: {} but wtih value: {} ACTUAL{}",
		GetEntitySafe().m_Name, std::get<Vec2*>(m_Fields[0].m_Value)->ToString(), m_Pos.ToString()));*/
		/*Assert(false, std::format("Tried to create field AT TRANFOEMR for: {} but wtih value: {} ACTUAL{}",
			GetEntitySafe().m_Name, std::get<Vec2*>(m_Fields[0].m_Value)->ToString(), m_Pos.ToString()));*/
	//Assert(false, std::format("Created value: {}", std::get<Vec2*>(m_Fields[0].m_Value)->ToString()));
}
//std::vector<std::string> TransformData::GetDependencyFlags() const
//{
//	return {};
//}

void TransformData::Deserialize(const Json& json)
{
	m_localPos = json.at("Pos").get<Vec2>();
	m_localPosLastFrame = json.at("LastFramePos").get<Vec2>();
}
Json TransformData::Serialize()
{
	return { {"Pos", m_localPos}, {"LastFramePos", m_localPosLastFrame}};
}

std::string TransformData::ToString() const
{
	return std::format("[<Transform> Pos: {}]", m_localPos.ToString());
}