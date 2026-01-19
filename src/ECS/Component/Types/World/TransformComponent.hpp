#pragma once
#include "ECS/Component/Component.hpp"
#include "Utils/Math/Vec3Type.hpp"
#include "Utils/Math/Quaternion.hpp"

inline constexpr Vec3 DEFAULT_POS = Vec3::Zero();
inline constexpr Vec3 DEFAULT_SCALE = Vec3::One();
inline constexpr Quat DEFAULT_ROTATION = Quat::Identity();

struct TransformPrecalculatedData
{
	Vec3 m_GlobalPos = DEFAULT_POS;
	Vec3 m_GlobalScale = DEFAULT_SCALE;
	Quat m_GlobalRotation = DEFAULT_ROTATION;
	Mat4 m_GlobalModelMatrix = Mat4::GetIdentity();
	//bool m_UpdatedThisFrame = false;
};

//namespace ECS { class TransformSystem; }
class TransformComponent : public Component
{
private:
	Vec3 m_localPos;
	Vec3 m_localScale;
	Quat m_localRotation;

	mutable TransformPrecalculatedData m_lastUpdateData;
public:
	static constexpr DirtyFlag POS_DIRTY_FLAG = 1;
	static constexpr DirtyFlag SCALE_DIRTY_FLAG = 1 << 1;
	static constexpr DirtyFlag ROTATIOn_DIRTY_FLAG = 1 << 2;

	//friend class ECS::TransformSystem;
private:
	void SetChildrenDirty();
	//Mat4 CalculateLocalModelMatrix() const;

	void UpdatePrecalculatedData() const;

public:
	TransformComponent(const Json& json);
	TransformComponent(const Vec3 pos= DEFAULT_POS, const Vec3 scale= DEFAULT_SCALE, 
		const Quat rotation= DEFAULT_ROTATION);

	/// <summary>
	/// Will force an update to transform if it is dirty.
	/// Returns true if it was dirty/updated, otherwise false
	/// </summary>
	/// <returns></returns>
	bool ForceUpdateIfDirty() const;

	//TODO: these position setting functions should get moved into transform
	/*void SetLocalPos(const Vec3& newPos);
	void SetLocalPosX(const float& newX);
	void SetLocalPosY(const float& newY);

	void SetLocalPosDeltaX(const float& xDelta);
	void SetLocalPosDeltaY(const float& yDelta);
	void SetLocalPosDelta(const Vec2& moveDelta);*/

	const Vec3& GetWorldPos() const;
	const Vec3& GetWorldScale() const;
	const Quat& GetWorldRotation() const;
	const Mat4& GetWorldModelMatrix() const;

	const Vec3& GetLocalPos() const;
	const Vec3& GetLocalScale() const;
	const Quat& GetLocalRotation() const;

	Vec3& GetLocalPosMutable();
	Vec3& GetLocalScaleMutable();
	Quat& GetLocalRotationMutable();

	Vec3 CalculateWorldForward() const;
	Vec3 CalculateWorldUp() const;
	Vec3 CalculateWorldRight() const;
	void CalculateWorldDirections(Vec3* outForward, Vec3* outUp, Vec3* outRight) const;

	const TransformPrecalculatedData& GetLastUpdateData() const;

	//std::vector<std::string> GetDependencyFlags() const override;
	void InitFields() override;
	std::string ToString() const override;

	void Deserialize(const Json& json) override;
	Json Serialize() override;
};