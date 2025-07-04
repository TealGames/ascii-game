#pragma once
#include <string>
#include <variant>
#include "Utils/Data/Vec2.hpp"
#include "Utils/Data/Vec2Int.hpp"
#include <type_traits>
#include <typeinfo>
#include <functional>
#include <optional>
#include <cstdint>
#include "Utils/Data/Color.hpp"
#include "Core/Analyzation/Debug.hpp"

//enum class ComponentFieldType
//{
//	String,
//	Integer,
//	Float,
//	Vector2
//};

//TODO: ideally we would not use a direct value but rather a function to set those values to add some abstraction
//and allow some clamping or other import actions to be taken if neccessary
using ComponentFieldVariant = std::variant<std::string*, int*, float*, std::uint8_t*, bool*, Vec2*, Vec2Int*, 
							Utils::Color*>;
using ComponentFieldSetAction = std::variant<std::function<void(std::string)>, std::function<void(int)>, std::function<void(float)>, 
	std::function<void(std::uint8_t)>, std::function<void(bool)>, std::function<void(Vec2)>, std::function<void(Vec2Int)>, std::function<void(Utils::Color)>>;

struct ComponentField
{
private:
	bool m_isReadonly;

public:
	std::string m_FieldName;
	ComponentFieldVariant m_Value;
	std::optional<ComponentFieldSetAction> m_MaybeSetFunction;

	//ComponentFieldType m_Type;
private:
public:
	ComponentField(const std::string& name, const ComponentFieldVariant& value, const bool& isWritable=true);
	ComponentField(const std::string& name, const ComponentFieldSetAction& setAction, const ComponentFieldVariant& value);

	const std::type_info& GetCurrentType() const;
	std::string ToString() const;

	bool IsReadonly() const;
	bool HasSetFunction() const;

	template<typename T>
	requires (!std::is_pointer_v<T>)
	bool IsSetFunctionofType() const
	{
		if (!HasSetFunction()) return false;
		return std::holds_alternative<std::function<void(T)>>(m_MaybeSetFunction.value());
	}

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
			if (HasSetFunction())
			{
				if (!Assert(IsSetFunctionofType<T>(), std::format("Tried to set value of field: '{}' of type: {} "
					"with a set function but set function does not match that type", m_FieldName, GetCurrentType().name())))
					throw std::invalid_argument("Invalid set function type");

				/*if (!Assert(m_MaybeSetFunction.value()!=nullptr, std::format("Tried to set value of field: '{}' of type: {} "
						"with a set function that is NULL", m_FieldName, GetCurrentType().name())))
					throw std::invalid_argument("Invalid set function");*/

				std::get<std::function<void(T)>>(m_MaybeSetFunction.value())(value);
			}
			else *(std::get<T*>(m_Value)) = value;
			
			return true;
		}
		return false;
	}
};

