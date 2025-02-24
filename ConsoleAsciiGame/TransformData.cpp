//NOT USED
#include "pch.hpp"

#include "TransformData.hpp"
#include "Debug.hpp"
#include "Entity.hpp"

TransformData::TransformData() : TransformData(Utils::Point2D{}) {}

TransformData::TransformData(const Utils::Point2D& pos) :
	ComponentData(), m_Pos(pos), m_LastPos(NULL_POS), m_LastFramePos(NULL_POS)
{
	
}

void TransformData::SetPos(const Utils::Point2D& newPos)
{
	m_LastPos = m_Pos;
	m_Pos = newPos;
}

//TODO: do scene bound checking
void TransformData::SetPosX(const float& newX)
{
	SetPos(Utils::Point2D(newX, m_Pos.m_Y));
}

//TODO: do scene bound checking
void TransformData::SetPosY(const float& newY)
{
	SetPos(Utils::Point2D(m_Pos.m_X, newY));
}

void TransformData::SetPosDeltaX(const float& xDelta)
{
	SetPosX(m_Pos.m_X + xDelta);
}

void TransformData::SetPosDeltaY(const float& yDelta)
{
	SetPosY(m_Pos.m_Y + yDelta);
}

void TransformData::SetPosDelta(const Utils::Point2D& moveDelta)
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
	m_Fields = { ComponentField("Pos", &m_Pos) };
	/*Assert(false, std::format("Tried to create field AT TRANFOEMR for: {} but wtih value: {} ACTUAL{}",
		GetEntitySafe().m_Name, std::get<Utils::Point2D*>(m_Fields[0].m_Value)->ToString(), m_Pos.ToString()));*/
		/*Assert(false, std::format("Tried to create field AT TRANFOEMR for: {} but wtih value: {} ACTUAL{}",
			GetEntitySafe().m_Name, std::get<Utils::Point2D*>(m_Fields[0].m_Value)->ToString(), m_Pos.ToString()));*/
	//Assert(false, std::format("Created value: {}", std::get<Utils::Point2D*>(m_Fields[0].m_Value)->ToString()));
}