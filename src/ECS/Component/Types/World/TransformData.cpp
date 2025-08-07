//NOT USED
#include "pch.hpp"
#include "ECS/Component/Types/World/TransformData.hpp"
#include "Core/Analyzation/Debug.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "Core/Serialization/JsonSerializers.hpp"
//#include "glm/gtc/matrix_transform.hpp"

TransformComponent::TransformComponent(const Json& json) : TransformComponent()
{
	m_isDirty = true;
	Deserialize(json);
}

TransformComponent::TransformComponent(const Vec3 pos, const Vec3 scale, const Quat rotation):
	Component(),
	m_localPos(pos), //m_localPosLastFrame(NULL_POS), 
	m_localScale(scale),
	m_localRotation(rotation),
	m_lastUpdateData()
{
	//By default we set dirty so that global pos can be calculated 
	//the first time we retrieve it
	m_isDirty = true;
}
void TransformComponent::UpdatePrecalculatedData() const
{
	const TransformComponent* parent = GetEntity().GetParentTransform();

	if (parent == nullptr)
	{
		m_lastUpdateData.m_GlobalPos = m_localPos; 
		m_lastUpdateData.m_GlobalScale = m_localScale;
		m_lastUpdateData.m_GlobalRotation = m_localRotation;
	}
	else if (!parent->m_isDirty)
	{
		m_lastUpdateData.m_GlobalPos = parent->m_lastUpdateData.m_GlobalPos + m_localPos;
		m_lastUpdateData.m_GlobalScale = parent->m_lastUpdateData.m_GlobalScale * m_localScale;
		m_lastUpdateData.m_GlobalRotation = parent->m_lastUpdateData.m_GlobalRotation * m_localRotation;
	}
	else
	{
		m_lastUpdateData.m_GlobalPos = parent->GetGlobalPos() + m_localPos;
		m_lastUpdateData.m_GlobalScale = parent->GetGlobalScale() * m_localScale;
		m_lastUpdateData.m_GlobalRotation = parent->GetGlobalRotation() * m_localRotation;
	}
	m_lastUpdateData.m_GlobalModelMatrix = CalculateModelMatrix(nullptr, m_lastUpdateData.m_GlobalPos, 
		m_lastUpdateData.m_GlobalScale, m_lastUpdateData.m_GlobalRotation);
}

const Vec3& TransformComponent::GetGlobalPos() const
{
	//TODO: isnt it a little hacky to use MUTABLE modifier to the last update data
	//so you can modify internal state even in const function?
	if (m_isDirty)
	{
		UpdatePrecalculatedData();
		m_isDirty = false;
	}

	return m_lastUpdateData.m_GlobalPos;
}
const Vec3& TransformComponent::GetGlobalScale() const
{
	if (m_isDirty)
	{
		UpdatePrecalculatedData();
		m_isDirty = false;
	}

	return m_lastUpdateData.m_GlobalScale;
}
const Quat& TransformComponent::GetGlobalRotation() const
{
	if (m_isDirty)
	{
		UpdatePrecalculatedData();
		m_isDirty = false;
	}

	return m_lastUpdateData.m_GlobalRotation;
}
const Mat4& TransformComponent::GetWorldModelMatrix() const
{
	if (m_isDirty)
	{
		UpdatePrecalculatedData();
		m_isDirty = false;
	}
	return m_lastUpdateData.m_GlobalModelMatrix;
}

void TransformComponent::SetChildrenDirty()
{
	for (auto& child : GetEntityMutable().GetChildrenOfTypeMutable<TransformComponent>())
	{
		//If we reach a child that is already dirty, it means the its children SHOULD ALREADY BE DIRTY
		//(because the dirty setting only occurs on pos, scale, rot mutation)
		if (child->m_isDirty)
			continue;
		
		child->m_isDirty = true;
		child->SetChildrenDirty();
	}
}

const Vec3& TransformComponent::GetLocalPos() const
{
	return m_localPos;
}
const Vec3& TransformComponent::GetLocalScaleMutable() const
{
	return m_localScale;
}
const Quat& TransformComponent::GetLocalRotationMutable() const
{
	return m_localRotation;
}

