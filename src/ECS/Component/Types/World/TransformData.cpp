//NOT USED
#include "pch.hpp"
#include "ECS/Component/Types/World/TransformData.hpp"
#include "Core/Analyzation/Debug.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "Core/Serialization/JsonSerializers.hpp"
//#include "glm/gtc/matrix_transform.hpp"

TransformData::TransformData(const Json& json) : TransformData()
{
	Deserialize(json);
}

TransformData::TransformData(const Vec3 pos, const Vec3 scale, const Quat rotation):
	Component(),
	m_LocalPos(pos), //m_localPosLastFrame(NULL_POS), 
	m_LocalScale(scale),
	m_LocalRotation(rotation)
{
	
}

//void TransformData::SetLocalPos(const Vec2& newPos)
//{
//	SetLocalPosX(newPos.m_X);
//	SetLocalPosY(newPos.m_Y);
//}
//void TransformData::SetLocalPosX(const float& newX)
//{
//	m_localPos.x = newX;
//}
//void TransformData::SetLocalPosY(const float& newY)
//{
//	m_localPos.y = newY;
//}
//void TransformData::SetLocalPosDeltaX(const float& xDelta)
//{
//	SetLocalPosX(m_localPos.x + xDelta);
//}
//void TransformData::SetLocalPosDeltaY(const float& yDelta)
//{
//	SetLocalPosY(m_localPos.y + yDelta);
//}
//void TransformData::SetLocalPosDelta(const Vec2& moveDelta)
//{
//	SetLocalPosDeltaX(moveDelta.m_X);
//	SetLocalPosDeltaY(moveDelta.m_Y);
//}
//
//Vec2 TransformData::GetLocalPos() const
//{
//	return m_localPos;
//}
Vec3 TransformData::GetGlobalPos() const
{
	const EntityData* parentEntity = GetEntity().GetParent();
	return parentEntity != nullptr ? parentEntity->GetTransform().GetGlobalPos() + m_LocalPos : m_LocalPos;
}
//Vec2 TransformData::GetLocalPosLastFrame() const
//{
//	return m_localPosLastFrame;
//}
//void TransformData::SetLocalPosLastFrame(const Vec2& pos)
//{
//	m_localPosLastFrame = pos;
//}
//bool TransformData::HasMovedThisFrame() const
//{
//	return m_localPosLastFrame == NULL_POS || m_localPos != m_localPosLastFrame;
//}

//Vec2 TransformData::GetLocalScale() const
//{
//	return m_localScale;
//}
Vec3 TransformData::GetGlobalScale() const
{
	const EntityData* parentEntity = GetEntity().GetParent();
	return parentEntity != nullptr ? parentEntity->GetTransform().GetGlobalScale() * m_LocalScale : m_LocalScale;
}

Quat TransformData::GetGlobalRotation() const
{
	const EntityData* parentEntity = GetEntity().GetParent();
	return parentEntity != nullptr ? parentEntity->GetTransform().GetGlobalRotation() * m_LocalRotation : m_LocalRotation;
}
//void TransformData::SetLocalScale(const Vec2 scale)
//{
//	m_localScale = scale;
//}

Mat4 TransformData::CalculateTranslationMatrix(const Vec3& pos)
{
	return Mat4(std::array<std::array<float, 4>, 4>
	{{
		{ {1, 0, 0, 0} },
		{ {0, 1, 0, 0} },
		{ {0, 0, 1, 0} },
		{ {pos.m_X, pos.m_Y, pos.m_Z, 1} }
		}});
}
Mat4 TransformData::CalculateScaleMatrix(const Vec3& scale)
{
	return Mat4(std::array<std::array<float, 4>, 4>
	{{
		{ {scale.m_X, 0, 0, 0} },
		{ {0, scale.m_Y, 0, 0} },
		{ {0, 0, scale.m_Z, 0} },
		{ {0, 0, 0, 1} }
		}});
}
Mat4 TransformData::CalculateRotationMatrix(const Quat& rotation)
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

Mat4 TransformData::CalculateLocalTranslationMatrix() const
{
	return CalculateTranslationMatrix(m_LocalPos);
}
Mat4 TransformData::CalculateLocalScaleMatrix() const
{
	return CalculateScaleMatrix(m_LocalScale);
}
Mat4 TransformData::CalculateLocalRotationMatrix() const
{
	return CalculateRotationMatrix(m_LocalRotation);
}

Mat4 TransformData::GetLocalModelMatrix() const
{
	return CalculateLocalTranslationMatrix() * CalculateLocalRotationMatrix() * CalculateLocalScaleMatrix();
}
Mat4 TransformData::GetWorldModelMatrix() const
{
	const EntityData* parent = GetEntity().GetParent();
	if (parent == nullptr) return GetLocalModelMatrix();

	return parent->GetTransform().GetWorldModelMatrix() * GetLocalModelMatrix();
}

void TransformData::InitFields()
{
	//TODO: implement
	//m_Fields = { ComponentField("Pos", &m_LocalPos) };
}

void TransformData::Deserialize(const Json& json)
{
	m_LocalPos = json.value("LocPos", DEFAULT_POS);
	m_LocalScale = json.value("LocScale", DEFAULT_SCALE);
	m_LocalRotation = json.value("LocRot", DEFAULT_ROTATION);
	//m_localPosLastFrame = json.at("LastFramePos").get<Vec2>();
}
Json TransformData::Serialize()
{
	return { {"Pos", m_LocalPos}, {"LocScale", m_LocalScale}, {"LocRot", m_LocalRotation}}; //{"LastFramePos", m_localPosLastFrame}};
}

std::string TransformData::ToString() const
{
	return std::format("[<Transform> LPos: {} LScale:{} LRotation:{}]", 
		m_LocalPos.ToString(), m_LocalScale.ToString(), m_LocalRotation.ToString());
}