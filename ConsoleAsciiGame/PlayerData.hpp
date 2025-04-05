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

	Vec2Int m_currentFrameDirectionalInput;
	Vec2Int m_lastFrameDirectionalInput;

public:

private:
	float CalculateInitialJumpSpeed() const;

private:
	PlayerData(PhysicsBodyData* body, const float& moveSpeed, const float& maxJumpHeight);

	void TrySetInitialJumpSpeed();

public:
	PlayerData();
	PlayerData(const Json& json);
	//TODO: what if the wrong boddy is provided meaning one that does not share the same entity as this
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
	const PhysicsBodyData& GetBodySafe() const;

	const Vec2Int& GetFrameInput() const;
	const Vec2Int& GetLastFrameInput() const;
	Vec2Int GetInputDelta() const;
	bool HasInputChanged() const;

	void SetFrameInput(const Vec2Int& input);
	void SetLastFrameInput(const Vec2Int& lastFrameInput);

	std::vector<std::string> GetDependencyFlags() const override;
	void InitFields() override;

	std::string ToString() const override;

	void Deserialize(const Json& json) override;
	Json Serialize() override;
};

