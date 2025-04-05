#include "pch.hpp"
#include "ReferenceDatabase.hpp"

namespace ReferenceRegistry
{
	DatabaseCollection Registry;

	std::string ToString(const ReferenceType& type)
	{
		if (type == ReferenceType::Float) return "Float";
		else if (type == ReferenceType::Integer32) return "Integer32";
		else if (type == ReferenceType::Uint8) return "Uint8";

		Assert(false, std::format("Tried to convert a database storeable type to string "
			"but no actions for this type were provided"));
		return "";
	}

	DatabaseCollection::iterator ReferenceRegistry::TryGetReferenceMutable(const std::string& name)
	{
		return Registry.find(name);
	}

	bool ReferenceRegistry::HasReference(const std::string& name)
	{
		return Registry.find(name) != Registry.end();
	}
}