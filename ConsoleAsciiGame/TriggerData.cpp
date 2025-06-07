#include "pch.hpp"
#include "TriggerData.hpp"
#include "Debug.hpp"

std::string ToString(const TriggerType& trigger)
{
	if (trigger == TriggerType::Default) return "Default";
	else if (trigger == TriggerType::Scene) return "Scene";
	else
	{
		LogError("Tried to stringify trigger type but type actions could not be found");
		return "";
	}
}


TriggerData::TriggerData() : TriggerData(nullptr) {}
TriggerData::TriggerData(const CollisionBoxData& collider) : TriggerData(&collider) {}
TriggerData::TriggerData(const CollisionBoxData* collider) : 
	Component(), m_collider(collider), m_triggerType(TriggerType::Default), m_OnEnter(), m_OnExit() {}

const CollisionBoxData& TriggerData::GetCollisionBox() const
{
	if (!Assert(this, m_collider != nullptr, std::format("Tried to get collider but it is NULL")))
		throw std::invalid_argument("Invalid collider box state");

	return *m_collider;
}

//std::vector<std::string> TriggerData::GetDependencyFlags() const
//{
//	return { Utils::GetTypeName<CollisionBoxData>()};
//}
void TriggerData::InitFields()
{
	m_Fields = {};
}

std::string TriggerData::ToString() const
{
	return std::format("[Trigger Type:{}]", ::ToString(m_triggerType));
}

void TriggerData::Deserialize(const Json& json)
{
	//TODO: implmenet
	return;
}
Json TriggerData::Serialize()
{
	//TODO: implmenet
	return {};
}
