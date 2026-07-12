#pragma once
#include "ECS/Component/Component.hpp"
#include "Core/Primitives/Vector.hpp"
#include "Math/Quaternion.hpp"

namespace Engine
{
	inline constexpr Vec3 DEFAULT_POS = Vec3::Zero();
	inline constexpr Vec3 DEFAULT_SCALE = Vec3::One();
	inline constexpr Math::Quat DEFAULT_ROTATION = Math::Quat::Identity();

	struct TransformPrecalculatedData
	{
		Vec3 m_GlobalPos = DEFAULT_POS;
		Vec3 m_GlobalScale = DEFAULT_SCALE;
		Math::Quat m_GlobalRotation = DEFAULT_ROTATION;
		Mat4 m_GlobalModelMatrix = Mat4::GetIdentity();
		//bool m_UpdatedThisFrame = false;
	};

	//namespace ECS { class TransformSystem; }
	class TransformComponent : public ECS::Component
	{
	private:
		Vec3 m_localPos;
		Vec3 m_localScale;
		Math::Quat m_localRotation;

		mutable TransformPrecalculatedData m_lastUpdateData;
	public:
		static constexpr ECS::DirtyFlag POS_DIRTY_FLAG = 1;
		static constexpr ECS::DirtyFlag SCALE_DIRTY_FLAG = 1 << 1;
		static constexpr ECS::DirtyFlag ROTATIOn_DIRTY_FLAG = 1 << 2;

		//friend class ECS::TransformSystem;
	private:
		void SetChildrenDirty();
		//Mat4 CalculateLocalModelMatrix() const;

		void UpdatePrecalculatedData() const;

	public:
		TransformComponent(const Vec3 pos = DEFAULT_POS, const Vec3 scale = DEFAULT_SCALE,
			const Math::Quat rotation = DEFAULT_ROTATION);

		/// <summary>
		/// Will force an update to transform if it is dirty.
		/// Returns true if it was dirty/updated, otherwise false
		/// </summary>
		/// <returns></returns>
		bool ForceUpdateIfDirty() const;

		const Vec3& GetWorldPos() const;
		const Vec3& GetWorldScale() const;
		const Math::Quat& GetWorldRotation() const;
		const Mat4& GetWorldModelMatrix() const;

		const Vec3& GetLocalPos() const;
		const Vec3& GetLocalScale() const;
		const Math::Quat& GetLocalRotation() const;

		Vec3& GetLocalPosMutable();
		Vec3& GetLocalScaleMutable();
		Math::Quat& GetLocalRotationMutable();

		Vec3 CalculateWorldForward() const;
		Vec3 CalculateWorldUp() const;
		Vec3 CalculateWorldRight() const;
		void CalculateWorldDirections(Vec3* outForward, Vec3* outUp, Vec3* outRight) const;

		const TransformPrecalculatedData& GetLastUpdateData() const;

		void InitFields() override;
		void Serialize(Serialization::Serializer& serializer) const override;
		void Deserialize(Serialization::Deserializer& deserializer) override;
		std::string ToString() const override;
	};
}