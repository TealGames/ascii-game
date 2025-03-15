#pragma once
#include "Vec2.hpp"
#include "ComponentData.hpp"

//Since negative positions are not allowed
const Vec2 NULL_POS = Vec2{ -1, -1 };

struct TransformData : public ComponentData
{
	Vec2 m_Pos;
	Vec2 m_LastPos;
	Vec2 m_LastFramePos;

	TransformData();
	TransformData(const Json& json);
	TransformData(const Vec2& pos);

	//TODO: these position setting functions should get moved into transform
	void SetPos(const Vec2& newPos);
	void SetPosX(const float& newX);
	void SetPosY(const float& newY);

	void SetPosDeltaX(const float& xDelta);
	void SetPosDeltaY(const float& yDelta);
	void SetPosDelta(const Vec2& moveDelta);

	bool HasMovedThisFrame() const;

	void InitFields() override;
	std::string ToString() const override;

	void Deserialize(const Json& json) override;
	Json Serialize() override;
};