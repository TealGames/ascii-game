#include "pch.hpp"
#include "EntityID.hpp"

namespace ECS
{
	bool IsValidID(const EntityID id) { return id != INVALID_ID; }
}
