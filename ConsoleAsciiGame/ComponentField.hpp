#pragma once
#include <string>
#include <variant>
#include "Vec2.hpp"
#include <type_traits>
#include <typeinfo>

enum class ComponentFieldType
{
	String,
	Integer,
	Float,
	Vector2
};

//TODO: ideally we would not use a direct value but rather a function to set those values to add some abstraction
//and allow some clamping or other import actions to be taken if neccessary
using ComponentFieldVariant = std::variant<std::string*, int*, float*, Utils::Point2D*>;
struct ComponentField
{
	std::string m_FieldName;
	ComponentFieldVariant m_Value;
	//ComponentFieldType m_Type;

	ComponentField(const std::string& name, const ComponentFieldVariant& value);

	const std::type_info& GetCurrentType() const;
	std::string ToString() const;

	template<typename T>
	requires (!std::is_pointer_v<T>)
	bool IsCurrentType() const
	{
		return std::holds_alternative<T*>(m_Value);
	}

	template<typename T>
	requires (!std::is_pointer_v<T>)
	const T* TryGetValue() const
	{
		if (IsCurrentType<T>()) return std::get<T*>(m_Value);
		return nullptr;
	}

	template<typename T>
	requires (!std::is_pointer_v<T>)
	bool TrySetValue(const T value)
	{
		if (IsCurrentType<T>())
		{
			*(std::get<T*>(m_Value)) = value;
			return true;
		}
		return false;
	}
};

