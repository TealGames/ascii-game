#include "pch.hpp"
#include "ECS/Component/Types/World/TransformComponent.hpp"
#include "Utils/Debug.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "Math/PlatformMath.hpp"
#include "Math/Math3d.hpp"
#include "Core/Serialization/Serializer.hpp"

namespace Engine
{
	TransformComponent::TransformComponent(const Vec3 pos, const Vec3 scale, const Math::Quat rotation) :
		Component(),
		m_localPos(pos), //m_localPosLastFrame(NULL_POS), 
		m_localScale(scale),
		m_localRotation(rotation),
		m_lastUpdateData()
	{
		//By default we set dirty so that global pos can be calculated 
		//the first time we retrieve it
		//m_isDirty = true;
		SetAllFlagsDirty(true);
	}
	bool TransformComponent::ForceUpdateIfDirty() const
	{
		const bool needsUpdate = IsDirty();
		if (IsDirty())
		{
			UpdatePrecalculatedData();
			SetAllFlagsDirty(false);
			//m_isDirty = false;
		}
		return needsUpdate;
	}
	void TransformComponent::UpdatePrecalculatedData() const
	{
		const TransformComponent* parent = GetEntity().GetParentTransform();

		//m_lastUpdateData.m_UpdatedThisFrame = true;
		if (parent == nullptr)
		{
			m_lastUpdateData.m_GlobalPos = m_localPos;
			m_lastUpdateData.m_GlobalScale = m_localScale;
			m_lastUpdateData.m_GlobalRotation = m_localRotation;
		}
		else if (!parent->IsDirty())
		{
			m_lastUpdateData.m_GlobalPos = parent->m_lastUpdateData.m_GlobalPos + m_localPos;
			m_lastUpdateData.m_GlobalScale = parent->m_lastUpdateData.m_GlobalScale * m_localScale;
			m_lastUpdateData.m_GlobalRotation = parent->m_lastUpdateData.m_GlobalRotation * m_localRotation;
		}
		//if the parent is dirty, we just invoke the same get functions which will cause it to update its
		//own precalculated data and will keep going up the hierarchy and once recursion collapses the 
		//laste line will have the updated global data
		else
		{
			m_lastUpdateData.m_GlobalPos = parent->GetWorldPos() + m_localPos;
			m_lastUpdateData.m_GlobalScale = parent->GetWorldScale() * m_localScale;
			m_lastUpdateData.m_GlobalRotation = parent->GetWorldRotation() * m_localRotation;
		}
		m_lastUpdateData.m_GlobalModelMatrix = Math::CalculateModelMatrix(nullptr, m_lastUpdateData.m_GlobalPos,
			m_lastUpdateData.m_GlobalScale, m_lastUpdateData.m_GlobalRotation);
	}

	const Vec3& TransformComponent::GetWorldPos() const
	{
		//TODO: isnt it a little hacky to use MUTABLE modifier to the last update data
		//so you can modify internal state even in const function?
		if (IsDirty())
		{
			UpdatePrecalculatedData();
			SetAllFlagsDirty(false);
			//m_isDirty = false;
		}

		return m_lastUpdateData.m_GlobalPos;
	}
	const Vec3& TransformComponent::GetWorldScale() const
	{
		if (IsDirty())
		{
			UpdatePrecalculatedData();
			SetAllFlagsDirty(false);
			//m_isDirty = false;
		}

		return m_lastUpdateData.m_GlobalScale;
	}
	const Math::Quat& TransformComponent::GetWorldRotation() const
	{
		if (IsDirty())
		{
			UpdatePrecalculatedData();
			SetAllFlagsDirty(false);
			//m_isDirty = false;
		}

		return m_lastUpdateData.m_GlobalRotation;
	}
	const Mat4& TransformComponent::GetWorldModelMatrix() const
	{
		if (IsDirty())
		{
			UpdatePrecalculatedData();
			SetAllFlagsDirty(false);
			//m_isDirty = false;
		}
		return m_lastUpdateData.m_GlobalModelMatrix;
	}

