#pragma once
#include "Vec2.hpp"
#include "ComponentData.hpp"

//Since negative positions are not allowed
const Vec2 NULL_POS = Vec2{ -1, -1 };

class TransformData : public ComponentData
{
private:
	Vec2 m_Pos;
	Vec2 m_LastPos;
	Vec2 m_LastFramePos;

public:

private:
public:
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

	Vec2 GetPos() const;
	Vec2 GetLastPos() const;
	Vec2 GetLastFramePos() const;
	
	void SetLastFramePos(const Vec2& vec);

	bool HasMovedThisFrame() const;

	std::vector<std::string> GetDependencyFlags() const override;
	void InitFields() override;
	std::string ToString() const override;

	void Deserialize(const Json& json) override;
	Json Serialize() override;
};