#pragma once
#include <entt/entt.hpp>

namespace ECS
{
	using EntityID = entt::entity;
	constexpr EntityID INVALID_ID = entt::null;

	bool IsValidID(const EntityID id);
	std::string ToString(const ECS::EntityID& id);
	//using EntityID = std::uint32_t;
}

