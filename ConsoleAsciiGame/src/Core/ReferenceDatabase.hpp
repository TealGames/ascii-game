#pragma once
#include "unordered_map"
#include <cstdint>
#include <type_traits>
#include "Utils/HelperFunctions.hpp"
#include "Core/Analyzation/Debug.hpp"

enum class ReferenceType
{
	Float,
	Integer32,
	Uint8,
};
std::string ToString(const ReferenceType& type);

struct StoredReference
{
	ReferenceType m_Type;
	void* m_Pointer;
};

namespace ReferenceRegistry
{
	/// <summary>
	/// Since it may be important to have access to references between executions we can solve this problem
	/// by utilizing a databse that stores id and reference key-value pairs
	/// </summary>
	using DatabaseCollection = std::unordered_map<std::string, StoredReference>;
	extern DatabaseCollection Registry;

	bool HasReference(const std::string& name);
	DatabaseCollection::iterator TryGetReferenceMutable(const std::string& name);

	template<typename T>
	bool TryAddReference(const std::string& name, T& reference)
	{
		if (!Assert(!HasReference(name), std::format("Tried to add a reference named:{} to database,"
			"but there is already a reference by that name", name)))
			return false;

		if (typeid(reference).name() == typeid(float))
		{
			Registry.emplace(name, StoredReference(ReferenceType::Float, &reference));
			return true;
		}
		else if (typeid(reference).name() == typeid(int))
		{
			Registry.emplace(name, StoredReference(ReferenceType::Integer32, &reference));
			return true;
		}
		else if (typeid(reference).name() == typeid(std::uint8_t))
		{
			Registry.emplace(name, StoredReference(ReferenceType::Uint8, &reference));
			return true;
		}

		LogError(std::format("Tried to store reference named: {} to database "
			"but it is not an acceptable type: {}", name, typeid(reference).name()));
		return false;
	}

	template<typename T>
		requires (!std::is_pointer_v<T>())
	T* TryGetReference(const std::string& name)
	{
		auto it = TryGetReferenceMutable(name);
		if (!Assert(it != Registry.end(), std::format("Tried to get reference named: {} from database "
			"but no reference by that name exists", name)))
			return nullptr;

		if (!Assert(it->second.m_Pointer != nullptr, std::format("Tried to get reference named: {} from database"
			" but a reference by that name has a null reference")))
			return nullptr;

		if (typeid(T).name() == typeid(float) && it->second.m_Type == ReferenceType::Float)
		{
			return static_cast<T*>(it->second.m_Pointer);
		}
		else if (typeid(T).name() == typeid(int) && it->second.m_Type == ReferenceType::Integer32)
		{
			return static_cast<T*>(it->second.m_Pointer);
		}
		else if (typeid(T).name() == typeid(std::uint8_t) && it->second.m_Type == ReferenceType::Uint8)
		{
			return static_cast<T*>(it->second.m_Pointer);
		}
		LogError(std::format("Tried to get reference named: {} using type: {} from database "
			"but its real type does not match one provided. Real type is: {}", name, typeid(T).name(), ToString(it->second.m_Type)));
		return nullptr;
	}
}

