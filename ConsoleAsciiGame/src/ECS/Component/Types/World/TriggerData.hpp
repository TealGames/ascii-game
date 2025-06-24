#pragma once
#include "ECS/Component/Component.hpp"
#include <functional>
#include "Utils/Data/Event.hpp"
#include "ECS/Component/Types/World/CollisionBoxData.hpp"

/// <summary>
/// The type of trigger that will determine any preset trigger functions 
/// that are triggered 
/// </summary>
enum class TriggerType
{
	/// <summary>
	/// 
	/// </summary>
	Default,
	Scene,
};
std::string ToString(const TriggerType& trigger);

class TriggerData : public Component
{
private:
	const CollisionBoxData* m_collider;
	TriggerType m_triggerType;
public:
	Event<void, const CollisionBoxData*> m_OnEnter;
	Event<void, const CollisionBoxData*> m_OnExit;

private:
	TriggerData(const CollisionBoxData* collider);
	
public:
	TriggerData();
	TriggerData(const CollisionBoxData& collider);

	const CollisionBoxData& GetCollisionBox() const;

	//std::vector<std::string> GetDependencyFlags() const override;
	void InitFields() override;

	std::string ToString() const override;

	void Deserialize(const Json& json) override;
	Json Serialize() override;
};

