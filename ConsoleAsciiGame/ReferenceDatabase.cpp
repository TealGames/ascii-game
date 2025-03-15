#include "pch.hpp"
#include "ReferenceDatabase.hpp"

ReferenceDatabase::ReferenceDatabase(): m_database() {}

std::string ToString(const ReferenceType& type)
{
	if (type == ReferenceType::Float) return "Float";
	else if (type == ReferenceType::Integer32) return "Integer32";
	else if (type == ReferenceType::Uint8) return "Uint8";

	Assert(false, std::format("Tried to convert a database storeable type to string "
		"but no actions for this type were provided"));
	return "";
}

DatabaseCollection::iterator ReferenceDatabase::TryGetReferenceMutable(const std::string& name)
{
	return m_database.find(name);
}

bool ReferenceDatabase::HasReference(const std::string& name) const
{
	return m_database.find(name) != m_database.end();
}
