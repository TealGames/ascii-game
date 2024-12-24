#include "Transform.hpp"
#include "Point2DInt.hpp"

namespace ECS
{
	const Utils::Point2DInt Transform::NULL_POS= Utils::Point2DInt{ -1, -1 };

	Transform::Transform(const Utils::Point2DInt& startPos) :
		Component(), m_pos(startPos), m_Pos(m_pos), m_lastPos(NULL_POS), m_lastFramePos(NULL_POS)
	{

	}

	UpdatePriority Transform::GetUpdatePriority() const
	{
		return {MAX_PRIORITY-1};
	}

	void Transform::Start() {}
	void Transform::UpdateStart(float deltaTime) {}
	void Transform::UpdateEnd(float deltaTime) 
	{
		m_lastFramePos = m_Pos;
	}

	//TODO: do scene bound checking
	void Transform::SetPos(const Utils::Point2DInt& newPos)
	{
		m_lastPos = m_pos;
		m_pos = newPos;
	}

	//TODO: do scene bound checking
	void Transform::SetPosX(const int& newX)
	{
		SetPos(Utils::Point2DInt(newX, m_pos.m_Y));
	}

	//TODO: do scene bound checking
	void Transform::SetPosY(const int& newY)
	{
		SetPos(Utils::Point2DInt(m_pos.m_X, newY));
	}

	void Transform::SetDeltaX(const int& xDelta)
	{
		SetPosX(m_Pos.m_X + xDelta);
	}

	void Transform::SetDeltaY(const int yDelta)
	{
		SetPosY(m_Pos.m_Y + yDelta);
	}

	bool Transform::HasMovedThisFrame() const
	{
		return m_lastFramePos==NULL_POS || m_Pos != m_lastFramePos;
	}
}

