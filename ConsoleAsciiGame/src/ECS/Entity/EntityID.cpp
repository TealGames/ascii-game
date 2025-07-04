#include "pch.hpp"
#include "ECS/Entity/EntityID.hpp"

namespace ECS
{
	bool IsValidID(const EntityID id) { return id != INVALID_ID; }
	std::string ToString(const ECS::EntityID& id) { return std::to_string(static_cast<uint32_t>(id)); }
}
