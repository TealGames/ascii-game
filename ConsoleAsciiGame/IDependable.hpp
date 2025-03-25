#pragma once
#include <type_traits>

template<typename ...Args>
struct IDependable
{
	IDependable() = default;
	virtual ~IDependable() = default;

	virtual void SetDependencies(std::add_lvalue_reference_t<Args>...) = 0;
};

