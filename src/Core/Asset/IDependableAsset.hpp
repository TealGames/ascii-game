#pragma once
#include <type_traits>

template<typename ...Args>
struct IDependableAsset
{
	IDependableAsset() = default;
	virtual ~IDependableAsset() = default;

	virtual void SetDependencies(std::add_lvalue_reference_t<Args>...) = 0;
};

