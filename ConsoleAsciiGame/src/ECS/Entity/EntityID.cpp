#include "pch.hpp"
#include "ECS/Entity/EntityID.hpp"

namespace ECS
{
	bool IsValidID(const EntityID id) { return id != INVALID_ID; }
}
