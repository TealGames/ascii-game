#pragma once
#include "Utils/Data/Vec2.hpp"
#include "ECS/Component/Component.hpp"
#include "glm/mat4x4.hpp"
#include "glm/vec3.hpp"
#include <glm/gtc/quaternion.hpp>

//Since negative positions are not allowed
const Vec2 NULL_POS = Vec2{ -1, -1 };

class TransformData : public Component
{
private:
	glm::vec3 m_localPos;
	//Vec2 m_lastLocalPos;
	//Vec2 m_localPosLastFrame;

	glm::vec3 m_localScale;
	glm::quat m_localRotation;
public:

private:
public:
	TransformData();
	TransformData(const Json& json);
	TransformData(const Vec2& pos);

	//TODO: these position setting functions should get moved into transform
	void SetLocalPos(const Vec2& newPos);
	void SetLocalPosX(const float& newX);
	void SetLocalPosY(const float& newY);

	void SetLocalPosDeltaX(const float& xDelta);
	void SetLocalPosDeltaY(const float& yDelta);
	void SetLocalPosDelta(const Vec2& moveDelta);

	Vec2 GetLocalPos() const;
	Vec2 GetGlobalPos() const;
	//Vec2 GetLastPos() const;
	//Vec2 GetLocalPosLastFrame() const;
	//void SetLocalPosLastFrame(const Vec2& vec);
	//bool HasMovedThisFrame() const;

	Vec2 GetLocalScale() const;
	Vec2 GetGlobalScale() const;
	void SetLocalScale(const Vec2 scale);

	glm::mat4 GetLocalModelMatrix() const;
	glm::mat4 GetWorldModelMatrix() const;

	//std::vector<std::string> GetDependencyFlags() const override;
	void InitFields() override;
	std::string ToString() const override;

	void Deserialize(const Json& json) override;
	Json Serialize() override;
};