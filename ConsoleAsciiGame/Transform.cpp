#include "Transform.hpp"
#include "Point2DInt.hpp"

namespace ECS
{
	Transform::Transform(const Utils::Point2DInt& startPos) :
		Component(), m_pos(startPos), m_Pos(m_pos)
	{

	}

	UpdatePriority Transform::GetUpdatePriority() const
	{
		return {0};
	}

	void Transform::Start() {}
	void Transform::Update(float deltaTime) {}

	//TODO: do scene bound checking
	void Transform::SetPos(const Utils::Point2DInt& newPos)
	{
		m_pos = newPos;
	}

	//TODO: do scene bound checking
	void Transform::SetPosX(const int& newX)
	{
		m_pos.m_X = newX;
	}

	//TODO: do scene bound checking
	void Transform::SetPosY(const int& newY)
	{
		m_pos.m_Y = newY;
	}

	void Transform::SetDeltaX(const int& xDelta)
	{
		SetPosX(m_Pos.m_X + xDelta);
	}

	void Transform::SetDeltaY(const int yDelta)
	{
		SetPosY(m_Pos.m_Y + yDelta);
	}
}

