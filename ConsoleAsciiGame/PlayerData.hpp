#pragma once
#include "ComponentData.hpp"
#include "PhysicsBodyData.hpp"
#include "Vec2.hpp"

class PlayerData : public ComponentData
{
private:
	PhysicsBodyData* m_body;

	float m_xMoveSpeed;
	float m_initialJumpSpeed;
	float m_maxJumpHeight;

	Utils::Point2DInt m_currentFrameDirectionalInput;
	Utils::Point2DInt m_lastFrameDirectionalInput;

public:

private:
public:
	PlayerData();
	PlayerData(PhysicsBodyData& bodyData, const float& moveSpeed, const float& maxJumpHeight);

	const float& GetMoveSpeed() const;
	const float& GetInitialJumpSpeed() const;
	const bool& GetIsGrounded() const;

	/// <summary>
	/// Will send out a raycast until a ground is found (or ray ends)
	/// Note: this function is expensive so it should be used sparingly
	/// </summary>
	/// <returns></returns>
	float GetVerticalDistanceToGround() const;

	PhysicsBodyData& GetBodyMutableSafe();

	const Utils::Point2DInt& GetFrameInput() const;
	const Utils::Point2DInt& GetLastFrameInput() const;
	Utils::Point2DInt GetInputDelta() const;
	bool HasInputChanged() const;

	void SetFrameInput(const Utils::Point2DInt& input);
	void SetLastFrameInput(const Utils::Point2DInt& lastFrameInput);
};

