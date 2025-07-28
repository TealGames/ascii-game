#pragma once
#include "ECS/Component/Component.hpp"
//#include "Math/Vec3.hpp"
//#include "Math/Mat4.hpp"
//#include "Math/Quaternion.hpp"
#include "Utils/Data/Vec3Type.hpp"
#include "Utils/Data/Quaternion.hpp"


class TransformData : public Component
{
private:
public:
	static inline constexpr Vec3 DEFAULT_POS = Vec3::Zero();
	static inline constexpr Vec3 DEFAULT_SCALE = Vec3::One();
	static inline constexpr Quat DEFAULT_ROTATION = Quat::Identity();

	Vec3 m_LocalPos;
	//Vec2 m_lastLocalPos;Math
	//Vec2 m_localPosLastFrame;

	Vec3 m_LocalScale;
	Quat m_LocalRotation;

private:
	

public:
	TransformData(const Json& json);
	TransformData(const Vec3 pos= DEFAULT_POS, const Vec3 scale= DEFAULT_SCALE, 
		const Quat rotation= DEFAULT_ROTATION);

	//TODO: these position setting functions should get moved into transform
	/*void SetLocalPos(const Vec3& newPos);
	void SetLocalPosX(const float& newX);
	void SetLocalPosY(const float& newY);

	void SetLocalPosDeltaX(const float& xDelta);
	void SetLocalPosDeltaY(const float& yDelta);
	void SetLocalPosDelta(const Vec2& moveDelta);*/

	static Mat4 CalculateTranslationMatrix(const Vec3& pos);
	static Mat4 CalculateScaleMatrix(const Vec3& scale);
	static Mat4 CalculateRotationMatrix(const Quat& rotation);

	Mat4 CalculateLocalScaleMatrix() const;
	Mat4 CalculateLocalTranslationMatrix() const;
	Mat4 CalculateLocalRotationMatrix() const;

	//Vec3 GetLocalPos() const;
	Vec3 GetGlobalPos() const;

	//Vec3 GetLocalScale() const;
	Vec3 GetGlobalScale() const;
	//void SetLocalScale(const Vec2 scale);
	Quat GetGlobalRotation() const;

	Mat4 GetLocalModelMatrix() const;
	Mat4 GetWorldModelMatrix() const;

	//std::vector<std::string> GetDependencyFlags() const override;
	void InitFields() override;
	std::string ToString() const override;

	void Deserialize(const Json& json) override;
	Json Serialize() override;
};