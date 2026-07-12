#pragma once

namespace Engine
{
	using ObjectID = size_t;
	inline constexpr ObjectID INVALID_ID = 0;

	inline bool IsValidID(const ObjectID id)
	{
		return id != INVALID_ID;
	}

	inline ObjectID GetID()
	{
		static ObjectID currID = INVALID_ID;
		return ++currID;
	}
}
