#pragma once
#include "ECS/Component/Component.hpp"
//#include "Math/Vec3.hpp"
//#include "Math/Mat4.hpp"
//#include "Math/Quaternion.hpp"
#include "Utils/Data/Vec3Type.hpp"
#include "Utils/Data/Quaternion.hpp"

inline constexpr Vec3 DEFAULT_POS = Vec3::Zero();
inline constexpr Vec3 DEFAULT_SCALE = Vec3::One();
inline constexpr Quat DEFAULT_ROTATION = Quat::Identity();

struct TransformPrecalculatedData
{
	Vec3 m_GlobalPos = DEFAULT_POS;
	Vec3 m_GlobalScale = DEFAULT_SCALE;
	Quat m_GlobalRotation = DEFAULT_ROTATION;
	Mat4 m_GlobalModelMatrix = Mat4::GetIdentity();
};

class TransformComponent : public Component
{
private:
	Vec3 m_localPos;

	Vec3 m_localScale;
	Quat m_localRotation;

	mutable TransformPrecalculatedData m_lastUpdateData;
public:
private:
	void SetChildrenDirty();
	//Mat4 CalculateLocalModelMatrix() const;

	void UpdatePrecalculatedData() const;

public:
	TransformComponent(const Json& json);
	TransformComponent(const Vec3 pos= DEFAULT_POS, const Vec3 scale= DEFAULT_SCALE, 
		const Quat rotation= DEFAULT_ROTATION);

	//TODO: these position setting functions should get moved into transform
	/*void SetLocalPos(const Vec3& newPos);
	void SetLocalPosX(const float& newX);
	void SetLocalPosY(const float& newY);

	void SetLocalPosDeltaX(const float& xDelta);
	void SetLocalPosDeltaY(const float& yDelta);
	void SetLocalPosDelta(const Vec2& moveDelta);*/

	const Vec3& GetGlobalPos() const;
	const Vec3& GetGlobalScale() const;
	const Quat& GetGlobalRotation() const;
	const Mat4& GetWorldModelMatrix() const;

	const Vec3& GetLocalPos() const;
	const Vec3& GetLocalScaleMutable() const;
	const Quat& GetLocalRotationMutable() const;

	Vec3& GetLocalPosMutable();
	Vec3& GetLocalScaleMutable();
	Quat& GetLocalRotationMutable();

	const TransformPrecalculatedData& GetLastUpdateData() const;

	//std::vector<std::string> GetDependencyFlags() const override;
	void InitFields() override;
	std::string ToString() const override;

	void Deserialize(const Json& json) override;
	Json Serialize() override;
};

Mat4 CalculateTranslationMatrix(const Vec3& pos);
Mat4 CalculateScaleMatrix(const Vec3& scale);
Mat4 CalculateRotationMatrix(const Quat& rotation);
Mat4 CalculateModelMatrix(const Mat4* parentMatrix, const Vec3& pos, const Vec3& scale, const Quat& rotation);