Vec3& TransformComponent::GetLocalPosMutable()
{
	if (!m_isDirty)
	{
		m_isDirty = true;
		SetChildrenDirty();
	}
	
	return m_localPos;
}
Vec3& TransformComponent::GetLocalScaleMutable()
{
	if (!m_isDirty)
	{
		m_isDirty = true;
		SetChildrenDirty();
	}
	return m_localScale;
}
Quat& TransformComponent::GetLocalRotationMutable()
{
	if (!m_isDirty)
	{
		m_isDirty = true;
		SetChildrenDirty();
	}
	return m_localRotation;
}

Mat4 CalculateTranslationMatrix(const Vec3& pos)
{
	return Mat4(std::array<std::array<float, 4>, 4>
	{{
		{ {1, 0, 0, pos.m_X} },
		{ {0, 1, 0, pos.m_Y} },
		{ {0, 0, 1, pos.m_Z} },
		{ {0, 0, 0, 1} }
		}});
}
Mat4 CalculateScaleMatrix(const Vec3& scale)
{
	return Mat4(std::array<std::array<float, 4>, 4>
	{{
		{ {scale.m_X, 0, 0, 0} },
		{ {0, scale.m_Y, 0, 0} },
		{ {0, 0, scale.m_Z, 0} },
		{ {0, 0, 0, 1} }
		}});
}
Mat4 CalculateRotationMatrix(const Quat& rotation)
{
	const float x = rotation.m_X, y = rotation.m_Y, z = rotation.m_Z, w = rotation.m_W;

	const float xx = x * x, yy = y * y, zz = z * z;
	const float xy = x * y, xz = x * z, yz = y * z;
	const float wx = w * x, wy = w * y, wz = w * z;

	return Mat4(
		{ {
			{{1 - 2 * yy - 2 * zz,	2 * xy - 2 * wz,		2 * xz + 2 * wy,		0.0f}},
			{{2 * xy + 2 * wz,		1 - 2 * xx - 2 * zz,	2 * yz - 2 * wx,		0.0f}},
			{{2 * xz - 2 * wy,		2 * yz + 2 * wx,		1 - 2 * xx - 2 * yy,	0.0f}},
			{{0.0f,					0.0f,					0.0f,					1.0f}}
		} });
}
Mat4 CalculateModelMatrix(const Mat4* parentMatrix, const Vec3& pos, const Vec3& scale, const Quat& rotation)
{
	if (parentMatrix == nullptr)
		return CalculateTranslationMatrix(pos) * CalculateRotationMatrix(rotation) * CalculateScaleMatrix(scale);
	else
		return *parentMatrix * CalculateTranslationMatrix(pos) * CalculateRotationMatrix(rotation) * CalculateScaleMatrix(scale);
}

//Mat4 TransformComponent::CalculateLocalTranslationMatrix() const
//{
//	return CalculateTranslationMatrix(m_localPos);
//}
//Mat4 TransformComponent::CalculateLocalScaleMatrix() const
//{
//	return CalculateScaleMatrix(m_localScale);
//}
//Mat4 TransformComponent::CalculateLocalRotationMatrix() const
//{
//	return CalculateRotationMatrix(m_localRotation);
//}

//Mat4 TransformComponent::CalculateLocalModelMatrix() const
//{
//	return CalculateModelMatrix(nullptr, m_localPos, m_localScale, m_localRotation);
//}

const TransformPrecalculatedData& TransformComponent::GetLastUpdateData() const
{
	return m_lastUpdateData;
}

void TransformComponent::InitFields()
{
	//TODO: implement
	//m_Fields = { ComponentField("Pos", &m_LocalPos) };
}

void TransformComponent::Deserialize(const Json& json)
{
	m_localPos = json.value("LocPos", DEFAULT_POS);
	m_localScale = json.value("LocScale", DEFAULT_SCALE);
	m_localRotation = json.value("LocRot", DEFAULT_ROTATION);
	//m_localPosLastFrame = json.at("LastFramePos").get<Vec2>();
}
Json TransformComponent::Serialize()
{
	return { {"Pos", m_localPos}, {"LocScale", m_localScale}, {"LocRot", m_localRotation}}; //{"LastFramePos", m_localPosLastFrame}};
}

std::string TransformComponent::ToString() const
{
	const EntityData* parent = GetEntity().GetParent();
	return std::format("[<Transform> Parent:{} LPos:{} GPos:{} LScale:{} GScale:{} LRot:{} GRot:{}]", 
		parent!=nullptr? parent->m_Name : "NULL",
		m_localPos.ToString(), GetGlobalPos().ToString(), m_localScale.ToString(), GetGlobalScale().ToString(), 
		m_localRotation.ToString(), GetGlobalRotation().ToString());
}