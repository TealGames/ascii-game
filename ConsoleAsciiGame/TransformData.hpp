#pragma once
#include "Point2DInt.hpp"
#include "Point2D.hpp"
#include "ComponentData.hpp"

//Since negative positions are not allowed
const Utils::Point2D NULL_POS = Utils::Point2D{ -1, -1 };

struct TransformData : public ComponentData
{
	Utils::Point2D m_Pos;
	Utils::Point2D m_LastPos;
	Utils::Point2D m_LastFramePos;

	bool m_IsFat;

	TransformData();
	TransformData(const Utils::Point2D& pos);

	//TODO: these position setting functions should get moved into transform
	void SetPos(const Utils::Point2D& newPos);
	void SetPosX(const float& newX);
	void SetPosY(const float& newY);

	void SetPosDeltaX(const float& xDelta);
	void SetPosDeltaY(const float& yDelta);
	void SetPosDelta(const Utils::Point2D& moveDelta);

	bool HasMovedThisFrame() const;

	void InitFields() override;
};