	void TransformComponent::SetChildrenDirty()
	{
		for (auto& child : GetEntityMutable().GetChildrenOfTypeMutable<TransformComponent>())
		{
			//If we reach a child that is already dirty, it means the its children SHOULD ALREADY BE DIRTY
			//(because the dirty setting only occurs on pos, scale, rot mutation)
			if (child->IsDirty())
				continue;

			//child->m_isDirty = true;
			child->SetAllFlagsDirty(true);
			child->SetChildrenDirty();
		}
	}

	const Vec3& TransformComponent::GetLocalPos() const
	{
		return m_localPos;
	}
	const Vec3& TransformComponent::GetLocalScale() const
	{
		return m_localScale;
	}
	const Math::Quat& TransformComponent::GetLocalRotation() const
	{
		return m_localRotation;
	}

	Vec3& TransformComponent::GetLocalPosMutable()
	{
		if (!IsDirty())
		{
			//m_isDirty = true
			SetAllFlagsDirty(true);
			SetChildrenDirty();
		}

		return m_localPos;
	}
	Vec3& TransformComponent::GetLocalScaleMutable()
	{
		if (!IsDirty())
		{
			//m_isDirty = true;
			SetAllFlagsDirty(true);
			SetChildrenDirty();
		}
		return m_localScale;
	}
	Math::Quat& TransformComponent::GetLocalRotationMutable()
	{
		if (!IsDirty())
		{
			//m_isDirty = true;
			SetAllFlagsDirty(true);
			SetChildrenDirty();
		}
		return m_localRotation;
	}

	Vec3 TransformComponent::CalculateWorldForward() const
	{
		return GetWorldRotation().ApplyRotationToDir(ENGINE_FORWARD_DIR);
	}
	Vec3 TransformComponent::CalculateWorldUp() const
	{
		return GetWorldRotation().ApplyRotationToDir(ENGINE_UP_DIR);
	}
	Vec3 TransformComponent::CalculateWorldRight() const
	{
		return GetWorldRotation().ApplyRotationToDir(ENGINE_RIGHT_DIR);
	}
	void TransformComponent::CalculateWorldDirections(Vec3* outForward, Vec3* outUp, Vec3* outRight) const
	{
		if (outForward != nullptr) *outForward = CalculateWorldForward();
		if (outUp != nullptr) *outUp = CalculateWorldUp();
		if (outRight != nullptr)
		{
			Vec3 up;
			if (outUp != nullptr) up = *outUp;
			else up = CalculateWorldUp();

			Vec3 forward;
			if (outForward != nullptr) forward = *outForward;
			else forward = CalculateWorldForward();

			if constexpr (ENGINE_FORWARD_SIGN_Z == ZForwardSign::Negative)
				*outRight = Math::CrossProduct(forward, up).GetNormalized();
			else *outRight = Math::CrossProduct(up, forward).GetNormalized();
		}
	}

	const TransformPrecalculatedData& TransformComponent::GetLastUpdateData() const
	{
		return m_lastUpdateData;
	}

	void TransformComponent::InitFields()
	{
		//TODO: implement
		//m_Fields = { ComponentField("Pos", &m_LocalPos) };
	}

	void TransformComponent::Serialize(Serialization::Serializer& serializer) const
	{
		serializer.AddProperty("LocPos", m_localPos);
		serializer.AddProperty("LocScale", m_localScale);
		serializer.AddProperty("LocRot", m_localRotation);
	}
	void TransformComponent::Deserialize(Serialization::Deserializer& deserializer)
	{
		deserializer.GetProperty("LocPos", &m_localPos);
		deserializer.GetProperty("LocScale", &m_localScale);
		deserializer.GetProperty("LocRot", &m_localRotation);
		SetAllFlagsDirty(true);
	}

	std::string TransformComponent::ToString() const
	{
		const ECS::EntityData* parent = GetEntity().GetParent();
		return std::format("[<Transform> Parent:{} LPos:{} GPos:{} LScale:{} GScale:{} LRot:{} GRot:{}]",
			parent != nullptr ? parent->m_Name : "NULL",
			m_localPos.ToString(), GetWorldPos().ToString(), m_localScale.ToString(), GetWorldScale().ToString(),
			m_localRotation.ToString(), GetWorldRotation().ToString());
	}
}
